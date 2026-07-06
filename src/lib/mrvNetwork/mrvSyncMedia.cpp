// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvNetwork/mrvFileTransferClient.h"
#include "mrvNetwork/mrvWebRTCClient.h"
#include "mrvNetwork/mrvCommandInterpreter.h"
#include "mrvNetwork/mrvFilePath.h"
#include "mrvNetwork/mrvFilesModelItem.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvPathMapping.h"

#include "mrvWidgets/mrvProgressReport.h"


#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"


#include <tlCore/StringFormat.h>

#include "mrViewer.h"

#include <iostream>


namespace
{
    const char* kModule = "sync";
}


namespace
{
    bool fileIsReadable(const std::string& filePath)
    {
        if (!mrv::file::isReadable(filePath))
        {
            /* xgettext:c++-format */
            const std::string msg =
                tl::string::Format(_("Remote file {0} does not "
                                     "exist on local filesystem."))
                    .arg(filePath);
            LOG_WARNING(msg);
            return false;
        }
        return true;
    }
} // namespace

namespace mrv
{
    struct SyncData
    {
        CommandInterpreter* self;
        bool shuttingDown;
        bool success;
        std::string cachePath;
        std::string audioCachePath;
        FilesModelItem item;
    };

    static void sync_callback(void* data)
    {
        std::unique_ptr<SyncData> d(static_cast<SyncData*>(data));
        if (d->shuttingDown)
            return;

        if (d->success)
            d->self->syncFile(
                d->cachePath,
                d->audioCachePath,
                d->item);
        else
            LOG_ERROR(_("Failed to fetch remote file via SFTP."));
    }

    void CommandInterpreter::shutdownSftpTransfers()
    {
        // std::lock_guard<std::mutex> lk(downloadThreadsMutex_);
        // for (auto& t : activeSftpDownloads_)
        // {
        //     if (t.joinable())
        //         t.join();
        // }
        // activeSftpDownloads_.clear();
    }

    void CommandInterpreter::shutdownTunnels()
    {
        // shuttingDown_ = true;  // signal all threads to wrap up
        // std::lock_guard<std::mutex> lk(tunnelMutex_);
        // for (auto& [peerId, tunnel] : peerTunnels_)
        // {
        //     tunnel->stop();  // stops accept loop, joins acceptThread_, stops all pumps
        // }
        // peerTunnels_.clear();
    }


    tl::file::Path
    CommandInterpreter::cachePathFor(const tl::file::Path& remotePath) const
    {
        // Content-addressed by the remote path string, not its content —
        // fine for now since re-fetch-on-mismatch isn't handled yet.
        size_t h = std::hash<std::string>{}(remotePath.get());
        std::string dir = mrv::homepath() + "/.cache/mrv2/remote/" +
                           std::to_string(h);
        file::mkdirRecursive(dir);
        tl::file::Path path(dir + "/" + file::basename(remotePath.get()));
        const auto& frames = remotePath.getFrames();
        if (frames.has_value())
            path.setFrames(frames.value());
        return path;
    }

    void CommandInterpreter::fetchRemoteFile(
        const std::string& peerId, const file::Path& filePath,
        const file::Path& audioFilePath, const FilesModelItem& item)
    {
        auto* webrtcClient = dynamic_cast<WebRTCClient*>(tcp);
        if (!webrtcClient)
        {
            LOG_ERROR(_("Not connected via WebRTC; cannot fetch remote file."));
            return;
        }

        auto& manager = webrtcClient->manager();

        auto client = manager.getClient(peerId);
        if (!client)
        {
            LOG_ERROR(_("Peer not connected."));
            return;
        }

        bool allowSftpOverRelay = false;
        if (client->isRelayedConnection && !allowSftpOverRelay)
        {
            const std::string msg =
                tl::string::Format(_("SFTP: {0} is only reachable via relay "
                                     "(TURN), which would consume VPS bandwidth. "
                                     "Skipping fetch to avoid unexpected costs."))
                .arg(filePath.get());
            LOG_WARNING(msg);
            return;
        }

        const tl::file::Path& cachePath = cachePathFor(filePath);
        const tl::file::Path& audioCachePath = cachePathFor(audioFilePath);

        ProgressReport* progress = new ProgressReport(App::ui->uiMain, 0, 100,
                                                      "");
        progress->show();


        FileTransferClient ftc(manager, peerId);
        bool ok = false;
        ftc.downloadFile(filePath, cachePath, [&](
                             bool& aborted,
                             const std::string& title,
                             uint64_t done,
                             uint64_t total)
            {
                // progress->set_title(title.c_str());
                // progress->set_end(total);
                // progress->set_value(done);
                // if (!progress->window()->shown())
                // {
                //     aborted = true;
                //     ok = false;
                // }
                std::cerr << done << "/" << total << std::endl;
                if (done == total)
                    ok = true;
            });

        while (!ok)
        {
            Fl::check();
        }

        bool audioOk = true;
        // if (ok && !audioFilePath.isEmpty())
        // {
        //     FileTransferClient ftc(manager, peerId);

        //     audioOk = false;
        //     ftc.downloadFile(audioFilePath, audioCachePath, [&](
        //                          bool& aborted,
        //                          const std::string& title,
        //                          uint64_t done,
        //                          uint64_t total)
        //         {
        //             progress->set_title(title.c_str());
        //             progress->set_end(total);
        //             progress->set_value(done);
        //             if (!progress->window()->shown())
        //             {
        //                 aborted = true;
        //                 audioOk = false;
        //             }
        //             if (done == total)
        //                 audioOk = true;
        //         });
        // }
        bool success = ok && audioOk;

        delete progress;

        if (success)
        {
            syncFile(cachePath.get(), audioCachePath.get(), item);
        }
    }

    void CommandInterpreter::syncFile(
        const std::string& filePath, const std::string& audioFilePath,
        const FilesModelItem& fileModelItem)
    {
        auto app = ui->app;
        auto prefs = ui->uiPrefs;
        auto model = app->filesModel();

        bool isLocked = tcp->isLocked();

        if (!isLocked)
            tcp->lock();

        LOG_STATUS("Opening " << filePath);
        app->open(filePath, audioFilePath);

        if (!isLocked)
            tcp->unlock();

        // Copy annotations to both item and player
        auto item = model->observeA()->get();
        item->annotations = fileModelItem.annotations;
        auto view = ui->uiView;
        if (!view)
            return;
        auto player = view->getTimelinePlayer();
        if (player)
            player->setAllAnnotations(item->annotations);
    }

    void CommandInterpreter::syncMedia(const Message& message)
    {
        // std::cerr << "message = " << message << std::endl;

        const std::string peerId = message.value(kLocalPeerIdKey, std::string());
        auto app = ui->app;
        auto prefs = ui->uiPrefs;
        auto view = ui->uiView;
        auto player = view->getTimelinePlayer();

        // Sync media
        auto model = app->filesModel();
        auto localFiles = model->observeFiles()->get();

        std::vector<FilesModelItem> remoteFiles = message["value"];
        size_t remoteFileSize = remoteFiles.size();
        size_t localFileSize = localFiles.size();

        if (remoteFileSize < localFileSize)
        {
            LOG_WARNING(_("Remote files are less than local files."));
        }

        for (size_t i = 0; i < remoteFileSize; ++i)
        {
            auto path = remoteFiles[i].path;
            auto filePath = path.get();

            fileSourcePeer_.try_emplace(filePath, peerId);

            if (i >= localFileSize)
            {
                auto audioPath = remoteFiles[i].audioPath;
                auto audioFilePath = audioPath.get();
                if (file::isReadable(filePath) &&
                    (audioFilePath.empty() || file::isReadable(audioFilePath)))
                {
                    syncFile(filePath, audioFilePath, remoteFiles[i]);
                }
                else
                {
                    replace_path(filePath);
                    if (!audioFilePath.empty())
                        replace_path(audioFilePath);
                    if (fileIsReadable(filePath) &&
                        (audioFilePath.empty() ||
                         fileIsReadable(audioFilePath)))
                    {
                        syncFile(filePath, audioFilePath, remoteFiles[i]);
                    }
                    else if (!peerId.empty())
                    {
                        // Nothing local worked — last resort, fetch from
                        // the peer that reported this file. Note: uses
                        // path.get()/audioPath.get(), the *original*
                        // remote paths — filePath/audioFilePath were
                        // just mutated in place by replace_path() above.
                        fetchRemoteFile(peerId, path, audioPath,
                                        remoteFiles[i]);
                    }
                    else
                    {
                        std::string msg = tl::string::Format(
                                      _("Remote file {0} not found locally and no "
                                        "peer to fetch it from.")).arg(filePath);
                        LOG_ERROR(msg);
                    }
                }
                continue;
            }
            else
            {
                auto remotePath = remoteFiles[i].path;
                auto localPath = localFiles[i]->path;
                if (remotePath != localPath)
                {
                    // Check if we match on one of the path mappings first.
                    auto filePath = remotePath.get();
                    replace_path(filePath);
                    if (fileIsReadable(filePath))
                    {
                        localFiles[i]->annotations = remoteFiles[i].annotations;
                        continue;
                    }

                    // We didn't.  Check if we match the base name.
                    std::string msg;
                    if (remotePath.getBaseName() == localPath.getBaseName())
                    {
                        /* xgettext:c++-format */
                        msg =
                            tl::string::Format(_("Remote file {0} matches "
                                                 "file name {1} but not path."))
                                .arg(remotePath.get())
                                .arg(remotePath.getBaseName());
                        localFiles[i]->annotations = remoteFiles[i].annotations;
                        LOG_WARNING(msg);
                    }
                    else
                    {
                        /* xgettext:c++-format */
                        msg = tl::string::Format(_("Remote file {0} does not "
                                                   "match local filename."))
                                  .arg(remotePath.get());
                        LOG_ERROR(msg);
                        break;
                    }
                }
            }
        }
    }

} // namespace mrv

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
    tl::file::Path
    CommandInterpreter::cachePathFor(const tl::file::Path& remotePath) const
    {
        if (remotePath.isEmpty())
            return remotePath;

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

        if (client->isRelayedConnection)
        {
            const std::string msg =
                tl::string::Format(_("{0} is only reachable via relay "
                                     "(TURN), which would consume VPS bandwidth. "
                                     "Skipping fetch to avoid unexpected costs."))
                .arg(filePath.get());
            LOG_WARNING(msg);
            return;
        }

        const tl::file::Path& cachePath = cachePathFor(filePath);
        const tl::file::Path& audioCachePath = cachePathFor(audioFilePath);

        ProgressReport* progress = new ProgressReport(App::ui->uiMain, 0, 100,
                                                      _("Transfers"));
        progress->show();
        Fl::check();

        // Blocks (pumping the FLTK event loop) until `remote` has finished
        // downloading to `local`, reusing the same progress window, and
        // returns whether it succeeded.
        //
        // Each call gets its own FileTransferClient rather than sharing
        // one across the video and audio transfers: the class doesn't
        // guarantee it's safe to reuse across two independent
        // downloadFile() calls, and a fresh instance per transfer keeps
        // that a non-issue rather than something to reason about.
        //
        // Completion is tracked with a dedicated atomic<bool> (`done`)
        // rather than by nulling out `progress` from the background
        // callback thread and testing it unsynchronized on the main
        // thread — that raw-pointer read/write pair was a data race, and
        // in the abort case it could also null the pointer out from under
        // a callback that was about to dereference it again.
        auto runTransfer = [&](const file::Path& remote,
                               const tl::file::Path& local) -> bool
            {
                FileTransferClient ftc(manager, peerId);
                std::atomic<bool> done = false;
                std::atomic<bool> success = false;

                ftc.downloadFile(remote, local, [&](
                                     bool& aborted,
                                     const std::string& title,
                                     uint64_t doneBytes,
                                     uint64_t total)
                    {
                        Fl::lock();   // Acquire the GUI lock

                        // Safely update the UI
                        progress->set_title(title.c_str());
                        progress->set_end(total / 1024);
                        progress->set_value(doneBytes / 1024);

                        if (progress->window() && !progress->window()->shown())
                            aborted = true;

                        // Wake up the main thread's event loop so it
                        // redraws immediately. Without this, the UI might
                        // not update until you move your mouse.
                        Fl::awake();

                        // Release the GUI lock
                        Fl::unlock();

                    }, [&](bool ok)
                        {
                            success = ok;
                            done = true;
                        });

                while (!done)
                {
                    Fl::check();
                }
                return success;
            };

        bool ok = runTransfer(filePath, cachePath);

        // Only fetch the audio track if the video actually came through —
        // an aborted or failed video transfer means there's nothing to
        // sync, so don't make the peer (or the user) sit through a second
        // transfer for nothing.
        bool audioOk = true;
        if (ok && !audioFilePath.isEmpty())
        {
            audioOk = runTransfer(audioFilePath, audioCachePath);
        }

        delete progress;

        if (ok && audioOk)
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
        tcp->lock();

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
        {
            player->mergeAllAnnotations(item->annotations);
        }
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

        std::cerr << "syncMedia from " << peerId << std::endl;

        for (size_t i = 0; i < remoteFileSize; ++i)
        {
            auto path = remoteFiles[i].path;
            auto filePath = path.get();

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

// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvUtil.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvPathMapping.h"

#include "mrvNetwork/mrvFilePath.h"
#include "mrvNetwork/mrvFilesModelItem.h"
#include "mrvNetwork/mrvCommandInterpreter.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "sync";
}

namespace
{
    bool fileIsReadable(const std::string& filePath)
    {
        if (!mrv::is_readable(filePath))
        {
            std::string msg =
                tl::string::Format(_("Remote file {0} does not "
                                     "exist on local filesystem."))
                    .arg(filePath);
            LOG_ERROR(msg);
            return false;
        }
        return true;
    }
} // namespace

namespace mrv
{

    void CommandInterpreter::syncFile(
        const std::string& filePath, const std::string& audioFilePath,
        const FilesModelItem& fileModelItem)
    {
        auto app = ui->app;
        auto prefs = ui->uiPrefs;
        auto model = app->filesModel();

        LOG_INFO("Opening " << filePath);
        app->open(filePath, audioFilePath);

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
            if (i >= localFileSize)
            {
                auto path = remoteFiles[i].path;
                auto audioPath = remoteFiles[i].audioPath;
                auto filePath = path.get();
                auto audioFilePath = audioPath.get();
                if (is_readable(filePath) &&
                    (audioFilePath.empty() || is_readable(audioFilePath)))
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

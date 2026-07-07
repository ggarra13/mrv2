// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvFl/mrvPreferences.h"

#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

#include "mrvEdit/mrvEditCallbacks.h"

#include "mrvNetwork/mrvHashFile.h"
#include "mrvNetwork/mrvFilesModelItem.h"
#include "mrvNetwork/mrvProtocolVersion.h"
#include "mrvNetwork/mrvConnectionHandler.h"

#include "mrvOptions/mrvCompareOptions.h"

#include <tlCore/StringFormat.h>

namespace
{
    const char* kModule = "sync";
}


namespace mrv
{
    void TCP::syncClient()
    {
        ViewerUI* ui = App::ui;
        PreferencesUI* prefs = ui->uiPrefs;
        auto view = ui->uiView;
        auto player = view->getTimelinePlayer();

        // Send the protocol version as first thing.
        pushMessage("Protocol Version", kProtocolVersion);

        int value;

        // Sync media
        auto model = ui->app->filesModel();
        auto fileItems = model->observeFiles()->get();

        std::vector< FilesModelItem > items;
        items.reserve(fileItems.size());


        for (const auto& fileItem : fileItems)
        {
            const file::Path& path = (*fileItem).path;
            if (path.hasProtocol())  // already a network file
                continue;
            items.push_back(*fileItem.get());
        }

        // Send all the items to client with their annotations
        Message msg;
        msg["command"] = "Media Items";
        msg["value"] = items;
        pushMessage(msg);


        // Sync main UI elements
        syncUI();


        // Sync panels
        panel::syncPanels();
        
        // Set the current file index
        msg["command"] = "Set A Index";
        msg["value"] = model->observeAIndex()->get();
        pushMessage(msg);


        // Set the comparison file indexes
        msg["command"] = "Set B Indexes";
        msg["value"] = model->observeBIndexes()->get();
        pushMessage(msg);


        // Set the current file index
        msg["command"] = "Set Stereo Index";
        msg["value"] = model->observeStereoIndex()->get();
        pushMessage(msg);


        // Sync current player with all its information
        if (player)
        {

            // Send all annotations of current player
            msg["command"] = "Annotations";
            const auto& annotationsPtr = player->getAllAnnotations();

            std::vector< draw::Annotation > annotations;
            annotations.reserve(annotationsPtr.size());
            for (const auto& annotationPtr : annotationsPtr)
            {
                if (annotationPtr->shapes.empty())
                    continue;
                annotations.push_back(*annotationPtr.get());
            }


            msg["value"] = annotations;
            pushMessage(msg);


            // Send Environment Map Options
            msg["command"] = "setEnvironmentMapOptions";
            msg["value"] = view->getEnvironmentMapOptions();
            pushMessage(msg);


            // Send Background Options
            msg["command"] = "setBackgroundOptions";
            msg["value"] = view->getBackgroundOptions();
            pushMessage(msg);


            // Send Compare Options
            msg["command"] = "setCompareOptions";
            msg["value"] = model->observeCompareOptions()->get();
            pushMessage(msg);


            // Send Stereo 3D Options
            msg["command"] = "setStereo3DOptions";
            msg["value"] = model->observeStereo3DOptions()->get();
            pushMessage(msg);


            // Set Edit mode
            msg["command"] = "setEditMode";
            EditMode mode = (editMode == EditMode::kFull)
                            ? EditMode::kFull
                            : EditMode::kTimeline;
            msg["value"] = mode;
            msg["height"] = editModeH;
            pushMessage(msg);


            // Seek to current time in player
            msg["command"] = "seek";
            msg["value"] = player->currentTime();
            pushMessage(msg);


            // Send playback (as it does not compare if equal, we don't need
            // to send it manually with a message).
            auto playback = player->playback();
            player->setPlayback(playback);
        }
    }
} // namespace mrv

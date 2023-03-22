// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvPlaylistsModel.h"
#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

namespace
{
    inline const std::shared_ptr<mrv::FilesModel>& filesModel()
    {
        mrv::App* app = mrv::App::application();
        return app->filesModel();
    }
    
    inline const std::shared_ptr<mrv::PlaylistsModel>& playlistModel()
    {
        mrv::App* app = mrv::App::application();
        return app->playlistsModel();
    }
    
    inline mrv::SettingsObject* settingsObject()
    {
        mrv::App* app = mrv::App::application();
        return app->settingsObject();
    }
    
    inline std::ostream& operator<<(std::ostream& o,
                                    const mrv::FilesModelItem& a)
    {
        o << "<mrv2.FileMedia path=" << a.path.get()
          << " audioPath=" << a.audioPath.get()
          << " timeRange=" << a.timeRange << " speed=" << a.speed
          << " playback=" << a.playback << " loop=" << a.loop
          << " currentTime=" << a.currentTime
          << " inOutRange=" << a.inOutRange
          << " videoLayer=" << a.videoLayer << " volume=" << a.volume
          << " mute=" << a.mute << " audioOffset=" << a.audioOffset
          << ">";
        return o;
    }
} // namespace

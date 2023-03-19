// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvApp/mrvPlaylistsModel.h"
#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

namespace
{
    const std::shared_ptr<mrv::FilesModel>& filesModel()
    {
        mrv::App* app = mrv::App::application();
        return app->filesModel();
    }
    
    const std::shared_ptr<mrv::PlaylistsModel>& playlistModel()
    {
        mrv::App* app = mrv::App::application();
        return app->playlistsModel();
    }
} // namespace

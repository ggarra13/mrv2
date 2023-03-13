// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

namespace
{
    const std::shared_ptr<mrv::FilesModel>& filesModel()
    {
        mrv::App* app = mrv::App::application();
        return app->filesModel();
    }
} // namespace

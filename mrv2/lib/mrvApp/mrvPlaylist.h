// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>
#include "mrvApp/mrvFilesModel.h"

namespace mrv
{
    struct Playlist
    {
        std::string name;
        std::vector<std::shared_ptr< FilesModelItem > > clips;
    };
    
} // namespace mrv

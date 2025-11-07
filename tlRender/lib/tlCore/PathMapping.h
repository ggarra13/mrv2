// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#pragma once

#include <FL/Fl_Preferences.H>

namespace tl
{
    namespace path_mapping
    {
        void init();
        bool replace_path(std::string& file);
    } // namespace path_mapping
} // namespace tl

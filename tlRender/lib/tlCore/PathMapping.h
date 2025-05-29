
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

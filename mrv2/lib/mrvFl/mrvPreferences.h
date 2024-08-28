// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#ifdef _WIN32
#    pragma warning(disable : 4275)
#endif

#include <string>

#include "mrvFl/mrvColorSchemes.h"

class ViewerUI;
class PreferencesUI;

namespace mrv
{
    class SettingsObject;

    class Preferences
    {
    public:
        Preferences(bool resetSettings, bool resetHotkeys);
        ~Preferences();

        static void run();

        static void reset();

        static void
        load(const bool resetSettings = false, const bool resetHotkeys = false);
        static void save();

        static void open_windows();

        static void OCIO(ViewerUI* ui);

        static void updateICS();

    protected:
        static bool set_transforms();

        static void setOcioConfig(std::string config);

    public:
        static ColorSchemes schemes;
        static bool native_file_chooser;
        static int bgcolor;
        static int textcolor;
        static int selectioncolor;
        static int selectiontextcolor;
        static int switching_images;

        static std::string root;
        static std::string hotkeys_file;
        static int language_index;
        static int debug;
    };

} // namespace mrv

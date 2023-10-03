// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

class ViewerUI;
class HotkeyUI;
class Fl_Preferences;

namespace mrv
{
    class Browser;

    void searchFunction(const std::string& text, mrv::Browser* o);
    void searchHotkey(const std::string& text, mrv::Browser* o);
    void fill_ui_hotkeys(mrv::Browser* o);
    void select_hotkey(HotkeyUI* m);

    void load_hotkeys(ViewerUI* ui);
    void save_hotkeys(Fl_Preferences& keys);

} // namespace mrv

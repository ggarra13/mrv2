// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

class HotkeyUI;

namespace mrv
{
    class Browser;

    void searchFunction(const std::string& text, mrv::Browser* o);
    void searchHotkey(const std::string& text, mrv::Browser* o);
    void fill_ui_hotkeys(mrv::Browser* o);
    void select_hotkey(HotkeyUI* m);

} // namespace mrv

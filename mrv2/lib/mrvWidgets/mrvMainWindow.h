// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <iostream>

#include "mrvDropWindow.h"

#ifdef __APPLE__
#    include <IOKit/pwr_mgt/IOPMLib.h>
#endif

class Fl_Menu_;

class ViewerUI;

namespace mrv
{

    class App;

    class MainWindow : public DropWindow
    {
    public:
        MainWindow(int X, int Y, int W, int H, const char* L = 0);
        MainWindow(int W, int H, const char* L = 0);
        ~MainWindow();

        void main(ViewerUI* m) { ui = m; };
        ViewerUI* main() const { return ui; }

        //! Initialize the window
        void init();

        //! Fill menu based on context information
        void fill_menu(Fl_Menu_* menu);

        //! Make window appear always on top of others
        void always_on_top(int above);

        bool is_on_top() const { return on_top; }

        //! Change window's icon to mrViewer's icon
        void set_icon();

        //! Iconize all windows
        void iconize_all();

    protected:
#ifdef __APPLE__
        IOPMAssertionID assertionID;
        IOReturn success;
#endif
        bool on_top  = false;
        ViewerUI* ui = nullptr;
    };

} // namespace mrv

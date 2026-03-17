// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Double_Window.H>

#include "mrvWidgets/mrvPack.h"

namespace mrv
{
   /** 
    * Class used to hold a panel when the panel is in window mode.
    * 
    */
    class PanelWindow : public Fl_Double_Window
    {
    protected:
        void create_dockable_window(void);
        static std::vector<PanelWindow*> active_list;

        enum Direction {
            kNone = 0,
            kRight = 1,
            kLeft = 2,
            kTop = 4,
            kBottom = 8,
            kTopRight = kTop | kRight,
            kTopLeft = kTop | kLeft,
            kBottomRight = kBottom | kRight,
            kBottomLeft = kBottom | kLeft
        };
        Direction dir;
        int valid;

        int last_x, last_y;
        
        // Track current screen for DPI change detection
        int _current_screen = -1;
        bool refresh_screen = true;

        void set_cursor(int ex, int ey);
        void handle_screen_change();

    public:
        // Normal FLTK constructors
        PanelWindow(int w, int h, const char* l = 0);
        PanelWindow(int x, int y, int w, int h, const char* l = 0);

        void update_resize();
        
        int newX, newY, newW, newH;
        
        virtual ~PanelWindow();

        int handle(int event) override;
        void resize(int X, int Y, int W, int H) override;
        
        //! Methods for hiding/showing *all* the floating windows
        static void show_all();
        static void hide_all();
    };

} // namespace mrv

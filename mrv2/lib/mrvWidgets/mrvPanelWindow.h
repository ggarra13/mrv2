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

        bool use_timeout = false;

        int last_x, last_y;
        int oldX, oldY, oldW, oldH;
        int newX, newY, newW, newH;
        Direction dir;
        int valid;

        void set_cursor(int ex, int ey);

    public:
        // Normal FLTK constructors
        PanelWindow(int w, int h, const char* l = 0);
        PanelWindow(int x, int y, int w, int h, const char* l = 0);

        virtual ~PanelWindow();

        int handle(int event) FL_OVERRIDE;

        void set_resize();

        //! Methods used to start/stop resizing on all panel windows
        static void no_resizable_all();
        static void resizable_all();

        //! Methods for hiding/showing *all* the floating windows
        static void show_all();
        static void hide_all();
    };

} // namespace mrv

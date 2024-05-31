// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Box.H>

class Fl_Window;

namespace mrv
{

    class DragButton : public Fl_Box
    {
    private:
        bool use_timeout = false;
        int x1 = 0, y1 = 0; // click posn., used for dragging and docking checks
        int xoff = 0, yoff = 0; // origin used for dragging calcs
        int previousX = -1, previousY = -1;
        int currentX = -1, currentY = -1;
        int was_docked; // used in handle to note that we have just undocked

    public:
        // basic constructor
        DragButton(int x, int y, int w, int h, const char* l = 0);
        virtual ~DragButton();
        
        // override handle method to catch drag/dock operations
        int handle(int event) override;

        void set_position();
        
    protected:
        int would_dock();

        void get_window_coords(int& X, int& Y);
        void get_global_coords(int& X, int& Y);
        
        void color_dock_group(Fl_Color c);
        void show_dock_group();
        void hide_dock_group();
    };

} // namespace mrv

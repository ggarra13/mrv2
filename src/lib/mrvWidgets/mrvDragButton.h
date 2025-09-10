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
        bool was_docked = false; // used in handle to note that we have just undocked
        int fromx = 0, fromy = 0; // click posn., used for dragging and docking checks
        int winx = 0, winy = 0; // window origin used for dragging calcs
    public:
        // basic constructor
        DragButton(int x, int y, int w, int h, const char* l = 0);
        virtual ~DragButton();
        
        // override handle method to catch drag/dock operations
        int handle(int event) FL_OVERRIDE;
        
    protected:
        int would_dock();

        void get_window_coords(int& X, int& Y);
        void get_global_coords(int& X, int& Y);
        
        void color_dock_group(Fl_Color c);
        void show_dock_group();
        void hide_dock_group();
    };

} // namespace mrv

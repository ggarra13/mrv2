// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>

#include "mrvWidgets/mrvPack.h"

namespace mrv
{

    class DockGroup : public Fl_Group
    {
    public:
        Fl_Window* win;
        Fl_Scroll* scroll;
        Pack* pack;
        int children;

    public:
        //! Normal FLTK constructors
        DockGroup(int x, int y, int w, int h, const char* l = 0);

        //! Point back to our parent
        void set_window(Fl_Window* w) { win = w; }
        Fl_Window* get_window() const { return win; }

        Fl_Scroll* get_scroll() const { return scroll; }

        Pack* get_pack() const { return pack; }

        // methods for adding or removing toolgroups from the dock
        void add(Fl_Widget* w);
        void remove(Fl_Widget* w);

        //! Get a list of all panels attached to the dock group, listed
        //! top to bottom.
        std::vector<std::string> getPanelList() const;
    };

} // namespace mrv

// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <FL/fl_draw.H>

#include "mrvWidgets/mrvTimelineGroup.h"

namespace mrv
{
    TimelineGroup::TimelineGroup(int X, int Y, int W, int H, const char* L) :
        Fl_Group(X, Y, W, H, L)
    {
        visible_focus(0);
    }

    void TimelineGroup::draw()
    {
        if (!visible())
            return;

        // Don't fill the whole background, just the dragbar
        fl_color(color());
        fl_rectf(x(), y(), w(), 22);

        int W = w() / 2 - 20;
        int W2 = w() / 2 + 20;
        const int Y = y() + 1;

        fl_color(FL_WHITE);
        for (int i = W; i <= W2; i += 4)
        {
            const int X = x() + i;
            fl_line(X, Y, X, y() + 6);
        }

        fl_color(FL_BLACK);
        for (int i = W; i <= W2; i += 4)
        {
            const int X = x() + i + 1;
            fl_line(X, Y, X, y() + 6);
        }

        if (!(damage() & FL_DAMAGE_CHILD))
            return;

        Fl_Widget* timeline = child(0);
        // if (timeline->label())
        //     std::cerr << "draw " << timeline->label() << std::endl;
        timeline->redraw();
    }

} // namespace mrv

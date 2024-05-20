// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

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
        Fl_Group::draw();
        int W = w() / 2 - 20;
        int W2 = w() / 2 + 20;
        fl_color(FL_WHITE);
        const int Y = y() + 1;
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
    }

} // namespace mrv

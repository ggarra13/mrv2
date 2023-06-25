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
        box(FL_FLAT_BOX);
    }

    void TimelineGroup::draw()
    {
        Fl_Group::draw();
        int W = w() / 2 - 20;
        int W2 = w() / 2 + 20;
        fl_color(FL_WHITE);
        for (int i = W; i <= W2; i += 4)
        {
            const int X = x() + i;
            fl_line(X, y(), X, y() + 4);
        }
    }

} // namespace mrv

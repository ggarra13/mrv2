// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Group.H>

namespace mrv
{

    class TimelineGroup : public Fl_Group
    {
    public:
        TimelineGroup(int X, int Y, int W, int H, const char* L = 0);
        void draw() override;
    };

} // namespace mrv

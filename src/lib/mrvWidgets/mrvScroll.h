// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Scroll.H>

namespace mrv
{
    class Scroll : public Fl_Scroll
    {
    public:
        Scroll(int X, int Y, int W, int H, const char* L = 0);

        int handle(int event) FL_OVERRIDE;
        void draw() FL_OVERRIDE;
    };

} // namespace mrv

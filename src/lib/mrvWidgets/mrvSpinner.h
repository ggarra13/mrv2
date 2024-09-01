// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Spinner.H>

namespace mrv
{
    class Spinner : public Fl_Spinner
    {
    public:
        Spinner(int X, int Y, int W, int H, const char* L = 0) :
            Fl_Spinner(X, Y, W, H, L)
        {
            labelsize(12);
            textcolor(FL_BLACK);
            input_.cursor_color(FL_RED);
        }
    };
} // namespace mrv

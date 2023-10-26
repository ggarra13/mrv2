// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Value_Input.H>

namespace mrv
{
    class FPSInput : public Fl_Value_Input
    {
    public:
        FPSInput(int X, int Y, int W, int H, char* L = 0) :
            Fl_Value_Input(X, Y, W, H, L)
        {
            cursor_color(FL_RED);
        };

        int format(char* buffer) FL_OVERRIDE;

        void resize(int X, int Y, int W, int H) FL_OVERRIDE;
    };
} // namespace mrv

// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Int_Input.H>

namespace mrv
{
    class IntInput : public Fl_Int_Input
    {
    public:
        IntInput(int X, int Y, int W, int H, const char* L = 0) :
            Fl_Int_Input(X, Y, W, H, L)
        {
            labelsize(12);
            color((Fl_Color)0xf98a8a800);
            textcolor(FL_BLACK);
            cursor_color(FL_RED);
        }
    };
} // namespace mrv

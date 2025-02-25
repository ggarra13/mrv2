// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvWidgets/mrvToggleButton.h"

namespace mrv
{

    ToggleButton::ToggleButton(int X, int Y, int W, int H, const char* L) :
        Fl_Button(X, Y, W, H, L)
    {
        c = FL_FOREGROUND_COLOR;
    }

    void ToggleButton::draw()
    {
        if (type() == FL_HIDDEN_BUTTON)
            return;
        Fl_Color col = value() ? selection_color() : color();
        draw_box(box(), col);
        draw_backdrop();
        if (value())
            Fl_Button::labelcolor(0xffffffff);
        else
            Fl_Button::labelcolor(c);
        draw_label();
    }

} // namespace mrv

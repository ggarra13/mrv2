// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Window.H>

#include "mrvButton.h"

namespace mrv
{

    Button::Button(int X, int Y, int W, int H, const char* L) :
        Fl_Button(X, Y, W, H, L)
    {
        default_color = color();
    }

    void Button::set_cursor(Fl_Cursor e)
    {
        if (window())
            window()->cursor(e);
    }
    
    int Button::handle(int e)
    {
        int ret = Fl_Button::handle(e);
        switch (e)
        {
        case FL_ENTER:
            default_color = color();
            if (active_r() && !value())
            {
                set_cursor(FL_CURSOR_ARROW);
                color(fl_lighter(default_color));
                redraw();
            }
            return 1;
        case FL_LEAVE:
            if (color() != FL_YELLOW)
                color(default_color);
            redraw();
            return 1;
        }
        return ret;
    }

    void Button::color(Fl_Color c)
    {
        if (c != FL_BACKGROUND_COLOR && c != fl_lighter(default_color) &&
            c != FL_YELLOW)
            default_color = c;
        Fl_Button::color(c);
    }

    void Button::draw()
    {
        if (type() == FL_HIDDEN_BUTTON)
            return;
        Fl_Color col = value() ? selection_color() : color();
        draw_box(FL_FLAT_BOX, color()); // needed to clear background on round
        if (value())
            draw_box(down_box(), col);
        draw_backdrop();
        draw_label();
    }

} // namespace mrv

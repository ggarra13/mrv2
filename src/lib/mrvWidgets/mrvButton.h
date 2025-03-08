// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Button.H>

namespace mrv
{

    //! Button class that changes cursor and highlights the button when hover it.
    class Button : public Fl_Button
    {
        Fl_Color default_color;

    public:
        Button(int X, int Y, int W, int H, const char* L = 0);

        int handle(int e) FL_OVERRIDE;
        void draw() FL_OVERRIDE;

        void color(Fl_Color c);
        Fl_Color color() { return Fl_Button::color(); }

    protected:
        void set_cursor(Fl_Cursor);
    };

} // namespace mrv

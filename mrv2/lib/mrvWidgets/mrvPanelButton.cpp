// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Window.H>

#include "mrvPanelButton.h"

namespace mrv
{

    PanelButton::PanelButton(int x, int y, int w, int h, const char* l) :
        Fl_Button(x, y, w, h, l)
    {
    }

    int PanelButton::handle(int event)
    {
        int ret = Fl_Button::handle(event);
        Fl_Window* win = window();
        if (!win)
            return ret;

        switch (event)
        {
        case FL_ENTER:
        {
            win->cursor(FL_CURSOR_ARROW);
            ret = 1;
        }
        case FL_LEAVE:
        {
            win->cursor(FL_CURSOR_DEFAULT);
            ret = 1;
        }
        }
        return ret;
    } // handle

} // namespace mrv

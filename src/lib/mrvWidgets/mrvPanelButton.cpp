// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Window.H>

#include "mrvUI/mrvDesktop.h"

#include "mrvWidgets/mrvPanelButton.h"

namespace mrv
{

    PanelButton::PanelButton(int x, int y, int w, int h, const char* l) :
        Fl_Button(x, y, w, h, l)
    {
#if FLTK_HAVE_PEN_SUPPORT
        if (!desktop::X11() && !desktop::XWayland())
            Fl::Pen::subscribe(this);
#endif
    }

    int PanelButton::handle(int event)
    {
        int ret = Fl_Button::handle(event);
        Fl_Window* win = window();
        if (!win)
            return ret;

        switch (event)
        {
        case Fl::Pen::ENTER:
            pen_handled = false;
            return 1;
        case Fl::Pen::TOUCH:
            do_callback();
            pen_handled = true;
            return 1;
        case Fl::Pen::DRAW:
            return 1;
        case Fl::Pen::LIFT:
            pen_handled = false;
            return 1;
        case FL_ENTER:
            win->cursor(FL_CURSOR_ARROW);
            return 1;
        case FL_LEAVE:
            win->cursor(FL_CURSOR_DEFAULT);
            return 1;
        default:
            break;
        }
        return ret;
    } // handle

} // namespace mrv

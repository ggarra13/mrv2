// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include "mrvWidgets/mrvPopupAudio.h"
#include <FL/Fl_Menu_Item.H>

namespace mrv
{

    int PopupAudio::handle(int e)
    {
        if (!menu() || !menu()->text)
            return 0;
        switch (e)
        {
        case FL_PUSH:
            if (!box())
            {
                if (Fl::event_button() != 3)
                    return 0;
            } else if (type())
            {
                if (!(type() & (1 << (Fl::event_button() - 1))))
                    return 0;
            }
            if (Fl::visible_focus())
                Fl::focus(this);
            if (children() > 3)
                popup();
            else
            {
                int v                 = value() ? 0 : 1;
                const Fl_Menu_Item* m = child(v);
                picked(m);
                do_callback();
                redraw();
            }
            return 1;
        }
        return PopupMenu::handle(e);
    }

    PopupAudio::PopupAudio(int X, int Y, int W, int H, const char* l) :
        PopupMenu(X, Y, W, H, l)
    {}

} // namespace mrv

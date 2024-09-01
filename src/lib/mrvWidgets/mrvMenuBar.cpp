// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvWidgets/mrvMenuBar.h"

namespace mrv
{
    int MenuBar::handle(int event)
    {
        const Fl_Menu_Item* v;
        switch (event)
        {
        case FL_SHORTCUT:
            v = menu()->find_shortcut(0, true);
            if (v && v->submenu())
            {
                v = menu()->pulldown(x(), y(), w(), h(), v, this, 0, 1);
                picked(v);
                return 1;
            }
            return test_shortcut() != 0;
        default:
            return Fl_Menu_Bar::handle(event);
        }
        return 0;
    }

    MenuBar::MenuBar(int X, int Y, int W, int H, const char* l) :
        Fl_Menu_Bar(X, Y, W, H, l)
    {
    }

} // namespace mrv

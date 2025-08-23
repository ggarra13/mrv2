// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvWidgets/mrvMenuBar.h"

namespace mrv
{
    int MenuBar::handle(int event)
    {
        return Fl_Menu_Bar::handle(event);
    }

    MenuBar::MenuBar(int X, int Y, int W, int H, const char* l) :
        Fl_Menu_Bar(X, Y, W, H, l)
    {
    }

} // namespace mrv

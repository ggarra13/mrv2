// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <FL/Fl.H>

#include "mrvWidgets/mrvScroll.h"

namespace mrv
{
    
    Scroll::Scroll(int X, int Y, int W, int H, const char* L) :
        Fl_Scroll(X, Y, W, H, L)
    {
        labeltype(FL_NO_LABEL);
    }
    
    int Scroll::handle(int event)
    {
        if (event == FL_MOUSEWHEEL)
        {
            int ret = 0;
            if (scrollbar.visible())
            {
                ret = scrollbar.handle(event);
            }

            if (hscrollbar.visible())
            {
                ret = hscrollbar.handle(event);
            }
            if (ret)
                return ret;
        }
        return Fl_Scroll::handle(event);
    }
    
} // namespace mrv

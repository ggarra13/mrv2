// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#pragma once

#include <FL/Fl_Button.H>

namespace mrv
{
    class ToggleButton : public Fl_Button
    {
        Fl_Color c;
    public:
        ToggleButton(int X, int Y, int W, int H, const char* L = 0);

        void labelcolor(Fl_Color col) { c = col; redraw(); } 
        
        void draw() FL_OVERRIDE;
    };

} // namespace mrv

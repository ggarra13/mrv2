// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Menu_Bar.H>

namespace mrv
{
    class MenuBar : public Fl_Menu_Bar
    {
    public:
        MenuBar(int X, int Y, int W, int H, const char* L = 0);

        int handle(int) FL_OVERRIDE;
    };

} // namespace mrv

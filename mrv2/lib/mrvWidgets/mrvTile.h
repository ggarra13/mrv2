// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "FL/Fl_Tile.H"

namespace mrv
{

    class Tile : public Fl_Tile
    {
    public:
        int handle(int event) FL_OVERRIDE;
        Tile(int X, int Y, int W, int H, const char* L = 0);
        void resize(int X, int Y, int W, int H) FL_OVERRIDE;
        void
        move_intersection(int oldx, int oldy, int newx, int newy, int event);
        void init_sizes();
    };

} // namespace mrv

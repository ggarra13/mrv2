
#pragma once

#include "FL/Fl_Tile.H"

namespace mrv
{

    class FL_EXPORT Tile : public Fl_Tile
    {
    public:
        int handle(int event) FL_OVERRIDE;
        Tile(int X, int Y, int W, int H, const char* L = 0);
        void resize(int X, int Y, int W, int H) FL_OVERRIDE;
        void move_intersection(int oldx, int oldy, int newx, int newy);
    };

} // namespace mrv

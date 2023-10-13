#pragma once

#include <FL/Fl_Input.H>

namespace mrv
{
    class Input : public Fl_Input
    {
    public:
        Input(int X, int Y, int W, int H, const char* L = 0) :
            Fl_Input(X, Y, W, H, L)
        {
            labelsize(12);
            color((Fl_Color)0xf98a8a800);
            textcolor(FL_BLACK);
            cursor_color(FL_RED);
        }
    };
} // namespace mrv

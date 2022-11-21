#pragma once


#include <FL/Fl_Button.H>


namespace mrv
{
    class ColorButton : public Fl_Button
    {
    public:
        ColorButton( int X, int Y, int W, int H, const char* L = 0 ) :
            Fl_Button( X, Y, W, H, L )
            {
            }

    };
}

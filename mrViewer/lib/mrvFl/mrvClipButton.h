#pragma once

#include <FL/Fl_Button.H>


namespace mrv
{
    class ClipButton : public Fl_Button
    {
    public:
        ClipButton( int X, int Y, int W, int H, const char* L = 0 ) :
            Fl_Button( X, Y, W, H, L )
            {
            }

        int handle( int event ) override;
    };
}

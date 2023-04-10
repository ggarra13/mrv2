
#pragma once

#include <FL/Fl_Menu_Bar.H>

namespace mrv
{

    class FL_EXPORT MenuBar : public Fl_Menu_Bar
    {
    public:
        MenuBar(int X, int Y, int W, int H, const char* L = 0);
        
        int handle(int) FL_OVERRIDE;
    };

}

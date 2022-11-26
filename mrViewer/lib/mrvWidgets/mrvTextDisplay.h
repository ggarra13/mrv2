#pragma once

#include <FL/Fl_Text_Display.H>

namespace mrv {

class TextDisplay : public Fl_Text_Display
{
public:
    TextDisplay( int x, int y, int w, int h, const char* l = 0 );
    ~TextDisplay();

};

}


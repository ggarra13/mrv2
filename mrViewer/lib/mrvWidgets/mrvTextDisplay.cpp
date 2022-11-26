

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl.H>

#include "mrvWidgets/mrvTextDisplay.h"

namespace mrv {


TextDisplay::TextDisplay( int x, int y, int w, int h, const char* l  ) :
Fl_Text_Display( x, y, w, h, l )
{
    color( FL_GRAY0 );
    buffer( new Fl_Text_Buffer() );
}

TextDisplay::~TextDisplay()
{
    Fl_Text_Buffer* b = buffer();
    buffer(NULL);
    delete b;
}


}

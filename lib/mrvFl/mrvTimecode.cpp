#include "mrvFl/mrvTimecode.h"


namespace mrv
{

Timecode::Timecode( int x, int y, int w, int h, const char* l ) :
Fl_Float_Input( x, y, w, h, l ),
uiMain( NULL )
{
    textcolor( fl_contrast( FL_BLACK, color() ) );
}


int Timecode::handle( int e )
{
    return Fl_Float_Input::handle( e );
}




}  // namespace mrv

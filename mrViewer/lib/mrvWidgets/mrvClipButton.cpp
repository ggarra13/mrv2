
#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvPreferences.h"

#include "mrvClipButton.h"

namespace mrv
{
    int ClipButton::handle( int event )
    {
        switch ( event )
        {
        case FL_FOCUS:
        case FL_UNFOCUS:
            return 1;
        case FL_KEYDOWN:
        case FL_KEYUP:
        {
            if ( value() )
            {
                unsigned rawkey = Fl::event_key();
                if ( rawkey == FL_Delete ||
                     rawkey == FL_BackSpace )
                    close_current_cb( this, Preferences::ui );
                return 1;
                break;
            }
        }
        }
        return Fl_Button::handle( event );
    }
}

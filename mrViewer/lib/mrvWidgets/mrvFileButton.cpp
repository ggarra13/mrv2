// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl.H>

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvPreferences.h"
#include "mrvFileButton.h"

namespace mrv
{
    FileButton::FileButton( int X, int Y, int W, int H, const char* L ) :
        ClipButton( X, Y, W, H, L )
    {
    }
    
    int FileButton::handle( int event )
    {
        switch ( event )
        {
        case FL_FOCUS:
        case FL_UNFOCUS:
            return 1;
        case FL_KEYDOWN:
        case FL_KEYUP:
        {
            if ( value() && Fl::focus() == this )
            {
                unsigned rawkey = Fl::event_key();
                if ( rawkey == FL_Delete ||
                     rawkey == FL_BackSpace )
                {
                    close_current_cb( this, Preferences::ui );
                    return 1;
                }
                return 0;
                break;
            }
        }
        }
        return Fl_Button::handle( event );
    }
}

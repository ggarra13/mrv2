// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvWidgets/mrvStatusBar.h"

namespace mrv
{
    void  StatusBar::clear_cb( StatusBar* o )
    {
        o->clear();
    }
    
    StatusBar::StatusBar( int X, int Y, int W, int H, const char* L ) :
        Fl_Group( X, Y, W, H, L )
    {
        box( FL_FLAT_BOX );
    }

    void  StatusBar::timeout( float seconds )
    {
        seconds_ = seconds;
    }

    void  StatusBar::clear()
    {
        Fl_Group::copy_label( "" );
        color( color_ );
        box( FL_FLAT_BOX );
        labelcolor( labelcolor_ );
        redraw();
    }

    void  StatusBar::copy_label( const char* msg )
    {
        if ( label() == NULL || strlen(label()) == 0 )
        {
            color_   = color();
            labelcolor_ = labelcolor();
            color( 0xFF000000 );
            labelcolor( FL_BLACK );
        }
        Fl_Group::copy_label( msg );
        redraw();
        Fl::add_timeout( seconds_, (Fl_Timeout_Handler) clear_cb,
                         this );
    }
    
}

// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <iostream>

#include <FL/Fl_Group.H>
#include <FL/Fl.H>

namespace mrv
{
    class StatusBar : public Fl_Group
    {
        float seconds_ = 5.0F;
        Fl_Color labelcolor_;
        Fl_Color color_;
        
        static void  clear_cb( StatusBar* o );
        
    public:
        StatusBar( int X, int Y, int W, int H, const char* L = 0 );

        void timeout( float seconds );

        void clear();

        void copy_label( const char* msg );
    };
}

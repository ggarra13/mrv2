// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvWidgets/mrvButton.h"


namespace mrv
{
    class StateButton : public Button
    {
        int state = 0;
        int max_states_ = 2;
    public:
        StateButton( int X, int Y, int W, int H, const char* L = 0 ) :
            Button( X, Y, W, H, L )
            {
            }
        void max_states(int x) { max_states_ = x; }
        virtual int handle( int e ) override
            {
                if ( e == FL_PUSH )
                {
                    ++state;
                    if ( state > max_states_ ) state = 0;
                    do_callback();
                    return 1;
                }
                return Button::handle(e);
            }
        int value() const { return state; }
        void value(int x ) { state = x; }
    };
}

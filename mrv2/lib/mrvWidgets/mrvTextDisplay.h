// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

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


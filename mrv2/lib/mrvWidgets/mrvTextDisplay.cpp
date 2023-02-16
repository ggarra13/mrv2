// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvWidgets/mrvTextDisplay.h"

#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_attr.h> // workaround till Fl_Text_Buffer.H adds this

namespace mrv {

TextDisplay::TextDisplay(int x, int y, int w, int h, const char *l)
    : Fl_Text_Display(x, y, w, h, l) {
  color(FL_GRAY0);
  buffer(new Fl_Text_Buffer());
}

TextDisplay::~TextDisplay() {
  Fl_Text_Buffer *b = buffer();
  buffer(NULL);
  delete b;
}

} // namespace mrv

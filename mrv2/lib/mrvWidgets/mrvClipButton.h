// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <iostream>

#include "FL/Fl_Button.H"

namespace mrv {
class ClipButton : public Fl_Button {
public:
  ClipButton(int X, int Y, int W, int H, const char *L = 0)
      : Fl_Button(X, Y, W, H, L) {
    box(FL_ENGRAVED_BOX);
    selection_color(FL_YELLOW);
    align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_IMAGE_NEXT_TO_TEXT);
    labelsize(12);
  }

  void draw() override {
    if (value())
      labelcolor(FL_BLACK);
    else
      labelcolor(FL_WHITE);
    Fl_Color col = value() ? selection_color() : color();
    draw_box(value() ? (down_box() ? down_box() : fl_down(box())) : box(), col);
    draw_backdrop();
    draw_label();
  }
};
} // namespace mrv

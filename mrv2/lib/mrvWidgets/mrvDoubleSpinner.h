// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Spinner.H>

namespace mrv {
class DoubleSpinner : public Fl_Spinner {
public:
  DoubleSpinner(int X, int Y, int W, int H, const char *L = 0)
      : Fl_Spinner(X, Y, W, H) {
    type(FL_FLOAT_INPUT); // float spinner
    wrap(0);              // disable wrap mode
    range(0.0, 1.0);
    step(0.01);
    color((Fl_Color)0xf98a8a800);
    textcolor(FL_BLACK);
  }

  inline void setEnabled(bool a) {
    if (a)
      activate();
    else
      deactivate();
  }
};
} // namespace mrv

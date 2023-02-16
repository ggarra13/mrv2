
// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Button.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hor_Slider.H>

namespace mrv {

class HorSlider : public Fl_Group {
protected:
  double default_value_;

public:
  Fl_Float_Input *uiValue;
  Fl_Hor_Slider *uiSlider;
  Fl_Button *uiReset;

public:
  HorSlider(int X, int Y, int W, int H, const char *L = 0);

  void default_value(double d) noexcept;
  void minimum(double mn) noexcept;
  void maximum(double mx) noexcept;
  void range(double mn, double mx) noexcept;
  void step(double s) noexcept;
  void value(double x) noexcept;
  double value() const noexcept;

  void textcolor(Fl_Color c) { uiValue->textcolor(c); }
  void textsize(int x) { uiValue->textsize(x); }
  void tooltip(const char *t);

  void check_size() noexcept;

  // C
  inline void setEnabled(bool a) {
    if (a)
      activate();
    else
      deactivate();
  }
};

} // namespace mrv

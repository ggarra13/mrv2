// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Enumerations.H>
#include <FL/Fl_Multiline_Input.H>
#include <tlCore/Vector.h>

namespace mrv {
using namespace tl;

class MultilineInput : public Fl_Multiline_Input {
public:
  MultilineInput(int X, int Y, int W, int H, char *L = 0);
  ~MultilineInput() override;

  int accept();

  int textsize() const;
  void textsize(int x);
  void recalc();

  int handle(int e) override;
  void draw() override;

public:
  double _font_size;
  std::string fontFamily = "NotoSans-Regular";
  math::Vector2i pos;
  math::Vector2i viewPos;
  double viewZoom = 1.F;
};

} // namespace mrv

// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Box.H>
#include <tlCore/Util.h>

#include "mrvFl/mrvColorAreaInfo.h"

class ViewerUI;

namespace mrv {
using namespace tl;

class Histogram : public Fl_Box {
public:
  enum Type {
    kLinear,
    kLog,
    kSqrt,
  };

  enum Channel {
    kRGB,
    kRed,
    kGreen,
    kBlue,
    kLumma,
  };

public:
  Histogram(int X, int Y, int W, int H, const char *L = 0);

  void channel(Channel c) {
    _channel = c;
    redraw();
  }
  Channel channel() const { return _channel; }

  void histogram_type(Type c) {
    _histtype = c;
    redraw();
  };
  Type histogram_type() const { return _histtype; };

  virtual void draw() override;

  void update(const area::Info &info);

  void main(ViewerUI *m) { ui = m; };
  ViewerUI *main() { return ui; };

protected:
  void draw_pixels() const noexcept;

  void count_pixel(const uint8_t *rgb) noexcept;

  inline float histogram_scale(float val, float maxVal) const noexcept;

  Channel _channel;
  Type _histtype;

  float maxLumma;
  float lumma[256];

  float maxColor;

  float red[256];
  float green[256];
  float blue[256];

  ViewerUI *ui;
};

} // namespace mrv

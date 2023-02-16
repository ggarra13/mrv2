// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <Imath/ImathVec.h>
#include <tlCore/Vector.h>

#include <vector>

namespace tl {
namespace draw {

class Point : public Imath::V2f {
public:
  Point() : Imath::V2f() {}

  Point(double xx, double yy) : Imath::V2f(xx, yy) {}

  Point(const Point &b) : Imath::V2f(b.x, b.y) {}

  Point(const tl::math::Vector2f &b) : Imath::V2f(b.x, b.y) {}

  Point(const tl::math::Vector2i &b) : Imath::V2f(b.x, b.y) {}

  Point(const Imath::V2f &b) : Imath::V2f(b.x, b.y) {}

  inline Point &operator=(const Imath::V2f &b) {
    x = b.x;
    y = b.y;
    return *this;
  }

  inline Point &operator=(const Point &b) {
    x = b.x;
    y = b.y;
    return *this;
  }

  inline double angle(const Point &b) {
    return std::acos(dot(b)) / (length() * b.length());
  }
};

typedef std::vector<Point> PointList;
} // namespace draw

} // namespace tl

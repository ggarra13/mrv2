// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

namespace tl {

namespace draw {

template <typename Vec2> struct LineSegment {
  LineSegment(const Vec2 &a, const Vec2 &b) : a(a), b(b) {}

  Vec2 a, b;

  /**
   * @return A copy of the line segment, offset by the given vector.
   */
  LineSegment operator+(const Vec2 &toAdd) const {
    return {a + toAdd, b + toAdd};
  }

  /**
   * @return A copy of the line segment, offset by the given vector.
   */
  LineSegment operator-(const Vec2 &toRemove) const {
    return {a - toRemove, b - toRemove};
  }

  /**
   * @return The line segment's normal vector.
   */
  Vec2 normal() const {
    auto dir = direction();

    // return the direction vector
    // rotated by 90 degrees counter-clockwise
    return {-dir.y, dir.x};
  }

  /**
   * @return The line segment's direction vector.
   */
  Vec2 direction(bool normalized = true) const {
    auto vec = b - a;

    return normalized ? vec.normalized() : vec;
  }

  static Vec2 *intersection(const LineSegment &a, const LineSegment &b,
                            bool infiniteLines) {
    // calculate un-normalized direction vectors
    auto r = a.direction(false);
    auto s = b.direction(false);

    auto originDist = b.a - a.a;

    auto uNumerator = originDist.cross(r);
    auto denominator = r.cross(s);

    if (std::abs(denominator) < 0.0001f) {
      // The lines are parallel
      return NULL;
    }

    // solve the intersection positions
    auto u = uNumerator / denominator;
    auto t = originDist.cross(s) / denominator;

    if (!infiniteLines && (t < 0 || t > 1 || u < 0 || u > 1)) {
      // the intersection lies outside of the line segments
      return NULL;
    }

    // calculate the intersection point
    // a.a + r * t;
    Vec2 *ret = new Vec2(a.a + r * t);
    return ret;
  }
};

} // namespace draw

} // namespace tl

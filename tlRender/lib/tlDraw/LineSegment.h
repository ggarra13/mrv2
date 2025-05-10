// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.
//
// Original code is:
//
// Copyright © 2019 Marius Metzger (CrushedPixel)

#pragma once

namespace tl
{

    namespace draw
    {

        template <typename Vec2> struct LineSegment
        {
            LineSegment(const Vec2& a, const Vec2& b) :
                a(a),
                b(b)
            {
            }

            LineSegment(
                const Vec2& a, const Vec2& b, const Vec2& uvA,
                const Vec2& uvB) :
                a(a),
                b(b)
            {
            }

            Vec2 a, b;
            Vec2 aUV, bUV;

            /**
             * @return A copy of the line segment, offset by the given vector.
             */
            LineSegment operator+(const Vec2& toAdd) const
            {
                return {a + toAdd, b + toAdd};
            }

            /**
             * @return A copy of the line segment, offset by the given vector.
             */
            LineSegment operator-(const Vec2& toRemove) const
            {
                return {a - toRemove, b - toRemove};
            }

            /**
             * @return The line segment's normal vector.
             */
            Vec2 normal() const
            {
                auto dir = direction();

                // return the direction vector
                // rotated by 90 degrees counter-clockwise
                return {-dir.y, dir.x};
            }

            /**
             * @return The line segment's direction vector.
             */
            Vec2 direction(bool normalized = true) const
            {
                auto vec = b - a;

                return normalized ? vec.normalized() : vec;
            }

            static void intersection(
                const LineSegment& a, const LineSegment& b, Vec2*& point,
                bool infiniteLines)
            {
                // Clear the pointers first
                point = nullptr;

                // calculate un-normalized direction vectors
                auto r = a.direction(false);
                auto s = b.direction(false);

                auto originDist = b.a - a.a;

                auto uNumerator = originDist.cross(r);
                auto denominator = r.cross(s);

                if (std::abs(denominator) < 0.0001f)
                {
                    // The lines are parallel
                    return;
                }

                // solve the intersection positions
                auto u = uNumerator / denominator;
                auto t = originDist.cross(s) / denominator;

                if (!infiniteLines && (t < 0 || t > 1 || u < 0 || u > 1))
                {
                    // the intersection lies outside of the line segments
                    return;
                }

                // calculate the intersection point
                // a.a + r * t;
                point = new Vec2(a.a + r * t);
            }
        };

    } // namespace draw

} // namespace tl

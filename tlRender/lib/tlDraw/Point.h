// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>

#include <tlCore/Vector.h>

#include <Imath/ImathVec.h>

namespace tl
{
    namespace draw
    {

        /**
         * @brief A 2D Point class built on top of Imath::V2d
         *
         */
        class Point : public Imath::V2d
        {
        public:
            float pressure = 1.F;

            Point() :
                Imath::V2d(), pressure(1.F)
            {
            }

            Point(double xx, double yy, float p = 1.F) :
                Imath::V2d(xx, yy), pressure(p)
            {
            }

            Point(const Point& b) :
                Imath::V2d(b.x, b.y), pressure(b.pressure)
            {
            }

            Point(const math::Vector2f& b) :
                Imath::V2d(b.x, b.y)
            {
            }

            Point(const tl::math::Vector2i& b) :
                Imath::V2d(b.x, b.y)
            {
            }

            Point(const Imath::V2d& b) :
                Imath::V2d(b.x, b.y)
            {
            }

            inline bool operator==(const Point& b)
                {
                    return (x == b.x && y == b.y);
                }

            inline bool operator!=(const Point& b)
                {
                    return !(*this == b);
                }

            inline Point& operator=(const Imath::V2d& b)
            {
                x = b.x;
                y = b.y;
                return *this;
            }

            inline Point& operator=(const Point& b)
            {
                x = b.x;
                y = b.y;
                return *this;
            }

            inline double angle(const Point& b)
            {
                return std::acos(dot(b)) / (length() * b.length());
            }
        };

        void to_json(nlohmann::json& json, const Point& value);

        void from_json(const nlohmann::json& json, Point& value);

        typedef std::vector< Point > PointList;
    } // namespace draw

} // namespace tl

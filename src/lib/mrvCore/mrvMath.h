// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Vector.h>

namespace mrv
{
    using tl::math::Vector2f;

    //! Helper function to calculate the signed area of a triangle
    inline int sign(int x1, int y1, int x2, int y2,
                    int x3, int y3)
    {
        return (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3);
    }
    
    inline float crossProduct(const Vector2f& p1,
                              const Vector2f& p2,
                              const Vector2f& p3)
    {
        return (p2[0] - p1[0]) * (p3[1] - p1[1]) -
               (p2[1] - p1[1]) * (p3[0] - p1[0]);
    }

    //! Check if a point is inside a triangle
    inline bool isPointInTriangle(const Vector2f& pt, const Vector2f& v1,
                                  const Vector2f& v2, const Vector2f& v3)
    {
        float d1 = crossProduct(pt, v1, v2);
        float d2 = crossProduct(pt, v2, v3);
        float d3 = crossProduct(pt, v3, v1);

        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(hasNeg && hasPos);
    }
    
    //! Function to check if the point (px, py) is inside a triangle formed by
    //! (x1, y1), (x2, y2), (x3, y3)
    inline bool isPointInTriangle(int px, int py, int x1, int y1, int x2,
                                  int y2, int x3, int y3)
    {
        int d1 = sign(px, py, x1, y1, x2, y2);
        int d2 = sign(px, py, x2, y2, x3, y3);
        int d3 = sign(px, py, x3, y3, x1, y1);
        
        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
        
        // True if all signs are either positive or negative
        // (point inside triangle)
        return !(hasNeg && hasPos);
    }
    
    // Function to determine which triangle the point (px, py) is in
    inline
    int checkWhichTriangle(int px, int py, int x1, int y1, int x2, int y2)
    {
        // Check top-left triangle (vertices: (x1, y1), (x2, y1), (x1, y2))
        if (isPointInTriangle(px, py, x1, y1, x2, y1, x1, y2)) {
            return 0;
        }
        
        // Check bottom-right triangle (vertices: (x2, y1), (x2, y2), (x1, y2))
        if (isPointInTriangle(px, py, x2, y1, x2, y2, x1, y2)) {
            return 1;
        }
        
        return -1;
    }

    //! Fuzzy equal comparison between ints or doubles.
    template < typename T >
    inline bool is_equal(const T x1, const T x2, const T e = 1e-5)
    {
        return (((x1 > x2) ? x1 - x2 : x2 - x1) <= e);
    }

    // Return 0, 1 or -1 depending if the rotation is +90, -90 or 0 and 180.
    inline int rotationSign(float angle)
    {
        int sign = 1;
        // Cast to integer and normalize to the range [0, 360)
        int normalized_angle = static_cast<int>(angle) % 360;
        if (normalized_angle < 0)
        {
            normalized_angle += 360;
            sign = -1;
        }

        // Check for specific multiples of 90
        return (normalized_angle == 90 || normalized_angle == 270) * sign;
    }

} // namespace mrv

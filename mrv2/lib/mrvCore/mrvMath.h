// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

namespace mrv
{

    //! Fuzzy equal comparison between floats or doubles.
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
        if (normalized_angle < 0) {
            normalized_angle += 360;
            sign = -1;
        }

        // Check for specific multiples of 90
        return (normalized_angle == 90 || normalized_angle == 270) * sign;
    }


} // namespace mrv

// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#pragma once




namespace mrv {


    template< typename T >
    inline bool is_equal( const T x1, const T x2,
                          const T e = 1e-5 )
    {
        return (((x1 > x2)? x1 - x2: x2 - x1) <= e);
    }

// Euclidian mod.  What you expect from % but you don't get as it is signed
    inline int64_t modE( int64_t D, int64_t d )
    {
        int64_t r = D%d;
        if (r < 0) {
            if (d > 0) r += d;
            else       r -= d;
        }
        return r;
    }

// Usage is Pow<int>(float).  Example: Pow<3>(2.0f) = 8.0f
    template <int n>
    static float Pow(float v) {
        static_assert(n > 0, "Power can't be negative");
        float n2 = Pow<n / 2>(v);
        return n2 * n2 * Pow<n & 1>(v);
    }
    template <> float Pow<1>(float v) {
        return v;
    }
    template <> float Pow<0>(float v) {
        return 1;
    }


}

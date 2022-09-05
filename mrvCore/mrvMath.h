/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvMath.h
 * @author gga
 * @date   Wed Aug 22 02:40:11 2007
 *
 * @brief  Auxialiary math routines
 *
 *
 */


#ifndef mrvMath_h
#define mrvMath_h

#ifndef NOMINMAX
#  define NOMINMAX
#endif



namespace mrv {

typedef float Float;

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
static Float Pow(Float v) {
    static_assert(n > 0, "Power can't be negative");
    Float n2 = Pow<n / 2>(v);
    return n2 * n2 * Pow<n & 1>(v);
}
template <> Float Pow<1>(Float v) {
    return v;
}
template <> Float Pow<0>(Float v) {
    return 1;
}


}

#endif // mrvMath_h

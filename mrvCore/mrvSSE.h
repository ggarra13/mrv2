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
 * @file   mrvSSE.h
 * @author gga
 * @date   Sat Aug 25 17:51:34 2007
 *
 * @brief
 *
 *
 */

#ifndef mrvSSE_h
#define mrvSSE_h


//
// Include SIMD Header
//

#pragma once

#ifdef LINUX
#  include "ext/fvec.h" // also contains xmmintrin.h
#elif defined(WIN32)
#  include <fvec.h>   // also contains xmmintrin.h
#elif OSX

#  define VIMAGE_H
#  include <Accelerate/Accelerate.h>


class F32vec4
{
    union {
        __m128 v;
        float f[4];
    };

public:
  F32vec4( float a, float b, float c, float d )
  {
    f[0] = d;
    f[1] = c;
    f[2] = b;
    f[3] = a;
  }

  inline float operator[](int i) const
  {
    return f[i];
  }

  inline F32vec4& operator*=( float x )
  {
    v *= x;
    return *this;
  }

  inline F32vec4& operator*=( const F32vec4& x )
  {
      v *= x.v;
      return *this;
  }

    inline operator __m128() const
        {
            return v;
        }

    inline operator __m128()
        {
            return v;
        }
};


inline F32vec4 select_lt( const F32vec4& a, const F32vec4& b,
                          const F32vec4& c, const F32vec4& d )
{
  return F32vec4( a[0] < b[0] ? c[0] : d[0],
                  a[1] < b[1] ? c[1] : d[1],
                  a[2] < b[2] ? c[2] : d[2],
                  a[3] < b[3] ? c[3] : d[3] );
}

inline F32vec4 select_gt( const F32vec4& a, const F32vec4& b,
                          const F32vec4& c, const F32vec4& d )
{
  return F32vec4( a[0] > b[0] ? c[0] : d[0],
                  a[1] > b[1] ? c[1] : d[1],
                  a[2] > b[2] ? c[2] : d[2],
                  a[3] > b[3] ? c[3] : d[3] );
}


#endif

#else

#if defined(WIN32) || defined(WIN64)
#pragma message( "Warning: Not using SSE" )
#else
#warning Not using SSE
#endif

class F32vec4
{
  float f[4];
public:
  F32vec4( float a, float b, float c, float d )
  {
    f[0] = d;
    f[1] = c;
    f[2] = b;
    f[3] = a;
  }

  inline float operator[](int i) const
  {
    return f[i];
  }

  inline F32vec4& operator*=( float x )
  {
    f[0] *= x;
    f[1] *= x;
    f[2] *= x;
    f[3] *= x;
    return *this;
  }

  inline F32vec4& operator*=( const F32vec4& x )
  {
    f[0] *= x.f[0];
    f[1] *= x.f[1];
    f[2] *= x.f[2];
    f[3] *= x.f[3];
    return *this;
  }
};


inline F32vec4 select_lt( const F32vec4& a, const F32vec4& b,
                          const F32vec4& c, const F32vec4& d )
{
  return F32vec4( a[0] < b[0] ? c[0] : d[0],
                  a[1] < b[1] ? c[1] : d[1],
                  a[2] < b[2] ? c[2] : d[2],
                  a[3] < b[3] ? c[3] : d[3] );
}

inline F32vec4 select_gt( const F32vec4& a, const F32vec4& b,
                          const F32vec4& c, const F32vec4& d )
{
  return F32vec4( a[0] > b[0] ? c[0] : d[0],
                  a[1] > b[1] ? c[1] : d[1],
                  a[2] > b[2] ? c[2] : d[2],
                  a[3] > b[3] ? c[3] : d[3] );
}


#endif

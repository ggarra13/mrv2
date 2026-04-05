// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.
//
// Some modifications:
// Copyright Contributors to the feather-tk project.

#include <cmath>

namespace tl
{
    namespace math
    {
        template<int C, typename T>
        constexpr Vector<C, T>::Vector()
        {
            for (int c = 0; c < C; ++c)
            {
                e[c] = T(0);
            }
        }
        
        template<int C, typename T>
        constexpr Vector<C, T>::Vector(const Size<C, T>& size)
        {
            for (int c = 0; c < C; ++c)
            {
                e[c] = size[c];
            }
        }

        template<int C, typename T>
        constexpr T Vector<C, T>::operator [] (int c) const
        {
            return e[c];
        }

        template<int C, typename T>
        constexpr T& Vector<C, T>::operator [] (int c)
        {
            return e[c];
        }

        template<int C, typename T>
        constexpr const T* Vector<C, T>::data() const
        {
            return e.data();
        }

        template<int C, typename T>
        constexpr T* Vector<C, T>::data()
        {
            return e.data();
        }

        template<typename T>
        constexpr Vector<2, T>::Vector() :
            e({ T(0), T(0) })
        {}

        template<typename T>
        constexpr Vector<2, T>::Vector(T x, T y) :
            e({ x, y })
        {}

        template<typename T>
        constexpr Vector<2, T>::Vector(const Size<2, T>& size) :
            e({ size.w, size.h })
        {}

        template<typename T>
        constexpr Vector<2, T>::Vector(const Vector<2, T>& v) :
            e(v.e)
        {}

        template<typename T>
        constexpr T Vector<2, T>::operator [] (int c) const
        {
            return e[c];
        }

        template<typename T>
        constexpr T& Vector<2, T>::operator [] (int c)
        {
            return e[c];
        }

        template<typename T>
        constexpr const T* Vector<2, T>::data() const
        {
            return e.data();
        }

        template<typename T>
        constexpr T* Vector<2, T>::data()
        {
            return e.data();
        }
            
        template<typename T>
        constexpr Vector<2, T>& Vector<2, T>::operator = (const Vector<2, T>& v)
        {
            e = v.e;
            return *this;
        }

        template<typename T>
        constexpr Vector<3, T>::Vector() :
            e({ T(0), T(0), T(0) })
        {}

        template<typename T>
        constexpr Vector<3, T>::Vector(T x, T y, T z) :
            e({ x, y, z })
        {}

        template<typename T>
        constexpr Vector<3, T>::Vector(const Size<3, T>& size) :
            e({ size.w, size.h, size.d })
        {}

        template<typename T>
        constexpr Vector<3, T>::Vector(const Vector<3, T>& v) :
            e(v.e)
        {}        

        template<typename T>
        constexpr T Vector<3, T>::operator [] (int c) const
        {
            return e[c];
        }

        template<typename T>
        constexpr T& Vector<3, T>::operator [] (int c)
        {
            return e[c];
        }

        template<typename T>
        constexpr const T* Vector<3, T>::data() const
        {
            return e.data();
        }

        template<typename T>
        constexpr T* Vector<3, T>::data()
        {
            return e.data();
        }
            
        template<typename T>
        constexpr Vector<3, T>& Vector<3, T>::operator = (const Vector<3, T>& v)
        {
            e = v.e;
            return *this;
        }

        template<typename T>
        constexpr Vector<4, T>::Vector() :
            e({ T(0), T(0), T(0), T(0) })
        {}

        template<typename T>
        constexpr Vector<4, T>::Vector(T x, T y, T z, T w) :
            e({ x, y, z, w })
        {}

        template<typename T>
        constexpr Vector<4, T>::Vector(const Vector<4, T>& v) :
            e(v.e)
        {}        

        template<typename T>
        constexpr T Vector<4, T>::operator [] (int c) const
        {
            return e[c];
        }

        template<typename T>
        constexpr T& Vector<4, T>::operator [] (int c)
        {
            return e[c];
        }

        template<typename T>
        constexpr const T* Vector<4, T>::data() const
        {
            return e.data();
        }

        template<typename T>
        constexpr T* Vector<4, T>::data()
        {
            return e.data();
        }
            
        template<typename T>
        constexpr Vector<4, T>& Vector<4, T>::operator = (const Vector<4, T>& v)
        {
            e = v.e;
            return *this;
        }

        template<int C, typename T>
        inline T length(const Vector<C, T>& v)
        {
            T s = T(0);
            for (int c = 0; c < C; ++c)
            {
                s += v[c] * v[c];
            }
            return std::sqrt(s);
        }
        
        template<int C, typename T>
        inline Vector<C, T> normalize(const Vector<C, T>& v)
        {
            Vector<C, T> out = v;
            const T l = length(v);
            if(l > T(0))
            {
                for (int c = 0; c < C; ++c)
                {
                    out[c] /= l;
                }
            }
            return out;
        }

        template<int C, typename T>
        constexpr T dot(const Vector<C, T>& v0, const Vector<C, T>& v1)
        {
            T out = T(0);
            for (int c = 0; c < C; ++c)
            {
                out += v0[c] * v1[c];
            }
            return out;
        }

        template<typename T>
        constexpr Vector<3, T> cross(const Vector<3, T>& a, const Vector<3, T>& b)
        {
            return Vector<3, T>(
                a.y * b.z - b.y * a.z,
                a.z * b.x - b.z * a.x,
                a.x * b.y - b.x * a.y);
        }

        template<typename T>
        constexpr Vector<2, T> perpCW(const Vector<2, T>& v)
        {
            return Vector<2, T>(v[1], -v[0]);
        }

        template<typename T>
        constexpr Vector<2, T> perpCCW(const Vector<2, T>& v)
        {
            return Vector<2, T>(-v[1], v[0]);
        }

        constexpr Vector<2, float> convert(const Vector<2, int>& value)
        {
            return Vector<2, float>(value.x, value.y);
        }

        inline Vector<2, float> round(const Vector<2, float>& value)
        {
            return Vector<2, float>(std::round(value.x), std::round(value.y));
        }

        inline Vector<2, float> floor(const Vector<2, float>& value)
        {
            return Vector<2, float>(std::floor(value.x), std::floor(value.y));
        }

        inline Vector<2, float> ceil(const Vector<2, float>& value)
        {
            return Vector<2, float>(std::ceil(value.x), std::ceil(value.y));
        }

        template<int C, typename T>
        constexpr Vector<C, T> operator + (const Vector<C, T>& v0, const Vector<C, T>& v1)
        {
            Vector<C, T> out;
            for (int c = 0; c < C; ++c)
            {
                out[c] = v0[c] + v1[c];
            }
            return out;
        }

        template<int C, typename T>
        constexpr Vector<C, T> operator + (const Vector<C, T>& v, T s)
        {
            Vector<C, T> out;
            for (int c = 0; c < C; ++c)
            {
                out[c] = v[c] + s;
            }
            return out;
        }

        template<int C, typename T>
        constexpr Vector<C, T> operator - (const Vector<C, T>& v0, const Vector<C, T>& v1)
        {
            Vector<C, T> out;
            for (int c = 0; c < C; ++c)
            {
                out[c] = v0[c] - v1[c];
            }
            return out;
        }

        template<int C, typename T>
        constexpr Vector<C, T> operator - (const Vector<C, T>& v)
        {
            Vector<C, T> out;
            for (int c = 0; c < C; ++c)
            {
                out[c] = -v[c];
            }
            return out;
        }

        template<int C, typename T>
        constexpr Vector<C, T> operator - (const Vector<C, T>& v, T s)
        {
            Vector<C, T> out;
            for (int c = 0; c < C; ++c)
            {
                out[c] = v[c] - s;
            }
            return out;
        }

        template<int C, typename T>
        constexpr Vector<C, T> operator * (const Vector<C, T>& v, float s)
        {
            Vector<C, T> out;
            for (int c = 0; c < C; ++c)
            {
                out[c] = v[c] * s;
            }
            return out;
        }

        template<int C, typename T>
        constexpr Vector<C, T> operator / (const Vector<C, T>& v, float s)
        {
            Vector<C, T> out;
            for (int c = 0; c < C; ++c)
            {
                out[c] = v[c] / s;
            }
            return out;
        }

        template<int C, typename T>
        constexpr bool operator == (const Vector<C, T>& a, const Vector<C, T>& b)
        {
            bool out = true;
            for (int c = 0; c < C; ++c)
            {
                out &= a[c] == b[c];
            }
            return out;
        }
        
        template<int C, typename T>
        constexpr bool operator != (const Vector<C, T>& a, const Vector<C, T>& b)
        {
            return !(a == b);
        }
        
        template<int C, typename T>
        inline std::ostream& operator << (std::ostream& os, const Vector<C, T>& v)
        {
            os << to_string(v);
            return os;
        }
        
        template<int C, typename T>
        inline std::istream& operator >> (std::istream& is, Vector<C, T>& v)
        {
            for (int i = 0; i < C; ++i)
                is >> v[i];
            return is;
        }
    }
}

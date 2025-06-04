// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace math
    {
        template <>
        constexpr Size2<int>::Size2() :
            w(0),
            h(0)
        {
        }

        template <>
        constexpr Size2<float>::Size2() :
            w(0.F),
            h(0.F)
        {
        }

        template <typename T>
        constexpr Size2<T>::Size2(T w, T h) :
            w(w),
            h(h)
        {
        }

        template <typename T> constexpr bool Size2<T>::isValid() const
        {
            return w > T(0) && h > T(0);
        }

        template <typename T> inline void Size2<T>::zero()
        {
            w = T(0);
            h = T(0);
        }

        template <typename T> constexpr float Size2<T>::getArea() const
        {
            return w * h;
        }

        template <typename T> constexpr float Size2<T>::getAspect() const
        {
            return h > T(0) ? (w / static_cast<float>(h)) : 0.F;
        }

        template <typename T>
        constexpr bool Size2<T>::operator==(const Size2<T>& other) const
        {
            return w == other.w && h == other.h;
        }

        template <typename T>
        constexpr bool Size2<T>::operator!=(const Size2<T>& other) const
        {
            return !(*this == other);
        }

        template <typename T>
        inline bool Size2<T>::operator<(const Size2<T>& other) const
        {
            return std::tie(w, h) < std::tie(other.w, other.h);
        }

        template <typename T>
        inline bool Size2<T>::operator>(const Size2<T>& other) const
        {
            return std::tie(w, h) > std::tie(other.w, other.h);
        }

        template <typename T>
        inline Size2<T> operator+(const Size2<T>& a, const Size2<T>& b)
        {
            return Size2<T>(a.w + b.w, a.h + b.h);
        }

        template <typename T> inline Size2<T> operator+(const Size2<T>& a, T b)
        {
            return Size2<T>(a.w + b, a.h + b);
        }

        template <typename T>
        inline Size2<T> operator-(const Size2<T>& a, const Size2<T>& b)
        {
            return Size2<T>(a.w - b.w, a.h - b.h);
        }

        template <typename T> inline Size2<T> operator-(const Size2<T>& a, T b)
        {
            return Size2<T>(a.w - b, a.h - b);
        }

        inline Size2i operator*(const Size2i& a, float b)
        {
            return Size2i(static_cast<int>(a.w * b), static_cast<int>(a.h * b));
        }

        inline Size2f operator*(const Size2f& a, float b)
        {
            return Size2f(a.w * b, a.h * b);
        }

        inline Size2i operator/(const Size2i& a, float b)
        {
            return Size2i(static_cast<int>(a.w / b), static_cast<int>(a.h / b));
        }

        inline Size2f operator/(const Size2f& a, float b)
        {
            return Size2f(a.w / b, a.h / b);
        }
    } // namespace math
} // namespace tl

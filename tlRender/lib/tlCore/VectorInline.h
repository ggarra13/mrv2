// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <cmath>

namespace tl
{
    namespace math
    {
        template <>
        constexpr Vector2<int>::Vector2() :
            x(0),
            y(0)
        {
        }

        template <>
        constexpr Vector2<float>::Vector2() :
            x(0.F),
            y(0.F)
        {
        }

        template <typename T>
        constexpr Vector2<T>::Vector2(T x, T y) :
            x(x),
            y(y)
        {
        }

        template <>
        constexpr Vector3<float>::Vector3() :
            x(0.F),
            y(0.F),
            z(0.F)
        {
        }

        template <typename T>
        constexpr Vector3<T>::Vector3(T x, T y, T z) :
            x(x),
            y(y),
            z(z)
        {
        }

        template <>
        constexpr Vector4<float>::Vector4() :
            x(0.F),
            y(0.F),
            z(0.F),
            w(0.F)
        {
        }

        template <typename T>
        constexpr Vector4<T>::Vector4(T x, T y, T z, T w) :
            x(x),
            y(y),
            z(z),
            w(w)
        {
        }

        template <typename T> inline void Vector2<T>::zero()
        {
            x = y = T(0);
        }

        template <typename T> inline void Vector3<T>::zero()
        {
            x = y = z = T(0);
        }

        template <typename T> inline void Vector4<T>::zero()
        {
            x = y = z = w = T(0);
        }

        template <typename T>
        constexpr T Vector2<T>::operator[](int index) const
        {
            return 0 == index ? x : y;
        }

        template <typename T> T& Vector2<T>::operator[](int index)
        {
            return 0 == index ? x : y;
        }

        template <typename T>
        constexpr T Vector3<T>::operator[](int index) const
        {
            return 0 == index ? x : (1 == index ? y : z);
        }

        template <typename T> T& Vector3<T>::operator[](int index)
        {
            return 0 == index ? x : (1 == index ? y : z);
        }

        template <typename T>
        constexpr T Vector4<T>::operator[](int index) const
        {
            return 0 == index ? x : (1 == index ? y : (2 == index ? z : w));
        }

        template <typename T> T& Vector4<T>::operator[](int index)
        {
            return 0 == index ? x : (1 == index ? y : (2 == index ? z : w));
        }

        template <typename T>
        constexpr bool Vector2<T>::operator==(const Vector2<T>& other) const
        {
            return x == other.x && y == other.y;
        }

        template <typename T>
        constexpr bool Vector2<T>::operator!=(const Vector2<T>& other) const
        {
            return !(*this == other);
        }

        template <typename T>
        constexpr bool Vector3<T>::operator==(const Vector3<T>& other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }

        template <typename T>
        constexpr bool Vector3<T>::operator!=(const Vector3<T>& other) const
        {
            return !(*this == other);
        }

        template <typename T>
        constexpr bool Vector4<T>::operator==(const Vector4<T>& other) const
        {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        template <typename T>
        constexpr bool Vector4<T>::operator!=(const Vector4<T>& other) const
        {
            return !(*this == other);
        }

        template <typename T> inline float length(const Vector2<T>& value)
        {
            return std::sqrt(value.x * value.x + value.y * value.y);
        }

        template <typename T> inline float length(const Vector3<T>& value)
        {
            return std::sqrt(
                value.x * value.x + value.y * value.y + value.z * value.z);
        }

        template <typename T>
        inline Vector2<T> operator+(const Vector2<T>& a, const Vector2<T>& b)
        {
            return Vector2<T>(a.x + b.x, a.y + b.y);
        }

        template <typename T>
        inline Vector3<T> operator+(const Vector3<T>& a, const Vector3<T>& b)
        {
            return Vector3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        template <typename T>
        inline Vector4<T> operator+(const Vector4<T>& a, const Vector4<T>& b)
        {
            return Vector4<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        }

        template <typename T>
        inline Vector2<T> operator+(const Vector2<T>& a, T b)
        {
            return Vector2<T>(a.x + b, a.y + b);
        }

        template <typename T>
        inline Vector3<T> operator+(const Vector3<T>& a, T b)
        {
            return Vector3<T>(a.x + b, a.y + b, a.z + b);
        }

        template <typename T>
        inline Vector4<T> operator+(const Vector4<T>& a, T b)
        {
            return Vector4<T>(a.x + b, a.y + b, a.z + b, a.w + b);
        }

        template <typename T>
        inline Vector2<T> operator-(const Vector2<T>& a, const Vector2<T>& b)
        {
            return Vector2<T>(a.x - b.x, a.y - b.y);
        }

        template <typename T>
        inline Vector3<T> operator-(const Vector3<T>& a, const Vector3<T>& b)
        {
            return Vector3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
        }

        template <typename T>
        inline Vector4<T> operator-(const Vector4<T>& a, const Vector4<T>& b)
        {
            return Vector4<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        }

        template <typename T>
        inline Vector2<T> operator-(const Vector2<T>& a, T b)
        {
            return Vector2<T>(a.x - b, a.y - b);
        }

        template <typename T>
        inline Vector3<T> operator-(const Vector3<T>& a, T b)
        {
            return Vector3<T>(a.x - b, a.y - b, a.z - b);
        }

        template <typename T>
        inline Vector4<T> operator-(const Vector4<T>& a, T b)
        {
            return Vector4<T>(a.x - b, a.y - b, a.z - b, a.w - b);
        }

        inline Vector2i operator*(const Vector2i& a, float b)
        {
            return Vector2i(
                static_cast<int>(a.x * b), static_cast<int>(a.y * b));
        }

        inline Vector2f operator*(const Vector2f& a, float b)
        {
            return Vector2f(a.x * b, a.y * b);
        }

        inline Vector3f operator*(const Vector3f& a, float b)
        {
            return Vector3f(a.x * b, a.y * b, a.z * b);
        }

        inline Vector4f operator*(const Vector4f& a, float b)
        {
            return Vector4f(a.x * b, a.y * b, a.z * b, a.w * b);
        }

        inline Vector2i operator/(const Vector2i& a, float b)
        {
            return Vector2i(
                static_cast<int>(a.x / b), static_cast<int>(a.y / b));
        }

        inline Vector2f operator/(const Vector2f& a, float b)
        {
            return Vector2f(a.x / b, a.y / b);
        }

        inline Vector3f operator/(const Vector3f& a, float b)
        {
            return Vector3f(a.x / b, a.y / b, a.z / b);
        }

        inline Vector4f operator/(const Vector4f& a, float b)
        {
            return Vector4f(a.x / b, a.y / b, a.z / b, a.w / b);
        }
    }; // namespace math
} // namespace tl

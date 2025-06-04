// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Math.h>

namespace tl
{
    namespace math
    {
        template <typename T> constexpr Matrix3x3<T>::Matrix3x3()
        {
            e[0] = 1.F;
            e[1] = 0.F;
            e[2] = 0.F;
            e[3] = 0.F;
            e[4] = 1.F;
            e[5] = 0.F;
            e[6] = 0.F;
            e[7] = 0.F;
            e[8] = 1.F;
        }

        template <typename T>
        constexpr Matrix3x3<T>::Matrix3x3(
            T e0, T e1, T e2, T e3, T e4, T e5, T e6, T e7, T e8)
        {
            e[0] = e0;
            e[1] = e1;
            e[2] = e2;
            e[3] = e3;
            e[4] = e4;
            e[5] = e5;
            e[6] = e6;
            e[7] = e7;
            e[8] = e8;
        }

        template <typename T> constexpr Matrix4x4<T>::Matrix4x4()
        {
            e[0] = 1.F;
            e[1] = 0.F;
            e[2] = 0.F;
            e[3] = 0.F;
            e[4] = 0.F;
            e[5] = 1.F;
            e[6] = 0.F;
            e[7] = 0.F;
            e[8] = 0.F;
            e[9] = 0.F;
            e[10] = 1.F;
            e[11] = 0.F;
            e[12] = 0.F;
            e[13] = 0.F;
            e[14] = 0.F;
            e[15] = 1.F;
        }

        template <typename T>
        constexpr Matrix4x4<T>::Matrix4x4(
            T e0, T e1, T e2, T e3, T e4, T e5, T e6, T e7, T e8, T e9, T e10,
            T e11, T e12, T e13, T e14, T e15)
        {
            e[0] = e0;
            e[1] = e1;
            e[2] = e2;
            e[3] = e3;
            e[4] = e4;
            e[5] = e5;
            e[6] = e6;
            e[7] = e7;
            e[8] = e8;
            e[9] = e9;
            e[10] = e10;
            e[11] = e11;
            e[12] = e12;
            e[13] = e13;
            e[14] = e14;
            e[15] = e15;
        }

        template <typename T>
        constexpr bool Matrix3x3<T>::operator==(const Matrix3x3<T>& other) const
        {
            return e[0] == other.e[0] && e[1] == other.e[1] &&
                   e[2] == other.e[2] && e[3] == other.e[3] &&
                   e[4] == other.e[4] && e[5] == other.e[5] &&
                   e[6] == other.e[6] && e[7] == other.e[7] &&
                   e[8] == other.e[8];
        }

        template <typename T>
        constexpr bool Matrix3x3<T>::operator!=(const Matrix3x3<T>& other) const
        {
            return !(*this == other);
        }

        template <typename T>
        constexpr bool Matrix4x4<T>::operator==(const Matrix4x4<T>& other) const
        {
            return e[0] == other.e[0] && e[1] == other.e[1] &&
                   e[2] == other.e[2] && e[3] == other.e[3] &&
                   e[4] == other.e[4] && e[5] == other.e[5] &&
                   e[6] == other.e[6] && e[7] == other.e[7] &&
                   e[8] == other.e[8] && e[9] == other.e[9] &&
                   e[10] == other.e[10] && e[11] == other.e[11] &&
                   e[12] == other.e[12] && e[13] == other.e[13] &&
                   e[14] == other.e[14] && e[15] == other.e[15];
        }

        template <typename T>
        constexpr bool Matrix4x4<T>::operator!=(const Matrix4x4<T>& other) const
        {
            return !(*this == other);
        }

        template <typename T>
        constexpr Matrix3x3<T> translate(const Vector2<T>& value)
        {
            return Matrix3x3<T>(
                T(1), T(0), T(0), T(0), T(1), T(0), value.x, value.y, T(1));
        }

        template <typename T>
        constexpr Matrix4x4<T> translate(const Vector3<T>& value)
        {
            return Matrix4x4<T>(
                T(1), T(0), T(0), T(0), T(0), T(1), T(0), T(0), T(0), T(0),
                T(1), T(0), value.x, value.y, value.z, T(1));
        }

        template <typename T> inline Matrix4x4<T> rotateX(T angle)
        {
            const T a = std::cos(deg2rad(angle));
            const T b = std::sin(deg2rad(angle));
            return Matrix4x4<T>(
                T(1), T(0), T(0), T(0), T(0), a, b, T(0), T(0), -b, a, T(0),
                T(0), T(0), T(0), T(1));
        }

        template <typename T> inline Matrix4x4<T> rotateY(T angle)
        {
            const T a = std::cos(deg2rad(angle));
            const T b = std::sin(deg2rad(angle));
            return Matrix4x4<T>(
                a, T(0), -b, T(0), T(0), T(1), T(0), T(0), b, T(0), a, T(0),
                T(0), T(0), T(0), T(1));
        }

        template <typename T> inline Matrix4x4<T> rotateZ(T angle)
        {
            const T a = std::cos(deg2rad(angle));
            const T b = std::sin(deg2rad(angle));
            return Matrix4x4<T>(
                a, -b, T(0), T(0), b, a, T(0), T(0), T(0), T(0), T(1), T(0),
                T(0), T(0), T(0), T(1));
        }

        template <typename T>
        constexpr Matrix4x4<T> scale(const Vector3<T>& value)
        {
            return Matrix4x4<T>(
                value.x, T(0), T(0), T(0), T(0), value.y, T(0), T(0), T(0),
                T(0), value.z, T(0), T(0), T(0), T(0), T(1));
        }

        template <typename T>
        constexpr Matrix4x4<T>
        ortho(T left, T right, T bottom, T top, T nearClip, T farClip)
        {
            const T a = T(2) / (right - left);
            const T b = T(2) / (top - bottom);
            const T c = T(-2) / (farClip - nearClip);
            const T x = -(right + left) / (right - left);
            const T y = -(top + bottom) / (top - bottom);
            const T z = -(farClip + nearClip) / (farClip - nearClip);
            return Matrix4x4<T>(
                a, T(0), T(0), T(0), T(0), b, T(0), T(0), T(0), T(0), c, T(0),
                x, y, z, T(1));
        }

        template <typename T>
        constexpr Matrix4x4<T>
        perspective(T fov, T aspect, T nearClip, T farClip)
        {
            const T f = T(1) / std::tan(deg2rad(fov) / T(2));
            const T a = f / aspect;
            const T b = (farClip + nearClip) / (nearClip - farClip);
            const T c = T(2) * farClip * nearClip / (nearClip - farClip);
            return Matrix4x4<T>(
                a, T(0), T(0), T(0), T(0), f, T(0), T(0), T(0), T(0), b, T(-1),
                T(0), T(0), c, T(0));
        }

        template <typename T>
        inline Matrix3x3<T>
        operator*(const Matrix3x3<T>& a, const Matrix3x3<T>& b)
        {
            Matrix3x3<T> out;
            for (int i = 0; i < 3; ++i)
            {
                for (int j = 0; j < 3; ++j)
                {
                    float tmp = 0.F;
                    for (int k = 0; k < 3; ++k)
                    {
                        tmp += b.e[i * 3 + k] * a.e[k * 3 + j];
                    }
                    out.e[i * 3 + j] = tmp;
                }
            }
            return out;
        }

        template <typename T>
        inline Vector2<T> operator*(const Matrix3x3<T>& a, const Vector2<T>& v)
        {
            const T x = v[0] * a.e[0] + v[1] * a.e[3] + a.e[6];
            const T y = v[0] * a.e[1] + v[1] * a.e[4] + a.e[7];
            return Vector2<T>(x, y);
        }

        template <typename T>
        inline Matrix4x4<T>
        operator*(const Matrix4x4<T>& a, const Matrix4x4<T>& b)
        {
            Matrix4x4<T> out;
            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    float tmp = 0.F;
                    for (int k = 0; k < 4; ++k)
                    {
                        tmp += b.e[i * 4 + k] * a.e[k * 4 + j];
                    }
                    out.e[i * 4 + j] = tmp;
                }
            }
            return out;
        }

        template <typename T>
        inline Vector3<T> operator*(const Matrix4x4<T>& a, const Vector3<T>& v)
        {
            const T x = v[0] * a.e[0] + v[1] * a.e[4] + v[2] * a.e[8] + a.e[12];
            const T y = v[0] * a.e[1] + v[1] * a.e[5] + v[2] * a.e[9] + a.e[13];
            const T z =
                v[0] * a.e[2] + v[1] * a.e[6] + v[2] * a.e[10] + a.e[14];
            const T w =
                v[0] * a.e[3] + v[1] * a.e[7] + v[2] * a.e[11] + a.e[15];
            return Vector3<T>(x / w, y / w, z / w);
        }

        template <typename T>
        inline Vector4<T> operator*(const Matrix4x4<T>& a, const Vector4<T>& v)
        {
            Vector4<T> out;
            out.x =
                v[0] * a.e[0] + v[1] * a.e[4] + v[2] * a.e[8] + v[3] * a.e[12];
            out.y =
                v[0] * a.e[1] + v[1] * a.e[5] + v[2] * a.e[9] + v[3] * a.e[13];
            out.z =
                v[0] * a.e[2] + v[1] * a.e[6] + v[2] * a.e[10] + v[3] * a.e[14];
            out.w =
                v[0] * a.e[3] + v[1] * a.e[7] + v[2] * a.e[11] + v[3] * a.e[15];
            return out;
        }
    }; // namespace math
} // namespace tl

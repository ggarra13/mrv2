// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <iostream>

namespace tl
{
    namespace math
    {
        //! Two-dimensional vector.
        template <typename T> class Vector2
        {
        public:
            constexpr Vector2();
            constexpr Vector2(T x, T y);

            T x;
            T y;

            //! \name Components
            ///@{

            void zero();

            constexpr T operator[](int) const;
            T& operator[](int);

            ///@}

            constexpr bool operator==(const Vector2<T>&) const;
            constexpr bool operator!=(const Vector2<T>&) const;
        };

        //! Three-dimensional vector.
        template <typename T> class Vector3
        {
        public:
            constexpr Vector3();
            constexpr Vector3(T x, T y, T z);

            T x;
            T y;
            T z;

            //! \name Components
            ///@{

            void zero();

            constexpr T operator[](int) const;
            T& operator[](int);

            ///@}

            constexpr bool operator==(const Vector3<T>&) const;
            constexpr bool operator!=(const Vector3<T>&) const;
        };

        //! Four-dimensional vector.
        template <typename T> class Vector4
        {
        public:
            constexpr Vector4();
            constexpr Vector4(T x, T y, T z, T w);

            T x;
            T y;
            T z;
            T w;

            //! \name Components
            ///@{

            void zero();

            constexpr T operator[](int) const;
            T& operator[](int);

            ///@}

            constexpr bool operator==(const Vector4<T>&) const;
            constexpr bool operator!=(const Vector4<T>&) const;
        };

        //! Two-dimensional integer vector.
        typedef Vector2<int> Vector2i;

        //! Two-dimensional floating point vector.
        typedef Vector2<float> Vector2f;

        //! Three-dimensional floating point vector.
        typedef Vector3<float> Vector3f;

        //! Four-dimensional floating point vector.
        typedef Vector4<float> Vector4f;

        //! Get the length of a vector.
        template <typename T> inline float length(const Vector2<T>&);

        //! Get the length of a vector.
        template <typename T> inline float length(const Vector3<T>&);

        //! \name Operators
        ///@{

        template <typename T>
        inline Vector2<T> operator+(const Vector2<T>&, const Vector2<T>&);
        template <typename T>
        inline Vector3<T> operator+(const Vector3<T>&, const Vector3<T>&);
        template <typename T>
        inline Vector4<T> operator+(const Vector4<T>&, const Vector4<T>&);

        template <typename T> inline Vector2<T> operator+(const Vector2<T>&, T);
        template <typename T> inline Vector3<T> operator+(const Vector3<T>&, T);
        template <typename T> inline Vector4<T> operator+(const Vector4<T>&, T);

        template <typename T>
        inline Vector2<T> operator-(const Vector2<T>&, const Vector2<T>&);
        template <typename T>
        inline Vector3<T> operator-(const Vector3<T>&, const Vector3<T>&);
        template <typename T>
        inline Vector4<T> operator-(const Vector4<T>&, const Vector4<T>&);

        template <typename T> inline Vector2<T> operator-(const Vector2<T>&, T);
        template <typename T> inline Vector3<T> operator-(const Vector3<T>&, T);
        template <typename T> inline Vector4<T> operator-(const Vector4<T>&, T);

        Vector2i operator*(const Vector2i&, float);
        Vector2f operator*(const Vector2f&, float);
        Vector3f operator*(const Vector3f&, float);
        Vector4f operator*(const Vector4f&, float);

        Vector2i operator/(const Vector2i&, float);
        Vector2f operator/(const Vector2f&, float);
        Vector3f operator/(const Vector3f&, float);
        Vector4f operator/(const Vector4f&, float);

        ///@}

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Vector2i&);
        void to_json(nlohmann::json&, const Vector2f&);
        void to_json(nlohmann::json&, const Vector3f&);
        void to_json(nlohmann::json&, const Vector4f&);

        void from_json(const nlohmann::json&, Vector2i&);
        void from_json(const nlohmann::json&, Vector2f&);
        void from_json(const nlohmann::json&, Vector3f&);
        void from_json(const nlohmann::json&, Vector4f&);

        std::ostream& operator<<(std::ostream&, const Vector2i&);
        std::ostream& operator<<(std::ostream&, const Vector2f&);
        std::ostream& operator<<(std::ostream&, const Vector3f&);
        std::ostream& operator<<(std::ostream&, const Vector4f&);

        std::istream& operator>>(std::istream&, Vector2i&);
        std::istream& operator>>(std::istream&, Vector2f&);
        std::istream& operator>>(std::istream&, Vector3f&);
        std::istream& operator>>(std::istream&, Vector4f&);

        ///@}
    } // namespace math
} // namespace tl

#include <tlCore/VectorInline.h>

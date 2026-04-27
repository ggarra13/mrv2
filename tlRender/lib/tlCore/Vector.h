// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.
//
// Some modifications:
// Copyright Contributors to the feather-tk project.

#pragma once

#include <tlCore/Size.h>

#include <nlohmann/json.hpp>

namespace tl
{
    namespace math
    {
        //! \name Vectors
        ///@{    
    
        //! Base class for vectors.
        template<int C, typename T>
        class Vector
        {
        public:
            constexpr Vector();
            explicit constexpr Vector(const Size<C, T>&);

            constexpr T operator [] (int) const;
            constexpr T& operator [] (int);

            constexpr const T* data() const;
            constexpr T* data();
            //! \name Components
            std::array<T, C> e;
        };

        //! Two-dimensional vector.
        template<typename T>
        class Vector<2, T>
        {
        public:
            constexpr Vector();
            constexpr Vector(T, T);
            explicit constexpr Vector(const Size<2, T>&);
            constexpr Vector(const Vector<2, T>&);

            constexpr T operator [] (int) const;
            constexpr T& operator [] (int);

            constexpr const T* data() const;
            constexpr T* data();

            union
            {
                std::array<T, 2> e;
                struct
                {
                    T x;
                    T y;
                };
            };

            constexpr Vector<2, T>& operator = (const Vector<2, T>&);
        };

        //! Three-dimensional vector.
        template<typename T>
        class Vector<3, T>
        {
        public:
            constexpr Vector();
            constexpr Vector(T, T, T);
            explicit constexpr Vector(const Size<3, T>&);
            constexpr Vector(const Vector<3, T>&);

            constexpr T operator [] (int) const;
            constexpr T& operator [] (int);
    
            constexpr const T* data() const;
            constexpr T* data();

            union
            {
                std::array<T, 3> e;
                struct
                {
                    T x;
                    T y;
                    T z;
                };
            };

            constexpr Vector<3, T>& operator = (const Vector<3, T>&);
        };

        //! Four-dimensional vector.
        template<typename T>
        class Vector<4, T>
        {
        public:
            constexpr Vector();
            constexpr Vector(T, T, T, T = T(1));
            constexpr Vector(const Vector<4, T>&);

            constexpr T operator [] (int) const;
            constexpr T& operator [] (int);

            constexpr const T* data() const;
            constexpr T* data();

            union
            {
                std::array<T, 4> e;
                struct
                {
                    T x;
                    T y;
                    T z;
                    T w;
                };
            };

            constexpr Vector<4, T>& operator = (const Vector<4, T>&);
        };

        typedef Vector<2, int> Vector2i;
        typedef Vector<2, float> Vector2f;
        typedef Vector<3, float> Vector3f;
        typedef Vector<4, float> Vector4f;

        template<int C, typename T>
        T length(const Vector<C, T>&);
        
        template<int C, typename T>
        T length2(const Vector<C, T>&);
        
        //! Normalize the given vector.
        template<int C, typename T>
        Vector<C, T> normalize(const Vector<C, T>&);        

        //! Two-dimensional integer vector.
        template<int C, typename T>
        constexpr T dot(const Vector<C, T>&, const Vector<C, T>&);

        //! Two-dimensional floating point vector.
        template<typename T>
        constexpr Vector<3, T> cross(const Vector<3, T>&, const Vector<3, T>&);

        //! Get the length of a vector.
        //! direction.
        template<typename T>
        constexpr Vector<2, T> perpCW(const Vector<2, T>&);

        //! Get a vector perpindicular to the given vector in the
        //! counter-clockwise direction.
        template<typename T>
        constexpr Vector<2, T> perpCCW(const Vector<2, T>&);

        //! Convert vector types.
        constexpr Vector<2, float> convert(const Vector<2, int>&);

        //! Round vector components.
        Vector<2, float> round(const Vector<2, float>&);

        Vector<2, float> floor(const Vector<2, float>&);

        Vector<2, float> ceil(const Vector<2, float>&);

        std::string to_string(const Vector2i&);
        std::string to_string(const Vector2f&);
        std::string to_string(const Vector3f&);
        std::string to_string(const Vector4f&);

        bool from_string(const std::string&, Vector2i&);
        bool from_string(const std::string&, Vector2f&);
        bool from_string(const std::string&, Vector3f&);
        bool from_string(const std::string&, Vector4f&);

        void to_json(nlohmann::json&, const Vector2i&);
        void to_json(nlohmann::json&, const Vector2f&);
        void to_json(nlohmann::json&, const Vector3f&);
        void to_json(nlohmann::json&, const Vector4f&);

        void from_json(const nlohmann::json&, Vector2i&);
        void from_json(const nlohmann::json&, Vector2f&);
        void from_json(const nlohmann::json&, Vector3f&);
        void from_json(const nlohmann::json&, Vector4f&);

        template<int C, typename T>
        constexpr Vector<C, T> operator + (const Vector<C, T>&, const Vector<C, T>&);
        template<int C, typename T>
        constexpr Vector<C, T> operator + (const Vector<C, T>&, T);

        template<int C, typename T>
        constexpr Vector<C, T> operator - (const Vector<C, T>&, const Vector<C, T>&);
        template<int C, typename T>
        constexpr Vector<C, T> operator - (const Vector<C, T>&);
        template<int C, typename T>
        constexpr Vector<C, T> operator - (const Vector<C, T>&, T);

        template<int C, typename T>
        constexpr Vector<C, T> operator * (const Vector<C, T>&, float);
        template<int C, typename T>
        constexpr Vector<C, T> operator / (const Vector<C, T>&, float);

        template<int C, typename T>
        constexpr bool operator == (const Vector<C, T>&, const Vector<C, T>&);
        template<int C, typename T>
        constexpr bool operator != (const Vector<C, T>&, const Vector<C, T>&);

        template<int C, typename T>
        std::ostream& operator << (std::ostream&, const Vector<C, T>&);
        
        template<int C, typename T>
        std::istream& operator >> (std::istream&, Vector<C, T>&);

        ///@}
    } // namespace math
} // namespace tl

#include <tlCore/VectorInline.h>

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
        //! Two-dimensional size.
        template <typename T> class Size2
        {
        public:
            constexpr Size2();
            constexpr Size2(T w, T h);

            T w;
            T h;

            //! \name Components
            ///@{

            constexpr bool isValid() const;

            void zero();

            ///@}

            //! \name Dimensions
            ///@{

            //! Get the area.
            constexpr float getArea() const;

            //! Get the aspect ratio.
            constexpr float getAspect() const;

            ///@}

            constexpr bool operator==(const Size2&) const;
            constexpr bool operator!=(const Size2&) const;
            bool operator<(const Size2&) const;
            bool operator>(const Size2&) const;
        };

        //! Two-dimensional integer size.
        typedef Size2<int> Size2i;

        //! Two-dimensional floating point size.
        typedef Size2<float> Size2f;

        //! \name Operators
        ///@{

        template <typename T>
        inline Size2<T> operator+(const Size2<T>&, const Size2<T>&);

        template <typename T> inline Size2<T> operator+(const Size2<T>&, T);

        template <typename T>
        inline Size2<T> operator-(const Size2<T>&, const Size2<T>&);

        template <typename T> inline Size2<T> operator-(const Size2<T>&, T);

        Size2i operator*(const Size2i&, float);

        Size2i operator/(const Size2i&, float);

        ///@}

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Size2i&);
        void to_json(nlohmann::json&, const Size2f&);

        void from_json(const nlohmann::json&, Size2i&);
        void from_json(const nlohmann::json&, Size2f&);

        std::ostream& operator<<(std::ostream&, const Size2i&);
        std::ostream& operator<<(std::ostream&, const Size2f&);

        std::istream& operator>>(std::istream&, Size2i&);
        std::istream& operator>>(std::istream&, Size2f&);

        ///@}
    } // namespace math
} // namespace tl

#include <tlCore/SizeInline.h>

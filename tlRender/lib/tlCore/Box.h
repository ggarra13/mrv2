// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Size.h>
#include <tlCore/Vector.h>

namespace tl
{
    namespace math
    {
        //! Two-dimensional axis aligned box.
        template <typename T> class Box2
        {
        public:
            constexpr Box2();
            constexpr explicit Box2(const Vector2<T>&);
            constexpr explicit Box2(const Size2<T>&);
            constexpr Box2(const Vector2<T>& min, const Vector2<T>& max);
            constexpr Box2(const Vector2<T>&, const Size2<T>&);
            constexpr Box2(T x, T y, T w, T h);

            Vector2<T> min;
            Vector2<T> max;

            //! \name Components
            ///@{

            constexpr T x() const;
            constexpr T y() const;
            constexpr T w() const;
            constexpr T h() const;

            constexpr bool isValid() const;

            void zero();

            ///@}

            //! \name Dimensions
            ///@{

            constexpr Size2<T> getSize() const;
            constexpr Vector2<T> getCenter() const;

            ///@}

            //! \name Intersections
            ///@{

            constexpr bool contains(const Box2<T>&) const;
            constexpr bool contains(const Vector2<T>&) const;

            constexpr bool intersects(const Box2<T>&) const;
            constexpr Box2<T> intersect(const Box2<T>&) const;

            ///@}

            //! \name Expand
            ///@{

            void expand(const Box2<T>&);
            void expand(const Vector2<T>&);

            ///@}

            //! \name Margin
            ///@{

            constexpr Box2<T> margin(const Vector2<T>&) const;
            constexpr Box2<T> margin(T) const;
            constexpr Box2<T> margin(T x0, T y0, T x1, T y1) const;

            ///@}

            constexpr bool operator==(const Box2<T>&) const;
            constexpr bool operator!=(const Box2<T>&) const;
        };

        //! Two-dimensional axis aligned integer box.
        typedef Box2<int> Box2i;

        //! Two-dimensional axis aligned floating point box.
        typedef Box2<float> Box2f;

        //! \name Operators
        ///@{

        Box2i operator*(const Box2i&, float);
        Box2f operator*(const Box2f&, float);

        ///@}

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Box2i&);
        void to_json(nlohmann::json&, const Box2f&);

        void from_json(const nlohmann::json&, Box2i&);
        void from_json(const nlohmann::json&, Box2f&);

        std::ostream& operator<<(std::ostream&, const Box2i&);
        std::ostream& operator<<(std::ostream&, const Box2f&);

        std::istream& operator>>(std::istream&, Box2i&);
        std::istream& operator>>(std::istream&, Box2f&);

        ///@}
    } // namespace math
} // namespace tl

#include <tlCore/BoxInline.h>

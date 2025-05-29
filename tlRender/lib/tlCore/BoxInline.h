// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <algorithm>

namespace tl
{
    namespace math
    {
        template <>
        constexpr Box2<int>::Box2() :
            min(0, 0),
            max(0, 0)
        {
        }

        template <>
        constexpr Box2<float>::Box2() :
            min(0.F, 0.F),
            max(0.F, 0.F)
        {
        }

        template <typename T>
        constexpr Box2<T>::Box2(const Vector2<T>& value) :
            min(value),
            max(value)
        {
        }

        template <>
        constexpr Box2<int>::Box2(const Size2<int>& size) :
            max(size.w - 1, size.h - 1)
        {
        }

        template <>
        constexpr Box2<float>::Box2(const Size2<float>& size) :
            max(size.w, size.h)
        {
        }

        template <typename T>
        constexpr Box2<T>::Box2(const Vector2<T>& min, const Vector2<T>& max) :
            min(min),
            max(max)
        {
        }

        template <>
        constexpr Box2<int>::Box2(
            const Vector2<int>& pos, const Size2<int>& size) :
            min(pos.x, pos.y),
            max(pos.x + size.w - 1, pos.y + size.h - 1)
        {
        }

        template <>
        constexpr Box2<float>::Box2(
            const Vector2<float>& pos, const Size2<float>& size) :
            min(pos.x, pos.y),
            max(pos.x + size.w, pos.y + size.h)
        {
        }

        template <>
        constexpr Box2<int>::Box2(int x, int y, int w, int h) :
            min(x, y),
            max(x + w - 1, y + h - 1)
        {
        }

        template <>
        constexpr Box2<float>::Box2(float x, float y, float w, float h) :
            min(x, y),
            max(x + w, y + h)
        {
        }

        template <typename T> constexpr T Box2<T>::x() const
        {
            return min.x;
        }

        template <typename T> constexpr T Box2<T>::y() const
        {
            return min.y;
        }

        template <> constexpr float Box2<float>::w() const
        {
            return max.x - min.x;
        }

        template <> constexpr int Box2<int>::w() const
        {
            return max.x - min.x + 1;
        }

        template <> constexpr float Box2<float>::h() const
        {
            return max.y - min.y;
        }

        template <> constexpr int Box2<int>::h() const
        {
            return max.y - min.y + 1;
        }

        template <typename T> constexpr bool Box2<T>::isValid() const
        {
            return min.x < max.x && min.y < max.y;
        }

        template <> inline void Box2<int>::zero()
        {
            min.x = min.y = max.x = max.y = 0;
        }

        template <> inline void Box2<float>::zero()
        {
            min.x = min.y = max.x = max.y = 0.F;
        }

        template <> constexpr Size2<int> Box2<int>::getSize() const
        {
            return Size2<int>(max.x - min.x + 1, max.y - min.y + 1);
        }

        template <> constexpr Size2<float> Box2<float>::getSize() const
        {
            return Size2<float>(max.x - min.x, max.y - min.y);
        }

        template <> constexpr Vector2<int> Box2<int>::getCenter() const
        {
            return Vector2<int>(
                min.x + static_cast<int>((max.x - min.x + 1) / 2.F),
                min.y + static_cast<int>((max.y - min.y + 1) / 2.F));
        }

        template <> constexpr Vector2<float> Box2<float>::getCenter() const
        {
            return Vector2<float>(
                min.x + (max.x - min.x) / 2.F, min.y + (max.y - min.y) / 2.F);
        }

        template <>
        constexpr bool Box2<int>::contains(const Box2<int>& value) const
        {
            return value.min.x >= min.x && value.max.x <= max.x &&
                   value.min.y >= min.y && value.max.y <= max.y;
        }

        template <>
        constexpr bool Box2<float>::contains(const Box2<float>& value) const
        {
            return value.min.x >= min.x && value.max.x <= max.x &&
                   value.min.y >= min.y && value.max.y <= max.y;
        }

        template <>
        constexpr bool Box2<int>::contains(const Vector2<int>& value) const
        {
            return value.x >= min.x && value.x < max.x && value.y >= min.y &&
                   value.y < max.y;
        }

        template <>
        constexpr bool Box2<float>::contains(const Vector2<float>& value) const
        {
            return value.x >= min.x && value.x <= max.x && value.y >= min.y &&
                   value.y <= max.y;
        }

        template <>
        constexpr bool Box2<int>::intersects(const Box2<int>& value) const
        {
            return !(
                value.max.x < min.x || value.min.x > max.x ||
                value.max.y < min.y || value.min.y > max.y);
        }

        template <>
        constexpr bool Box2<float>::intersects(const Box2<float>& value) const
        {
            return !(
                value.max.x < min.x || value.min.x > max.x ||
                value.max.y < min.y || value.min.y > max.y);
        }

        template <typename T>
        constexpr Box2<T> Box2<T>::intersect(const Box2<T>& value) const
        {
            Box2<T> out;
            out.min.x = std::max(min.x, value.min.x);
            out.min.y = std::max(min.y, value.min.y);
            out.max.x = std::min(max.x, value.max.x);
            out.max.y = std::min(max.y, value.max.y);
            return out;
        }

        template <typename T> inline void Box2<T>::expand(const Box2<T>& value)
        {
            min.x = std::min(min.x, value.min.x);
            min.y = std::min(min.y, value.min.y);
            max.x = std::max(max.x, value.max.x);
            max.y = std::max(max.y, value.max.y);
        }

        template <typename T>
        inline void Box2<T>::expand(const Vector2<T>& value)
        {
            min.x = std::min(min.x, value.x);
            min.y = std::min(min.y, value.y);
            max.x = std::max(max.x, value.x);
            max.y = std::max(max.y, value.y);
        }

        template <typename T>
        constexpr Box2<T> Box2<T>::margin(const Vector2<T>& value) const
        {
            return Box2<T>(
                Vector2<T>(min.x - value.x, min.y - value.y),
                Vector2<T>(max.x + value.x, max.y + value.y));
        }

        template <typename T> constexpr Box2<T> Box2<T>::margin(T value) const
        {
            return Box2<T>(
                Vector2<T>(min.x - value, min.y - value),
                Vector2<T>(max.x + value, max.y + value));
        }

        template <typename T>
        constexpr Box2<T> Box2<T>::margin(T x0, T y0, T x1, T y1) const
        {
            return Box2<T>(
                Vector2<T>(min.x - x0, min.y - y0),
                Vector2<T>(max.x + x1, max.y + y1));
        }

        template <typename T>
        constexpr bool Box2<T>::operator==(const Box2<T>& value) const
        {
            return min == value.min && max == value.max;
        }

        template <typename T>
        constexpr bool Box2<T>::operator!=(const Box2<T>& value) const
        {
            return !(*this == value);
        }
    } // namespace math
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <algorithm>

namespace tl
{
    namespace math
    {
        template <typename T>
        constexpr Range<T>::Range() :
            _min(static_cast<T>(0)),
            _max(static_cast<T>(0))
        {
        }

        template <typename T>
        constexpr Range<T>::Range(T minMax) :
            _min(minMax),
            _max(minMax)
        {
        }

        template <typename T>
        constexpr Range<T>::Range(T min, T max) :
            _min(std::min(min, max)),
            _max(std::max(min, max))
        {
        }

        template <typename T> constexpr T Range<T>::getMin() const
        {
            return _min;
        }

        template <typename T> constexpr T Range<T>::getMax() const
        {
            return _max;
        }

        template <> inline void Range<int>::zero()
        {
            _min = _max = 0;
        }

        template <> inline void Range<std::size_t>::zero()
        {
            _min = _max = std::size_t(0);
        }

        template <> inline void Range<float>::zero()
        {
            _min = _max = 0.F;
        }

        template <typename T> constexpr bool Range<T>::equal() const
        {
            return _min == _max;
        }
        
        template <typename T> constexpr bool Range<T>::contains(T value) const
        {
            return value >= _min && value <= _max;
        }

        template <typename T>
        constexpr bool Range<T>::intersects(const Range<T>& value) const
        {
            return !(value._max < _min || value._min > _max);
        }

        template <typename T> inline void Range<T>::expand(T value)
        {
            _min = std::min(_min, value);
            _max = std::max(_max, value);
        }

        template <typename T>
        inline void Range<T>::expand(const Range<T>& value)
        {
            _min = std::min(_min, value._min);
            _max = std::max(_max, value._max);
        }

        template<typename T>
        inline bool contains(const Range<T>& range, T value)
        {
            return value >= range.getMin() && value <= range.getMax();
        }

        template<typename T>
        inline bool intersects(const Range<T>& range, const Range<T>& value)
        {
            return !(
                value.getMax() < range.getMin() ||
                value.getMin() > range.getMax());
        }

        template<typename T>
        inline Range<T> expand(const Range<T>& range, T value)
        {
            return Range<T>(
                std::min(range.getMin(), value),
                std::max(range.getMax(), value));
        }

        template<typename T>
        inline Range<T> expand(const Range<T>& range, const Range<T>& value)
        {
            return Range<T>(
                std::min(range.getMin(), value.getMin()),
                std::max(range.getMax(), value.getMax()));
        }

        
        template <typename T>
        constexpr bool Range<T>::operator==(const Range<T>& value) const
        {
            return _min == value._min && _max == value._max;
        }

        template <typename T>
        constexpr bool Range<T>::operator!=(const Range<T>& value) const
        {
            return !(*this == value);
        }

        template <typename T>
        constexpr bool Range<T>::operator<(const Range<T>& value) const
        {
            return _min < value._min;
        }
    } // namespace math
} // namespace tl

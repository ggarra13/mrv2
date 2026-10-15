// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace time
    {
        inline bool isValid(const OTIO_NS::RationalTime& value)
        {
            return !value.is_invalid_time();
        }

        inline bool isValid(const OTIO_NS::TimeRange& value)
        {
            return !value.start_time().is_invalid_time() &&
                   !value.duration().is_invalid_time();
        }

        constexpr bool
        compareExact(const OTIO_NS::TimeRange& a, const OTIO_NS::TimeRange& b)
        {
            return a.start_time().strictly_equal(b.start_time()) &&
                   a.duration().strictly_equal(b.duration());
        }

        constexpr bool compareExact(
            const std::optional<OTIO_NS::RationalTime>& a,
            const std::optional<OTIO_NS::RationalTime>& b)
    {
        return a.has_value() && b.has_value() ?
            a.value().strictly_equal(b.value()) :
            a.has_value() == b.has_value();
    }

        constexpr bool compareExact(
            const std::optional<OTIO_NS::TimeRange>& a,
            const std::optional<OTIO_NS::TimeRange>& b)
        {
            return a.has_value() && b.has_value() ?
                compareExact(a.value(), b.value()) :
                a.has_value() == b.has_value();
        }
    } // namespace time
} // namespace tl

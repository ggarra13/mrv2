// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace time
    {
        inline bool isValid(const otime::RationalTime& value)
        {
            return !value.is_invalid_time();
        }

        inline bool isValid(const otime::TimeRange& value)
        {
            return !value.start_time().is_invalid_time() &&
                   !value.duration().is_invalid_time();
        }

        constexpr bool
        compareExact(const otime::TimeRange& a, const otime::TimeRange& b)
        {
            return a.start_time().strictly_equal(b.start_time()) &&
                   a.duration().strictly_equal(b.duration());
        }
    } // namespace time
} // namespace tl

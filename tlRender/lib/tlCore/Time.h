// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <opentimelineio/version.h>

//
// \@bug: MSVC Windows compiler cannot compile constexpr in DLLs in OTIO
//        v1.1
//
#if defined(_WINDOWS) && defined(_MSV_VER)

#  undef OPENTIME_API
#  define OPENTIME_API
#  undef OPENTIME_API_TYPE
#  define OPENTIME_API_TYPE

#  undef OTIO_API
#  define OTIO_API
#  undef OTIO_API_TYPE
#  define OTIO_API_TYPE

#endif

#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>

#include <nlohmann/json.hpp>

#include <chrono>
#include <iostream>

namespace tl
{
    namespace otime = opentime::OPENTIME_VERSION;
    namespace otio = opentimelineio::OPENTIMELINEIO_VERSION;

    //! Time
    namespace time
    {
        //! Invalid time.
        constexpr otime::RationalTime invalidTime(-1.0, -1.0);

        //! Invalid time range.
        constexpr otime::TimeRange invalidTimeRange(invalidTime, invalidTime);

        //! Check whether the given time is valid. This function should be
        //! used instead of comparing a time to the "invalidTime" constant.
        inline bool isValid(const otime::RationalTime&);

        //! Check whether the given time range is valid. This function
        //! should be used instead of comparing a time range to the
        //! "invalidTimeRange" constant.
        inline bool isValid(const otime::TimeRange&);

        //! Compare two time ranges. This function compares the values
        //! exactly, unlike the "==" operator which rescales the values.
        constexpr bool
        compareExact(const otime::TimeRange&, const otime::TimeRange&);

        //! Get the frames in a time range.
        std::vector<otime::RationalTime> frames(const otime::TimeRange&);

        //! Split a time range at into seconds.
        std::vector<otime::TimeRange> seconds(const otime::TimeRange&);

        //! Sleep for a given time.
        void sleep(const std::chrono::microseconds&);

        //! Sleep up to the given time.
        void sleep(
            const std::chrono::microseconds&,
            const std::chrono::steady_clock::time_point& t0,
            const std::chrono::steady_clock::time_point& t1);

        //! Convert a floating point rate to a rational.
        std::pair<int, int> toRational(double);

        //! \name Keycode
        ///@{

        std::string
        keycodeToString(int id, int type, int prefix, int count, int offset);

        void stringToKeycode(
            const std::string&, int& id, int& type, int& prefix, int& count,
            int& offset);

        ///@}

        //! \name Timecode
        ///@{

        void timecodeToTime(
            uint32_t, int& hour, int& minute, int& second, int& frame);

        uint32_t timeToTimecode(int hour, int minute, int second, int frame);

        std::string timecodeToString(uint32_t);

        void stringToTimecode(const std::string&, uint32_t&);

        ///@}
    } // namespace time
} // namespace tl

namespace opentime
{
    namespace OPENTIME_VERSION
    {
        void to_json(nlohmann::json&, const RationalTime&);
        void to_json(nlohmann::json&, const TimeRange&);

        void from_json(const nlohmann::json&, RationalTime&);
        void from_json(const nlohmann::json&, TimeRange&);

        std::ostream& operator<<(std::ostream&, const RationalTime&);
        std::ostream& operator<<(std::ostream&, const TimeRange&);

        std::istream& operator>>(std::istream&, RationalTime&);
        std::istream& operator>>(std::istream&, TimeRange&);
    } // namespace OPENTIME_VERSION
} // namespace opentime

#include <tlCore/TimeInline.h>

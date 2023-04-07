// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <mrvCore/mrvString.h>

#include <tlCore/Time.h>

namespace mrv
{
    using namespace tl;

    //! Time units.
    enum class TimeUnits {
        Frames,
        Seconds,
        Timecode,

        Count,
        First = Frames
    };
    TLRENDER_ENUM(TimeUnits);
    TLRENDER_ENUM_SERIALIZE(TimeUnits);

    //! Get the time units size hint string.
    std::string sizeHintString(TimeUnits);

    //! Get the time units validator regular expression.
    std::string validator(TimeUnits);

    //! Convert a time value to text.
    void
    timeToText(char* out, const otime::RationalTime&, const TimeUnits) noexcept;

    //! Convert text to a time value.
    otime::RationalTime
    textToTime(const String& text, double rate, TimeUnits, otime::ErrorStatus*);

} // namespace mrv

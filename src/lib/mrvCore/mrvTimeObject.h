// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Time.h>

#include "mrvOS/mrvString.h"

namespace mrv
{
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

    std::ostream& operator<<(std::ostream&, const TimeUnits&);
    std::istream& operator>>(std::istream&, TimeUnits&);

    //! Get the time units validator regular expression.
    std::string validator(TimeUnits);

    //! Convert a time value to text.
    void
    timeToText(char* out, const OTIO_NS::RationalTime&, const TimeUnits) noexcept;

    //! Convert text to a time value.
    OTIO_NS::RationalTime textToTime(
        const string::String& text, double rate, TimeUnits,
        opentime::ErrorStatus*);

} // namespace mrv

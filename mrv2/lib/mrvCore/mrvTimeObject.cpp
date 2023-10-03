// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <ostream>
#include <istream>
#include <array>

#include <tlCore/StringFormat.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include "mrvCore/mrvTimeObject.h"

namespace mrv
{

    TLRENDER_ENUM_IMPL(TimeUnits, "Frames", "Seconds", "Timecode");
    TLRENDER_ENUM_SERIALIZE_IMPL(TimeUnits);

    std::ostream& operator<<(std::ostream& ds, const TimeUnits& value)
    {
        ds << static_cast<int32_t>(value);
        return ds;
    }

    // std::istream& operator >> (std::istream& ds, TimeUnits& value)
    // {
    //     int32_t tmp = 0;
    //     ds >> tmp;
    //     value = static_cast<TimeUnits>(tmp);
    //     return ds;
    // }

    std::string validator(TimeUnits units)
    {
        std::string out;
        switch (units)
        {
        case TimeUnits::Frames:
            out = "[0-9]*";
            break;
        case TimeUnits::Seconds:
            out = "[0-9]*\\.[0-9]+|[0-9]+";
            break;
        case TimeUnits::Timecode:
            out = "[0-9][0-9]:[0-9][0-9]:[0-9][0-9]:[0-9][0-9]";
            break;
        default:
            break;
        }
        return out;
    }

    void timeToText(
        char* out, const otime::RationalTime& time,
        const TimeUnits units) noexcept
    {
        out[0] = 0;
        switch (units)
        {
        case TimeUnits::Frames:
            snprintf(out, 20, "%d", time::isValid(time) ? time.to_frames() : 0);
            break;
        case TimeUnits::Seconds:
        {
            snprintf(
                out, 20, "%.2f", time::isValid(time) ? time.to_seconds() : 0.0);
            break;
        }
        case TimeUnits::Timecode:
        {
            otime::ErrorStatus errorStatus;
            snprintf(
                out, 13, "%s",
                time::isValid(time) ? time.to_timecode(&errorStatus).c_str()
                                    : "00:00:00:00");
            break;
        }
        default:
            break;
        }
    }

    otime::RationalTime textToTime(
        const String& text, double rate, TimeUnits units,
        otime::ErrorStatus* errorStatus)
    {
        otime::RationalTime out = time::invalidTime;
        switch (units)
        {
        case TimeUnits::Frames:
            out = otime::RationalTime::from_frames(text.toInt(), rate);
            break;
        case TimeUnits::Seconds:
            out = otime::RationalTime::from_seconds(text.toDouble())
                      .rescaled_to(rate);
            break;
        case TimeUnits::Timecode:
            out = otime::RationalTime::from_timecode(
                text.c_str(), rate, errorStatus);
            break;
        default:
            break;
        }
        return out;
    }

} // namespace mrv

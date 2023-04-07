// SPDX-License-Identifier: BSD-3-Clause
// mrv2
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

    std::string sizeHintString(TimeUnits units)
    {
        std::string out;
        switch (units)
        {
        case TimeUnits::Frames:
            out = "000000";
            break;
        case TimeUnits::Seconds:
            out = "000000.00";
            break;
        case TimeUnits::Timecode:
            out = "00:00:00:00";
            break;
        default:
            break;
        }
        return out;
    }

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
            sprintf(out, "%d", time::isValid(time) ? time.to_frames() : 0);
            break;
        case TimeUnits::Seconds:
        {
            sprintf(out, "%.2f", time::isValid(time) ? time.to_seconds() : 0.0);
            break;
        }
        case TimeUnits::Timecode:
        {
            otime::ErrorStatus errorStatus;
            sprintf(
                out, "%s",
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

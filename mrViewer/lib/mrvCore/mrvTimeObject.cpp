// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.


#include <tlCore/StringFormat.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <mrvCore/mrvTimeObject.h>

#include <mrViewer.h>

#include <ostream>
#include <istream>

#include <array>

namespace mrv
{

    TLRENDER_ENUM_IMPL(
        TimeUnits,
        "Frames",
        "Seconds",
        "Timecode");
    TLRENDER_ENUM_SERIALIZE_IMPL(TimeUnits);

    std::ostream& operator << (std::ostream& ds, const TimeUnits& value)
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
        default: break;
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
        default: break;
        }
        return out;
    }

    String timeToText(const otime::RationalTime& time, TimeUnits units)
    {
        String out;
        switch (units)
        {
        case TimeUnits::Frames:
            out = String::fromStdString(string::Format("{0}").
                                        arg(time != time::invalidTime ? time.to_frames() : 0));
            break;
        case TimeUnits::Seconds:
            out = String::fromStdString(string::Format("{0}").
                                        arg(time != time::invalidTime ? time.to_seconds() : 0.0, 2));
            break;
        case TimeUnits::Timecode:
        {
            otime::ErrorStatus errorStatus;
            out = String::fromStdString(
                time != time::invalidTime ?
                time.to_timecode(&errorStatus) :
                "00:00:00:00");
            break;
        }
        default: break;
        }
        return out;
    }

    otime::RationalTime textToTime(
        const String& text,
        double rate,
        TimeUnits units,
        otime::ErrorStatus* errorStatus)
    {
        otime::RationalTime out = time::invalidTime;
        switch (units)
        {
        case TimeUnits::Frames:
            out = otime::RationalTime::from_frames(text.toInt(), rate);
            break;
        case TimeUnits::Seconds:
            out = otime::RationalTime::from_seconds(text.toDouble()).rescaled_to(rate);
            break;
        case TimeUnits::Timecode:
            out = otime::RationalTime::from_timecode(text.toUtf8().data(), rate, errorStatus);
            break;
        default: break;
        }
        return out;
    }

    TimeObject::TimeObject( ViewerUI* m ) :
        ui( m )
    {}

    TimeUnits TimeObject::units() const
    {
        return _units;
    }

    void TimeObject::setUnits(TimeUnits units)
    {
        if (_units == units)
            return;
        _units = units;

        ui->uiFrame->setUnits(units);
        ui->uiTimeline->setUnits(units);
        ui->uiStartFrame->setUnits(units);
        ui->uiEndFrame->setUnits(units);
    }

}

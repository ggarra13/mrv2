// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <mrvCore/mrvString.h>

#include <tlCore/Time.h>

namespace mrv
{
    using namespace tl;

    //! Time units.
    enum class TimeUnits
    {
        Frames,
        Seconds,
        Timecode,

        Count,
        First = Frames
    };
    // TLRENDER_ENUM(TimeUnits);
    // TLRENDER_ENUM_SERIALIZE(TimeUnits);
    // Q_ENUM_NS(TimeUnits);

    std::ostream& operator << (std::ostream&, const TimeUnits&);
    std::istream& operator >> (std::istream&, TimeUnits&);

    //! Get the time units size hint string.
    std::string sizeHintString(TimeUnits);

    //! Get the time units validator regular expression.
    std::string validator(TimeUnits);

    //! Convert a time value to text.
    String timeToText(const otime::RationalTime&, TimeUnits);

    //! Convert text to a time value.
    otime::RationalTime textToTime(
        const String& text,
        double rate,
        TimeUnits,
        otime::ErrorStatus*);

    //! Time object.
    class TimeObject
    {
        TimeUnits units() const;
        void setUnits( TimeUnits t );

    public:
        TimeObject();

        //! This signal is emitted when the time units are changed.
        void unitsChanged(TimeUnits);

    private:
        TimeUnits _units = TimeUnits::Timecode;
    };
}

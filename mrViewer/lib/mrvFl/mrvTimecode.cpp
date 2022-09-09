// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.


#include <mrvFl/mrvTimecode.h>

// #include <mrvFl/Util.h>


namespace mrv
{
    struct Timecode::Private
    {
        otime::RationalTime value = time::invalidTime;
        TimeUnits units = TimeUnits::Timecode;
        TimeObject* timeObject = nullptr;
    };

    Timecode::Timecode(int X, int Y, int W, int H, const char* L) :
        Fl_Float_Input( X, Y, W, H, L ),
        _p(new Private)
    {
        TLRENDER_P();

        _textUpdate();
    }

    Timecode::~Timecode()
    {}

    void Timecode::setTimeObject(TimeObject* timeObject)
    {
        TLRENDER_P();
        if (timeObject == p.timeObject)
            return;
        p.timeObject = timeObject;
        if (p.timeObject)
        {
            p.units = p.timeObject->units();
        }
        _textUpdate();
        _updateGeometry();
    }

    const otime::RationalTime& Timecode::time() const
    {
        return _p->value;
    }

    TimeUnits Timecode::units() const
    {
        return _p->units;
    }

    void Timecode::setTime(const otime::RationalTime& value) noexcept
    {
        TLRENDER_P();
        if (value.value() == p.value.value() &&
            value.rate() == p.value.rate())
            return;
        p.value = value;
        _textUpdate();
    }

    void Timecode::setUnits(TimeUnits units)
    {
        TLRENDER_P();
        if (units == p.units)
            return;
        p.units = units;
        _textUpdate();
        _updateGeometry();
    }

    void Timecode::_textUpdate() noexcept
    {
        TLRENDER_P();
        char buf[20];
        timeToText(buf, p.value, p.units);
        value( buf );
    }

    void Timecode::_updateGeometry() noexcept
    {
        TLRENDER_P();
        switch( p.units )
        {
        case TimeUnits::Frames:
            type( FL_INT_INPUT ); break;
        case TimeUnits::Seconds:
            type(FL_FLOAT_INPUT); break;
        case TimeUnits::Timecode:
        default:
            type(FL_NORMAL_INPUT); break;
        }
    }

}

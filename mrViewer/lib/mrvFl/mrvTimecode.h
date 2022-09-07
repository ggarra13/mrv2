// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <FL/Fl_Float_Input.H>

#include <mrvCore/mrvTimeObject.h>

#include <memory>

namespace mrv
{
    class TimeObject;

    //! Time label.
    class Timecode : public Fl_Float_Input
    {
    public:
        Timecode(int X, int Y, int W, int H, const char* L = 0);

        ~Timecode();

        //! Set the time object.
        void setTimeObject(TimeObject*);

        //! Get the time value.
        const otime::RationalTime& time() const;

        //! Get the time units.
        TimeUnits units() const;

    public:
        //! Set the time value.
        void setTime(const otime::RationalTime&);

        //! Set the time units.
        void setUnits(TimeUnits);

    private:
        void _textUpdate();
        void _updateGeometry();

        TLRENDER_PRIVATE();
    };
}

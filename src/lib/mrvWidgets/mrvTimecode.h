// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvCore/mrvTimeObject.h"

#include <FL/Fl_Input.H>

#include <memory>

namespace mrv
{
    //! Time label.
    class Timecode : public Fl_Input
    {
    public:
        Timecode(int X, int Y, int W, int H, const char* L = 0);

        ~Timecode();

        //! Get the time value.
        const otime::RationalTime& time() const;

        //! Get the time units.
        TimeUnits units() const;

        //! Set the time value.
        void setTime(const otime::RationalTime&) noexcept;

        //! Set the time units.
        void setUnits(TimeUnits);

        int handle(int e) FL_OVERRIDE;
        
    private:
        int eval(uchar *&s, int prio) const;
        int eval(const char *s) const;

        void _textUpdate() noexcept;
        void _updateGeometry() noexcept;

        TLRENDER_PRIVATE();
    };
} // namespace mrv

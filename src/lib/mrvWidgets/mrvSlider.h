// BSD 3-Clause License
//
// Copyright (c) 2024, mrv2 project contributors.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ---------------------------------------------------------------------
// mrv::Slider
//
// An FLTK 1.4 slider widget that adds optional logarithmic scaling and
// tick-mark drawing on top of Fl_Slider. This is an independent
// implementation written for the mrv2 project: it targets the same
// feature set that FLTK2's fltk::Slider offered (linear / logarithmic /
// squared value mapping, drawn tick marks with adaptive spacing, and
// mouse/keyboard dragging along that mapping) but the code below is a
// clean, original implementation rather than a derivative of FLTK2's
// GPL/LGPL-licensed sources.
// ---------------------------------------------------------------------

#pragma once

#include <FL/Fl_Slider.H>

#include <tlCore/Box.h>

namespace mrv
{

    //! A slider that can optionally map its track to a logarithmic scale
    //! and can draw graduated tick marks along the track.
    class Slider : public Fl_Slider
    {
    public:
        //! How a value on [minimum(), maximum()] is mapped onto the track.
        enum SliderType
        {
            kNORMAL = 0, //!< Plain linear mapping (Fl_Slider behavior).
            kLOG = 1,    //!< Logarithmic / squared mapping, see below.
        };

        //! Where tick marks are drawn relative to the track.
        enum Ticks
        {
            TICK_ABOVE = 1,
            TICK_BELOW = 2,
            TICK_BOTH = 3,
            NO_TICK = 0,
        };

        Slider(int X, int Y, int W, int H, const char* L = nullptr) :
            Fl_Slider(X, Y, W, H, L),
            m_type(kNORMAL),
            m_ticks(NO_TICK),
            m_tickColor(FL_BLACK),
            m_tickLength(4)
        {
            type(FL_HORIZONTAL);
        }

        //! True when this slider is currently using the non-linear mapping.
        bool log() const { return m_type == kLOG; }

        void ticks(Ticks t) { m_ticks = t; }
        Ticks ticks() const { return m_ticks; }

        int tick_size() const { return m_tickLength; }
        void tick_size(int px) { m_tickLength = px; }

        void tick_color(Fl_Color c) { m_tickColor = c; }
        Fl_Color tick_color() const { return m_tickColor; }

        SliderType slider_type() const { return m_type; }
        void slider_type(SliderType t) { m_type = t; }

        void draw() FL_OVERRIDE;
        int handle(int event) FL_OVERRIDE;

    protected:
        //! Map a value in [minimum(), maximum()] to a pixel offset within a
        //! track of the given pixel width, honoring the current mapping
        //! (linear/log/squared) and axis orientation.
        int slider_position(double value, int trackWidth);

        //! Inverse of slider_position(): map a pixel offset within a track
        //! of the given pixel width back to a value, snapped to a sensible
        //! multiple of step().
        double position_value(int pixelOffset, int trackWidth);

        //! Draw graduated tick marks and their numeric labels across r.
        void draw_ticks(const tl::math::Box2i& r, int minTickSpacingPx);

    private:
        //! Handles the FL_PUSH/FL_DRAG mouse-tracking math for kLOG
        //! sliders; returns the FLTK event-handled status.
        int handleDrag(int event, const tl::math::Box2i& r);

        SliderType m_type;
        Ticks m_ticks;
        Fl_Color m_tickColor;
        int m_tickLength;
    };

} // namespace mrv

//
// "$Id$"
//
// Copyright 1998-2006 by Bill Spitzak and others.
// Port to FLTK 1.4 by Gonzalo Garramuño.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php
//

#pragma once

#include <FL/Fl_Slider.H>

#include <tlCore/Box.h>

namespace mrv
{

    class Slider : public Fl_Slider
    {
    public:
        enum SliderType {
            kNORMAL = 0,
            kLOG = 1,
        };

        enum Ticks { TICK_ABOVE = 1, TICK_BELOW = 2, TICK_BOTH = 3, NO_TICK };

    public:
        Slider(int x, int y, int w, int h, const char* l = 0) :
            Fl_Slider(x, y, w, h, l),
            _slider_type(kNORMAL),
            _tick_color(FL_BLACK),
            tick_size_(4)
        {
            type(FL_HORIZONTAL);
        }

        bool log() const { return _slider_type & kLOG; }

        inline void ticks(Ticks t) { _ticks = t; }
        inline Ticks ticks() const { return _ticks; }

        inline int tick_size() const { return tick_size_; }
        inline void tick_size(int i) { tick_size_ = i; }

        inline void tick_color(Fl_Color c) { _tick_color = c; }

        inline SliderType slider_type() const { return _slider_type; }
        inline void slider_type(enum SliderType x) { _slider_type = x; }

        virtual void draw() FL_OVERRIDE;
        virtual int handle(int e) FL_OVERRIDE;

    protected:
        double position_value(int X, int w);
        int slider_position(double p, int w);
        void draw_ticks(const tl::math::Box2i& r, int min_spacing);

        SliderType _slider_type;
        Ticks _ticks;
        Fl_Color _tick_color;
        int tick_size_;
    };

} // namespace mrv

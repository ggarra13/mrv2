/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvTimeline.h
 * @author gga
 * @date   Fri Oct 13 12:58:46 2006
 *
 * @brief  An fltk widget to represent a video timeline, displaying
 *         sequence edits, timecode and frame ticks.
 *
 */

#pragma once

#include "mrvFl/mrvSlider.h"
#include "mrvFl/mrvTimecode.h"


class ViewerUI;

namespace mrv
{

class Timeline : public mrv::Slider
{
public:
    enum DisplayMode
    {
        kSingle,
        kEDL_Single,
        kEDL_All
    };

public:
    Timeline( int x, int y, int w, int h, char* l = 0 );
    ~Timeline();


    virtual int  handle( int e ) override;
    virtual void draw()          override;

    void main( ViewerUI* m ) {
        uiMain = m;
    }
    ViewerUI* main() const {
        return uiMain;
    }

    int draw_coordinate( double p, int w );
    int slider_position( double p, int w );


    inline Timecode::Display display() const {
        return _display;
    }
    inline void display(Timecode::Display x) {
        _display = x;
        redraw();
    }

    void draw_annotation( const bool t ) {
       m_draw_annotation = t;
    }
    bool draw_annotation() const {
        return m_draw_annotation;
    }

    void draw_cache( const bool t ) {
        m_draw_cache = t;
    }
    bool draw_cache() const {
        return m_draw_cache;
    }

protected:

    static Timecode::Display _display;

    int event_x, event_y;
    bool m_draw_annotation;
    bool m_draw_cache;
    Fl_Window* win;
    ViewerUI* uiMain;
};

} // namespace mrv

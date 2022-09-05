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
 * @file   mrvTimecode.h
 * @author
 * @date   Fri Oct 13 07:51:42 2006
 *
 * @brief  fltk2 value input displaying value as either frame (normal
 *         value input) or as timecode data in hh:mm:ss|ff(+) format.
 *
 *
 */

#pragma once

#include <FL/Fl_Float_Input.H>


class ViewerUI;

namespace mrv
{


class Timecode : public Fl_Float_Input
{
public:
    enum Display
    {
        kFrames,
        kSeconds,
        kTimecode
    };

public:
    Timecode( int x, int y, int w, int h, const char* l = 0 );

    virtual int handle( int e ) override;

    void main( ViewerUI* m ) {
        uiMain = m;
    }
    ViewerUI* main() const {
        return uiMain;
    }


protected:
    ViewerUI*     uiMain;
};

} // namespace mrv

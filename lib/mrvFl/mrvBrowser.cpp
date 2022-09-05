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
 * @file   mrvBrowser.cpp
 * @author gga
 * @date   Wed Jan 31 14:28:24 2007
 *
 * @brief  @TODO: fltk1.4
 *
 *
 */

#include <iostream>

#define assert0(x) if ( !(x) ) do { std::cerr << #x << " FAILED"; abort(); } while(0);

#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Item.H>
#include "mrvBrowser.h"


namespace mrv
{

    // CTOR
    Browser::Browser(int X,int Y,int W,int H,const char*L) :
        Fl_Browser(X,Y,W,H,L)
    {
        _colsepcolor = FL_DARK1;
        _last_cursor = FL_CURSOR_DEFAULT;
        _showcolsep  = 0;
        _drag_col    = -1;
        _nowidths[0] = 0;
        _widths      = _nowidths;
        color( FL_DARK2 );
        textcolor( FL_BLACK );
    }

    void Browser::draw() {
        // DRAW BROWSER}
        Fl_Browser::draw();
        if ( _showcolsep ) {
            // DRAW COLUMN SEPARATORS
            int colx = this->x() - hposition();
            int X,Y,W,H;
            Fl_Browser::bbox(X,Y,W,H);
            fl_color(_colsepcolor);
            for ( int t=0; _widths[t]; t++ ) {
                colx += _widths[t];
                if ( colx > X && colx < (X+W) ) {
                    fl_line(colx, Y, colx, Y+H-1);
                }
            }
        }
    }
} // namespace mrv

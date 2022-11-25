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
 * @file   mrvTable.h
 * @author gga
 * @date   Wed Jan 31 14:26:28 2007
 *
 * @brief  A table that can contain widgets.
 *
 *
 */

#include <iostream>

#include <FL/fl_draw.H>

#include "mrvTable.h"

#define MAX_COLS 2

namespace mrv {

Table::Table( int x, int y, int w, int h, const char* l ) :
Fl_Table( x, y, w, h, l )
{
    rows(0);                   // start with no rows, grow later
    row_header(0);              // enable row headers (along left)
    row_height_all(24);         // default height of rows
    row_resize(0);              // enable row resizing
    // Cols
    cols(MAX_COLS);             // how many columns
    col_header(1);              // enable column headers (along top)
    col_width_all(w/2);         // default width of columns
    col_resize(1);              // enable column resizing
    col_header_height( 28 );
    end();
    // std::cerr << "Table::CONSTRUCTOR " << this << " "
    //           << ( label() ? label() : "none" ) << std::endl;
}


Table::~Table()
{
    // std::cerr << "Table::DESTRUCTOR " << this << " "
    //           << ( label() ? label() : "none" ) << std::endl;
}

int Table::handle( int event )
{
    // std::cerr << "Table::handle " << this << " "
    //           << ( label() ? label() : "none" )
    //           << " hscrollbar " << hscrollbar
    //           << " vscrollbar " << vscrollbar
    //           << std::endl;
    return Fl_Table::handle( event );
}

// Draw the row/col headings
//    Make this a dark thin upbox with the text inside.
//
void Table::DrawHeader(const char *s, int X, int Y, int W, int H) {
    fl_push_clip(X,Y,W,H);
    fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
    fl_color(FL_WHITE);
    fl_draw(s, X,Y,W,H, FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    fl_pop_clip();
}


void Table::add( Fl_Widget* w )
{
    int c = children() % 2;
    if ( c == 0 )
    {
        rows( rows() + 1 );
    }
    int r = rows() - 1;

    int X, Y, W, H;
    find_cell( CONTEXT_TABLE, r, c, X, Y, W, H );
    w->resize( X, Y, W, H );
    Fl_Table::add( w );
    layout();
}

void Table::draw_cell(TableContext context, int ROW, int COL,
                      int X, int Y, int W, int H)
{
    char s[40];
    switch ( context ) {
        case CONTEXT_STARTPAGE:         // before page is drawn..
            fl_font(FL_HELVETICA, 16);  // the font for our drawing operations
            return;
        case CONTEXT_COL_HEADER:                  // Draw column headers
            if ( COL >= 2 ) return;
            sprintf(s,"%s",headers[COL]);                // "A", "B", "C", etc.
            DrawHeader(s,X,Y,W,H);
            return;
        case CONTEXT_CELL:
            return;
        case CONTEXT_RC_RESIZE:
            {
                // Change the table's column_width() for Value column to
                // reach table's edge
                // width of table minus Attribute's current col_width(),
                // which may have been changed interactively by user.
                // Note: use of Fl_Table:tiw for table's "inner width"

                col_width(1, tiw - col_width(0));

                int X, Y, W, H;
                int index = 0;
                int R = rows();
                int C = cols();
                for ( int r = 0; r < R; ++r ) {
                    for ( int c = 0; c < C; ++c ) {
                        if ( index >= children() ) break;
                        find_cell(CONTEXT_TABLE, r, c, X, Y, W, H);
                        child(index++)->resize(X,Y,W,H);
                    }
                }
                init_sizes();                   // tell group children resized
                return;
            }
        default:
            return;
    }
}


// Force table's overall height to match height of all cells + header
//
//                                   top_y1
//        _________________________ /   ^
//    0: |____________|____________|    :
//    1: |____________|____________|    :__ height of table data
//             :           :            :
//        _________________________     :
//    n: |____________|____________|    v
//                                  \bot_y2
//
void Table::layout() {
    int top_y1;
    int bot_y2;
    int R,C,X,Y,W,H;
    // Get Y position of first row
    R=0;        C=0; find_cell(CONTEXT_TABLE, R,C, X,Y,W,H); top_y1 = Y;
    // Get Y position of last row
    R=rows()-1; C=0; find_cell(CONTEXT_TABLE, R,C, X,Y,W,H); bot_y2 = Y+H;
    // Determine overall height of table (diff between top_y1 and bot_y2 plus col_header)
    int dh = bot_y2 - top_y1 + col_header_height() + 4;         // +4: probably border width
    resize(x(),y(),w(),dh);
}


    void Table::resize( int X, int Y, int W, int H )
    {
        Fl_Table::resize( X, Y, W, H );
        col_width_all( W / 2 );
    }

} // namespace mrv

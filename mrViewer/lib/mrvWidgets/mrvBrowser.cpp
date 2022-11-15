
#include <iostream>

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

// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Item.H>

#include "mrvWidgets/mrvBrowser.h"


namespace mrv
{

    // CTOR
    Browser::Browser(int X, int Y, int W, int H, const char* L) :
        Fl_Browser(X, Y, W, H, L)
    {
        _colsepcolor = FL_DARK1;
        _last_cursor = FL_CURSOR_DEFAULT;
        _showcolsep = 0;
        _drag_col = -1;
        _nowidths[0] = 0;
        _widths = _nowidths;
        color(FL_DARK2);
        textcolor(FL_BLACK);
    }

    // MANAGE EVENTS TO HANDLE COLUMN RESIZING
    int Browser::handle(int e)
    {
        // Not showing column separators? Use default Fl_Browser::handle()
        // logic
        if (!showcolsep())
            return (Fl_Browser::handle(e));
        // Handle column resizing
        int ret = 0;
        switch (e)
        {
        case FL_ENTER:
        {
            ret = 1;
            break;
        }
        case FL_MOVE:
        {
            change_cursor(
                (which_col_near_mouse() >= 0) ? FL_CURSOR_WE
                : FL_CURSOR_DEFAULT);
            ret = 1;
            break;
        }
        case FL_PUSH:
        {   
            int whichcol = which_col_near_mouse();
            if (whichcol >= 0)
            {
                // CLICKED ON RESIZER? START DRAGGING
                _drag_col = whichcol;
                change_cursor(FL_CURSOR_DEFAULT);
                return 1; // eclipse event from Fl_Browser's handle()
            }             // (prevents FL_PUSH from selecting item)

            break;
        }
        case FL_DRAG:
        {
            if (_drag_col != -1)
            {
                // Sum up column widths to determine position
                int mousex = Fl::event_x() + hposition();
                int newwidth = mousex - x();
                for (int t = 0; _widths[t] && t < _drag_col; t++)
                {
                    newwidth -= _widths[t];
                }
                if (newwidth > 0)
                {
                    // Apply new width, redraw interface
                    _widths[_drag_col] = newwidth;
                    if (_widths[_drag_col] < 2)
                    {
                        _widths[_drag_col] = 2;
                    }
                    recalc_hscroll();
                    redraw();
                }
                return 1; // eclipse event from Fl_Browser's handle()
            }
            break;
        }
        case FL_LEAVE:
        case FL_RELEASE:
        {
            _drag_col = -1;                   // disable drag mode
            change_cursor(FL_CURSOR_DEFAULT); // ensure normal cursor
            if (e == FL_RELEASE)
                return 1; // eclipse event
            ret = 1;
            break;
        }
        }
        return (Fl_Browser::handle(e) ? 1 : ret);
    }
    
    void Browser::draw()
    {
        // DRAW BROWSER}
        Fl_Browser::draw();
        if (_showcolsep)
        {
            // DRAW COLUMN SEPARATORS
            int colx = this->x() - hposition();
            int X, Y, W, H;
            Fl_Browser::bbox(X, Y, W, H);
            fl_color(_colsepcolor);
            for (int t = 0; _widths[t]; t++)
            {
                colx += _widths[t];
                if (colx > X && colx < (X + W))
                {
                    fl_line(colx, Y, colx, Y + H - 1);
                }
            }
        }
    }
} // namespace mrv

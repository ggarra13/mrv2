// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>

class ViewerUI;

namespace mrv
{

    class Browser : public Fl_Browser
    {
        Fl_Color _colsepcolor;  // color of column separator lines
        int _showcolsep;        // flag to enable drawing column separators
        Fl_Cursor _last_cursor; // saved cursor state info
        int _drag_col;          // col# user is dragging (-1 = not dragging)
        int* _widths;           // pointer to user's width[] array
        int _nowidths[1];       // default width array (non-const)
        // CHANGE CURSOR
        //     Does nothing if cursor already set to value specified.
        //
        void change_cursor(Fl_Cursor newcursor)
        {
            if (newcursor == _last_cursor)
                return;
            window()->cursor(newcursor);
            _last_cursor = newcursor;
        }
        // RETURN THE COLUMN MOUSE IS 'NEAR'
        //     Returns -1 if none.
        //
        int which_col_near_mouse()
        {
            int X, Y, W, H;
            Fl_Browser::bbox(X, Y, W, H); // area inside browser's box()
            // EVENT NOT INSIDE BROWSER AREA? (eg. on a scrollbar)
            if (!Fl::event_inside(X, Y, W, H))
            {
                return (-1);
            }
            int mousex = Fl::event_x() + hposition();
            int colx = this->x();
            for (int t = 0; _widths[t]; t++)
            {
                colx += _widths[t];
                int diff = mousex - colx;
                // MOUSE 'NEAR' A COLUMN?
                //     Return column #
                //
                if (diff >= -4 && diff <= 4)
                {
                    return (t);
                }
            }
            return (-1);
        }
        // FORCE SCROLLBAR RECALC
        //    Prevents scrollbar from getting out of sync during column drags
        void recalc_hscroll()
        {
            int size = textsize();
            textsize(size + 1); // XXX: changing textsize() briefly triggers
            textsize(size);     // XXX: recalc Fl_Browser's scrollbars
            redraw();
        }

    public:
        // CTOR
        Browser(int X, int Y, int W, int H, const char* L = 0);

        void draw() override;
        int handle(int e) override;
        
        // GET/SET COLUMN SEPARATOR LINE COLOR
        Fl_Color colsepcolor() const { return (_colsepcolor); }
        void colsepcolor(Fl_Color val) { _colsepcolor = val; }
        // GET/SET DISPLAY OF COLUMN SEPARATOR LINES
        //     1: show lines, 0: don't show lines
        //
        int showcolsep() const { return (_showcolsep); }
        void showcolsep(int val) { _showcolsep = val; }
        // GET/SET COLUMN WIDTHS ARRAY
        //    Just like fltk method, but array is non-const.
        //
        int* column_widths() const { return (_widths); }

        void column_labels(const char** labels) { add(labels[0]); }

        void column_widths(const int* val)
        {
            _widths = (int*)val;
            Fl_Browser::column_widths(val);
        }
    };

} // namespace mrv

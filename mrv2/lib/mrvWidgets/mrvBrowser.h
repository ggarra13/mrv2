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
        Fl_Color  _colsepcolor;     // color of column separator lines
        int       _showcolsep;      // flag to enable drawing column separators
        Fl_Cursor _last_cursor;     // saved cursor state info
        int       _drag_col;        // col# user is dragging (-1 = not dragging)
        int      *_widths;          // pointer to user's width[] array
        int       _nowidths[1];     // default width array (non-const)
        // CHANGE CURSOR
        //     Does nothing if cursor already set to value specified.
        //
        void change_cursor(Fl_Cursor newcursor)
            {
                if ( newcursor == _last_cursor ) return;
                window()->cursor(newcursor);
                _last_cursor = newcursor;
            }
        // RETURN THE COLUMN MOUSE IS 'NEAR'
        //     Returns -1 if none.
        //
        int which_col_near_mouse()
            {
                int X,Y,W,H;
                Fl_Browser::bbox(X,Y,W,H);            // area inside browser's box()
                // EVENT NOT INSIDE BROWSER AREA? (eg. on a scrollbar)
                if ( ! Fl::event_inside(X,Y,W,H) ) {
                    return(-1);
                }
                int mousex = Fl::event_x() + hposition();
                int colx = this->x();
                for ( int t=0; _widths[t]; t++ ) {
                    colx += _widths[t];
                    int diff = mousex - colx;
                    // MOUSE 'NEAR' A COLUMN?
                    //     Return column #
                    //
                    if ( diff >= -4 && diff <= 4 ) {
                        return(t);
                    }
                }
                return(-1);
            }
        // FORCE SCROLLBAR RECALC
        //    Prevents scrollbar from getting out of sync during column drags
        void recalc_hscroll()
            {
                int size = textsize();
                textsize(size+1);     // XXX: changing textsize() briefly triggers
                textsize(size);       // XXX: recalc Fl_Browser's scrollbars
                redraw();
            }
        
    protected:
        // MANAGE EVENTS TO HANDLE COLUMN RESIZING
        int handle(int e)
            {
                // Not showing column separators? Use default Fl_Browser::handle() logic
                if ( !showcolsep() ) return(Fl_Browser::handle(e));
                // Handle column resizing
                int ret = 0;
                switch ( e ) {
                case FL_ENTER: {
                    ret = 1;
                    break;
                }
                case FL_MOVE: {
                    change_cursor( (which_col_near_mouse() >= 0) ? FL_CURSOR_WE
                                   : FL_CURSOR_DEFAULT);
                    ret = 1;
                    break;
                }
                case FL_PUSH: {
                    int whichcol = which_col_near_mouse();
                    if ( whichcol >= 0 ) {
                        // CLICKED ON RESIZER? START DRAGGING
                        _drag_col = whichcol;
                        change_cursor(FL_CURSOR_DEFAULT);
                        return 1;   // eclipse event from Fl_Browser's handle()
                    }               // (prevents FL_PUSH from selecting item)
                    break;
                }
                case FL_DRAG: {
                    if ( _drag_col != -1 ) {
                        // Sum up column widths to determine position
                        int mousex = Fl::event_x() + hposition();
                        int newwidth = mousex - x();
                        for ( int t=0; _widths[t] && t<_drag_col; t++ ) {
                            newwidth -= _widths[t];
                        }
                        if ( newwidth > 0 ) {
                            // Apply new width, redraw interface
                            _widths[_drag_col] = newwidth;
                            if ( _widths[_drag_col] < 2 ) {
                                _widths[_drag_col] = 2;
                            }
                            recalc_hscroll();
                            redraw();
                        }
                        return 1;   // eclipse event from Fl_Browser's handle()
                    }
                    break;
                }
                case FL_LEAVE:
                case FL_RELEASE: {
                    _drag_col = -1;                         // disable drag mode
                    change_cursor(FL_CURSOR_DEFAULT);       // ensure normal cursor
                    if ( e == FL_RELEASE ) return 1;        // eclipse event
                    ret = 1;
                    break;
                }
                }
                return(Fl_Browser::handle(e) ? 1 : ret);
            }
    public:
        // CTOR
        Browser(int X,int Y,int W,int H,const char*L=0);

        virtual void draw();


        // GET/SET COLUMN SEPARATOR LINE COLOR
        Fl_Color colsepcolor() const
            {
                return(_colsepcolor);
            }
        void colsepcolor(Fl_Color val)
            {
                _colsepcolor = val;
            }
        // GET/SET DISPLAY OF COLUMN SEPARATOR LINES
        //     1: show lines, 0: don't show lines
        //
        int showcolsep() const {
            return(_showcolsep);
        }
        void showcolsep(int val)
            {
                _showcolsep = val;
            }
        // GET/SET COLUMN WIDTHS ARRAY
        //    Just like fltk method, but array is non-const.
        //
        int *column_widths() const
            {
                return(_widths);
            }

        void column_labels( const char** labels )
            {
                add( labels[0] );
            }

        void column_widths(const int *val) {
            _widths = (int*)val;
            Fl_Browser::column_widths(val);
        }
    };

} // namespace mrv

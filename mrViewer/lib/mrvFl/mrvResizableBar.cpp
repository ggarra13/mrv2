//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <iostream>

#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/math.h>

#include "mrvResizableBar.h"


namespace mrv
{

    void ResizableBar::HandleDrag(int diff)
    {
        Fl_Group*  g = static_cast<Fl_Group*>( parent() );
        Fl_Flex* grp = static_cast<Fl_Flex*>( g->parent() );
        int X = g->x() + diff;
        if ( X < min_x ) return;
        int W = g->w() - diff;
        if ( W < min_w && diff > 0 ) return;
        g->resize( X, g->y(), W, g->h() );
        g = static_cast< Fl_Group* >( g->child(1) );  // skip resizebar (0)
        g->resize( X+w(), g->y(), W-w(), g->h() );
        Fl_Scroll* s = static_cast< Fl_Scroll* >( g->child(0) );
        int sw = s->scrollbar.visible() ? s->scrollbar.w() : 0;
        for ( int i = 0; i < s->children(); ++i )
        {
            Fl_Widget* o = s->child(i);
            o->resize( o->x(), o->y(), W-w()-sw, o->h() );
        }
        grp->layout();
    }
    
    ResizableBar::ResizableBar( int X, int Y, int W, int H, const char* L ) :
        Fl_Box( X, Y, W, H, L )
    {
        orig_w = W;
        last_x = 0;
        min_w = 150;
        min_x = 60;
        align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
        visible_focus(0);
        box(FL_DOWN_BOX);
    }

    void ResizableBar::draw() 
    {
        Fl_Box::draw();
        int H  = h() / 2 - 20;
        int H2 = h() / 2 + 20;
        fl_color( FL_BLACK );
        for ( int i = H; i <= H2; i += 4 )
        {
            fl_line( x(), y()+i, x()+w(), y()+i );
        }
    }
    
    int ResizableBar::handle(int e)
    {
        int ret = 0;
        int this_x = Fl::event_x_root();
        switch (e) {
        case FL_UNFOCUS: ret = 1; break;
        case FL_FOCUS: ret = 1; break;
        case FL_ENTER:
            window()->cursor(FL_CURSOR_WE);
            return 1;
            break;
        case FL_LEAVE:
            window()->cursor(FL_CURSOR_DEFAULT);
            return 1;
            break;
        case FL_PUSH:
            ret = 1; last_x = this_x;
            break;
        case FL_DRAG:
            HandleDrag(this_x-last_x);
            last_x = this_x;
            ret = 1;
            break;
        default: break;
        }
        return(Fl_Box::handle(e) | ret);
    }
    
    void ResizableBar::resize(int X,int Y,int W,int H)
    {
        Fl_Box::resize(X,Y,orig_w,H); // width of bar stays constant size
    }

} // namespace mrv

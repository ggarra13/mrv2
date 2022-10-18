//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//
#include <iostream>

#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>



class ResizableBar : public Fl_Box {
    int orig_w;
    int last_x;
    int min_w;                  // min width for widget right of us
    int min_x;                  // max width for widget left of us
    void HandleDrag(int diff)
        {
            Fl_Group*  g = (Fl_Group*) parent();
            Fl_Flex* grp = (Fl_Flex*)g->parent();
            int X = g->x()+diff;
            if ( X < min_x ) return;
            int W = g->w() - diff;
            if ( W < min_w ) return;
            g->resize( X, g->y(), W, g->h() );
            Fl_Scroll* s = (Fl_Scroll*) g->child(1);
            for ( int i = 0; i < s->children(); ++i )
            {
                Fl_Widget* o = s->child(i);
                o->resize( o->x(), o->y(), W-w(), o->h() );
            }
            grp->layout();
        }
public:
    ResizableBar( int X, int Y, int W, int H, const char* L = 0 ) :
        Fl_Box( X, Y, W, H, L )
        {
            orig_w = W;
            last_x = 0;
            min_w = 60;
            min_x = 60;
            visible_focus(0);
            box(FL_DOWN_BOX);
        }
    void draw() override
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
    int handle(int e) override
        {
            int ret = 0;
            int this_x = Fl::event_x_root();
            switch (e) {
            case FL_UNFOCUS: ret = 1; break;
            case FL_FOCUS: ret = 1; break;
            case FL_ENTER:
                ret = 1;
                window()->cursor(FL_CURSOR_WE);
                break;
            case FL_LEAVE: ret = 1; window()->cursor(FL_CURSOR_DEFAULT); break;
            case FL_PUSH:  ret = 1; last_x = this_x;              break;
            case FL_DRAG:
                HandleDrag(this_x-last_x);
                last_x = this_x;
                ret = 1;
                break;
            default: break;
            }
            return(Fl_Box::handle(e) | ret);
        }
    void resize(int X,int Y,int W,int H)
        {
            Fl_Box::resize(X,Y,orig_w,H); // width stays constant size
        }
};

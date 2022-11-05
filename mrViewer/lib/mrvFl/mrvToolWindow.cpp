
#include <iostream>

#include "mrvToolWindow.h"
#include "mrvEventHeader.h"

#include <FL/Fl_Button.H>

namespace mrv
{

// HACK:: This just stores the ToolWindowdows in a static array. I'm too lazy
//        to make a proper linked list to store these in...
    ToolWindow* ToolWindow::active_list[TW_MAX_FLOATERS]; // list of active ToolWindows
    short ToolWindow::active = 0; // count of active tool windows
    constexpr int kMinWidth = 150;
    constexpr int kMinHeight = 150;

// Dummy close button callback
    static void cb_ignore(void)
    {
        // Just shrug off the close callback...
    }

// constructors
    ToolWindow::ToolWindow(int x, int y, int w, int h, const char *l) 
        : Fl_Double_Window(x, y, w, h, l) 
    {
        create_dockable_window();
        box( FL_FLAT_BOX );
    }

    ToolWindow::ToolWindow(int w, int h, const char *l) 
        : Fl_Double_Window(w, h, l) 
    {
        create_dockable_window();
        box( FL_FLAT_BOX );
    }
    

// destructor
    ToolWindow::~ToolWindow()
    {
        active_list[idx] = nullptr;
        active --;
    }

// construction function
    void ToolWindow::create_dockable_window() 
    {
        static int first_window = 1;
        // window list intialisation...
        // this is a nasty hack, should make a proper list
        if(first_window)
        {
            first_window = 0;
            for(short i = 0; i < TW_MAX_FLOATERS; i++)
                active_list[i] = nullptr;
        }
        // find an empty index
        for(short i = 0; i < TW_MAX_FLOATERS; i++)
        {
            if(!active_list[i])
            {
                idx = i;
                active_list[idx] = this;
                active ++;
                clear_border();
                set_non_modal();
                resizable( this );
                callback((Fl_Callback *)cb_ignore);
                return;
            }
        }
        // if we get here, the list is probably full, what a hack.
        // FIX THIS:: At present, we will get a non-modal window with
        // decorations as a default instead...
        set_non_modal();
    }

// show all the active floating windows
    void ToolWindow::show_all(void)
    {
        if (active)
        {
            for(short i = 0; i < TW_MAX_FLOATERS; i++)
            {
                if(active_list[i])
                    active_list[i]->show();
            }
        }
    }

// hide all the active floating windows
    void ToolWindow::hide_all(void)
    {
        if (active)
        {
            for(short i = 0; i < TW_MAX_FLOATERS; i++)
            {
                if(active_list[i])
                    active_list[i]->hide();
            }
        }
    }

    constexpr int kDiff = 5;

    void ToolWindow::set_cursor(int ex, int ey)
    {
        valid = Direction::kNone;
        int rdir = x()+w()-kDiff;
        int ldir = x()+kDiff;
        int bdir = y()+h()-kDiff;
        int tdir = y()+kDiff;
        if ( ex >= rdir ) valid |= Direction::kRight;
        if ( ex <= ldir ) valid |= Direction::kLeft;
        if ( ey >= bdir ) valid |= Direction::kBottom;
        if ( ey <= tdir ) valid |= Direction::kTop;
        if ( valid == Direction::kRight ||
             valid == Direction::kLeft )
            cursor( FL_CURSOR_WE );
        else if ( valid == Direction::kTop ||
                  valid == Direction::kBottom )
            cursor( FL_CURSOR_NS );
        else if ( valid == Direction::kTopRight )
            cursor( FL_CURSOR_NESW );
        else if ( valid == Direction::kTopLeft )
            cursor( FL_CURSOR_NWSE );
        else if ( valid == Direction::kBottomRight )
            cursor( FL_CURSOR_NWSE );
        else if ( valid == Direction::kBottomLeft )
            cursor( FL_CURSOR_NESW );
        else
            cursor( FL_CURSOR_DEFAULT );
    }
        
    int ToolWindow::handle( int event )
    {
        int ret = Fl_Double_Window::handle( event );
        int ex = Fl::event_x_root();
        int ey = Fl::event_y_root();
        switch( event )
        {
        case FL_UNFOCUS:
        case FL_FOCUS:
            return 1;
        case FL_ENTER:
        {
            set_cursor( ex, ey );
            return 1;
        }
        case FL_LEAVE:
            cursor( FL_CURSOR_DEFAULT );
            return 1;
        case FL_MOVE:
        {
            set_cursor( ex, ey );
            return 1;
        }
        case FL_PUSH:
        {
            set_cursor( ex, ey );
            if ( valid != Direction::kNone )
            {
                dir = (Direction)valid;
                last_x = ex; last_y = ey;
                return 1;
            }
            break;
        }
        case FL_DRAG:
        {
            int diffX = ex - last_x;
            int diffY = ey - last_y;
            if ( dir == Direction::kRight )
            {
                if ( w() + diffX > kMinWidth )
                    size( w() + diffX, h() );
            }
            else if ( dir == Direction::kLeft )
            {
                if ( w() - diffX > kMinWidth )
                    resize( x() + diffX, y(), w() - diffX, h() );
            }
            else if ( dir == Direction::kBottom )
            {
                if ( h() + diffY > kMinHeight )
                    size( w(), h() + diffY );
            }
            else if ( dir == Direction::kBottomRight )
            {
                if ( h() + diffY > kMinHeight && w() + diffX > kMinWidth )
                    size( w() + diffX, h() + diffY );
            }
            else if ( dir == Direction::kBottomLeft )
            {
                if ( h() + diffY > kMinHeight && w() - diffX > kMinWidth )
                    resize( x() + diffX, y(), w() - diffX, h() + diffY );
            }
            else if ( dir == Direction::kTop )
            {
                if ( h() - diffY > kMinHeight )
                    resize( x(), y() + diffY, w(), h() - diffY );
            }
            else if ( dir == Direction::kTopRight )
            {
                if ( h() - diffY > kMinHeight && w() + diffX > kMinWidth )
                    resize( x(), y() + diffY, w() + diffX, h() - diffY );
            }
            else if ( dir == Direction::kTopLeft )
            {
                if ( h() - diffY > kMinHeight && w() - diffX > kMinWidth )
                    resize( x() + diffX, y() + diffY,
                            w() - diffX, h() - diffY );
            }
            last_x = ex; last_y = ey;
            return 1;
        }
        }
        return ret;
    }
}

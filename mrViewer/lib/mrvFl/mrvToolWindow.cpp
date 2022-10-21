
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
        tool_group = (void *)0;
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
            Direction valid = Direction::None;
            if ( ex >= x()+w()-kDiff ) valid = Direction::Right;
            if ( ex <= x()+kDiff ) valid = Direction::Left;
            if ( valid == Direction::Right ||
                 valid == Direction::Left )
                cursor( FL_CURSOR_WE );
            return 1;
        }
        case FL_LEAVE:
            cursor( FL_CURSOR_DEFAULT );
            return 1;
        case FL_MOVE:
        {
            Direction valid = Direction::None;
            if ( ex >= x()+w()-kDiff ) valid = Direction::Right;
            if ( ex <= x()+kDiff ) valid = Direction::Left;
            if ( valid == Direction::Right ||
                 valid == Direction::Left )
                cursor( FL_CURSOR_WE );
            else
                cursor( FL_CURSOR_DEFAULT );
            return 1;
        }
        case FL_PUSH:
        {
            Direction valid = Direction::None;
            if ( ex >= x()+w()-kDiff ) valid = Direction::Right;
            if ( ex <= x()+kDiff ) valid = Direction::Left;
            if ( valid != Direction::None )
            {
                dir = valid;
                last_x = ex; last_y = ey;
                return 1;
            }
            break;
        }
        case FL_DRAG:
        {
            int diff = ex - last_x;
            if ( dir == Direction::Right )
            {
                if ( w() + diff > kMinWidth )
                    size( w() + diff, h() );
            }
            else if ( dir == Direction::Left )
            {
                if ( w() - diff > kMinWidth )
                    resize( x()+diff, y(), w() - diff, h() );
            }
            last_x = ex; last_y = ey;
            return 1;
        }
        }
        return ret;
    }
}

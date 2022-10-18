
#include "mrvToolWindow.h"
#include "mrvEventHeader.h"

#include <FL/Fl_Button.H>

namespace mrv
{

// HACK:: This just stores the ToolWindowdows in a static array. I'm too lazy
//        to make a proper linked list to store these in...
    ToolWindow* ToolWindow::active_list[TW_MAX_FLOATERS]; // list of active ToolWindows
    short ToolWindow::active = 0; // count of active tool windows

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
    }

    ToolWindow::ToolWindow(int w, int h, const char *l) 
        : Fl_Double_Window(w, h, l) 
    {
        create_dockable_window();
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

}

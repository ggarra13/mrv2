#pragma once

/* fltk includes */
#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Double_Window.H>

#include "mrvPack.h"

namespace mrv
{

    class ToolWindow : public Fl_Double_Window
    {
#define TW_MAX_FLOATERS	16

    protected:
	void create_dockable_window(void);
	short idx;
	static ToolWindow* active_list[TW_MAX_FLOATERS];
	static short active;

        enum Direction
        {
            None = 0,
            Right,
            Left,
            Top,
            Bottom,
            TopRight,
            TopLeft,
            BottomRight,
            BottomLeft
        };
        
        int last_x, last_y;
        Direction dir, valid;


        void set_cursor(int ex, int ey);
        
    public:
	// Normal FLTK constructors
	ToolWindow(int w, int h, const char *l = 0);
	ToolWindow(int x, int y, int w, int h, const char *l = 0);
	
	// destructor
	~ToolWindow();

        int handle( int event ) override;

	// methods for hiding/showing *all* the floating windows
	static void show_all(void);
	static void hide_all(void);

    };

} // namespace mrv

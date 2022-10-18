#pragma once

/* fltk includes */
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

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
	void *tool_group;

    public:
	// Normal FLTK constructors
	ToolWindow(int w, int h, const char *l = 0);
	ToolWindow(int x, int y, int w, int h, const char *l = 0);
	
	// destructor
	~ToolWindow();

	// methods for hiding/showing *all* the floating windows
	static void show_all(void);
	static void hide_all(void);

	// set the inner group
	void set_inner(void *v) {tool_group = v;}
    };

} // namespace mrv

#pragma once

#include <FL/Fl_Group.H>
//#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>

#include "mrvPack.h"

namespace mrv
{

class DockGroup : public Fl_Group
{
public:
    Fl_Window *win;
    Fl_Scroll* scroll;
    Pack *pack;
    int children;
    int bar_w;

public:
    // Normal FLTK constructors
    DockGroup(int x, int y, int w, int h, const char *l = 0);
	
    // point back to our parent
    void set_window(Fl_Window *w) {win = w;}
    Fl_Window* get_window() const { return win; }

    // methods for adding or removing toolgroups from the dock
    void add(Fl_Widget *w);
    void remove(Fl_Widget *w);

};

}


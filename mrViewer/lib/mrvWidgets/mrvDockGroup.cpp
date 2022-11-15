#include <iostream>

#include <FL/Fl.H>

#include "mrvDockGroup.h"
#include "mrvDropWindow.h"

namespace mrv
{

// basic fltk constructors
DockGroup::DockGroup(int x, int y, int w, int h, const char *l) 
  : Fl_Group(x, y, w, h, l) 
{
    scroll = new Fl_Scroll(x, y, w, h);
    scroll->type( Fl_Scroll::BOTH );
    scroll->begin();
    
    pack = new Pack(x, y, w, h);
    pack->type(Pack::VERTICAL);
    pack->end();
    children = 0;
    scroll->resizable(pack);
    scroll->end();
    resizable( scroll );
    scroll->scrollbar.hide();
    end();
}

void DockGroup::add(Fl_Widget *grp)
{
	int wd = w();
	int ht = h();
	
	// if the dock is "closed", open it back up
	if (!parent()->visible())
	{
            DropWindow *dw = (DropWindow *)win;
            parent()->show();
            dw->workspace->layout();
	}
	pack->add(grp);
	children++;
}

void DockGroup::remove(Fl_Widget *grp)
{
	int wd = w();
	int ht = h();
	pack->remove(grp);
	children--;
        
	// If the dock is empty, close it down
	if (children <= 0)
	{
		DropWindow *dw = (DropWindow *)win;
		children = 0; // just in case...!
                parent()->hide();
                dw->workspace->layout();
	}
}

} // namespace mrv

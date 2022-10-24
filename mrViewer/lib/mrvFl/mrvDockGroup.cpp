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
    scroll->type( Fl_Scroll::VERTICAL );
    scroll->begin();
    
    pack = new Pack(x, y, w, h, "DockGroup pack");
    pack->type(Pack::VERTICAL);
    pack->end();
    children = 0;
    bar_w = parent()->child(0)->w();
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
        
        int pack_sum = pack->h() + grp->h();
        
        int sw = 0;
        if ( children > 1 && pack_sum > scroll->h() )
        {
            sw = scroll->scrollbar_size() ? scroll->scrollbar_size() :
                 Fl::scrollbar_size();
        }
        if ( sw == 0 ) return;

        // Adjust widget sizes to account for scrollbar
        for ( int i = 0; i < scroll->children(); ++i )
        {
            Fl_Widget* o = scroll->child(i);
            o->resize( o->x(), o->y(), scroll->w()-sw, o->h() );
        }
}

void DockGroup::remove(Fl_Widget *grp)
{
	int wd = w();
	int ht = h();
	pack->remove(grp);
	children--;

        int pack_sum = pack->h() - grp->h();
        
        int sw = scroll->scrollbar_size() ? scroll->scrollbar_size() :
                 Fl::scrollbar_size();
        if ( pack_sum <= scroll->h() )
        {
            sw = 0;
        }

        // Adjust widget sizes to account for scrollbar
        for ( int i = 0; i < scroll->children(); ++i )
        {
            Fl_Widget* o = scroll->child(i);
            o->resize( o->x(), o->y(), scroll->w()-sw, o->h() );
        }
        
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

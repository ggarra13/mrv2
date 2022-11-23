
#include <iostream>

/* fltk includes */
#include <FL/Fl.H>

#include "mrvFl/mrvUtil.h"

#include "mrvResizableBar.h"
#include "mrvToolGroup.h"
#include "mrvDropWindow.h"
#include "mrvDockGroup.h"
#include "mrvCollapsibleGroup.h"

#include <FL/Fl_Flex.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>

#include <mrvCore/mrvI8N.h>

// #define DEBUG_COORDS 1
#define LEFT_BUTTONS 1
namespace
{
  const char* kIcon = "@-4circle";
}

namespace mrv
{
    void cb_dock( Fl_Button* o, void* v )
    {
	ToolGroup *gp = (ToolGroup *)v;
        if ( !gp->docked() )
        {
            gp->dock_grp( v );
        }
        else
        {
            gp->undock_grp( v );
        }
    }

    
// function to handle the dock actions
    void ToolGroup::dock_grp(void* v) 
    { // dock CB
	ToolGroup *gp = (ToolGroup *)v;
	DockGroup *dock = gp->get_dock();

	// we can only dock a group that's not already docked 
        // and only if a dock exists for it
        if((!gp->docked()) && (dock))
           {	
               docker->tooltip(_("Undock"));
               //re-dock the group
               ToolWindow *cur_parent = (ToolWindow *)gp->parent();
               // Make sure we turn off the toolgroup scroller, as we are going
               // to handle it with the dockgroup scroller
               gp->scroll->size( gp->pack->w(), gp->pack->h() );
               dock->add(gp); // move the toolgroup into the dock
        
               gp->docked(-1);    // toolgroup is docked...
               // so we no longer need the tool window.
               cur_parent->hide();
               delete cur_parent;

               dock->redraw();
           }
    }

// static CB to handle the undock actions
    void ToolGroup::undock_grp(void* v) 
    { // undock CB
	ToolGroup *gp = (ToolGroup *)v;
	DockGroup *dock = gp->get_dock();
	
	if(gp->docked())
	{	// undock the group into its own non-modal tool window
            docker->tooltip(_("Dock"));
            int sw = 0;
            if ( gp->pack->h() > gp->scroll->h() )
            {
                sw = Fl::scrollbar_size();
            }
            int W = gp->w() + sw;
            int H = gp->h();
            Fl_Group::current(0);
            tw = new ToolWindow(Fl::event_x_root() - 10,
                                Fl::event_y_root() - 35, W + 3, H + 3);
            tw->end();
            gp->scroll->type( Fl_Scroll::BOTH );
            gp->end();  // needed to adjust pack and scroll
            dock->remove(gp);
            tw->add(gp);// move the tool group into the floating window
            gp->position(1, 1); // align group in floating window
            tw->resizable(gp);
            tw->show(); // show floating window
            gp->docked(0);      // toolgroup is no longer docked
            dock->redraw();     // update the dock, to show the group has gone...
	}
    }

// static CB to handle the dismiss action
    void ToolGroup::cb_dismiss(Fl_Button*, void* v) 
    {
	ToolGroup *gp = (ToolGroup *)v;
	DockGroup *dock = gp->get_dock();
	
	if(gp->docked())
	{	// remove the group from the dock
            dock->remove(gp);
            gp->docked(0);
            dock->redraw();     // update the dock, to show the group has gone...
            Fl::delete_widget(gp);
	}
	else
	{   // remove the group from the floating window, 
	    // and remove the floating window
	    ToolWindow* cur_parent = gp->get_window();
            cur_parent->remove(gp);
            //delete cur_parent; // we no longer need the tool window.
            Fl::delete_widget(cur_parent);
            Fl::delete_widget(gp);
	}
    }

    void ToolGroup::resize( int X, int Y, int W, int H )
    {
        Fl_Group::resize( X, Y, W, H );
        
        pack->size(W - 3, pack->h());
        pack->layout();
        
        if ( !tw )
        {
            int H = pack->h();
            if ( H > dock->h() ) H = dock->h();
            scroll->size( W, H );
        }
    }
    
    void ToolGroup::end()
    {
        pack->end();
        Fl_Group::end();
        int sw = Fl::scrollbar_size();                // scrollbar width
        int W = pack->w();
        int H = pack->h() + 23;
	int Hx = 23;
	for ( unsigned i = 0; i < pack->children(); ++i )
	  {
	    Fl_Widget* w = pack->child(i);
	    CollapsibleGroup* c = dynamic_cast< CollapsibleGroup* >( w );
	    if ( !c )
	      {
                  Hx += w->visible() ? w->h() : 0;
                  continue;
	      }
	    int groupH = c->visible() ? c->h() : 0;
	    Hx += groupH;
	    if ( c->is_open() )
	      {
		continue;
	      }
	    else
	      {
		Pack* contents = c->contents();
		Hx += contents->h() + contents->spacing();
	      }
	  }
	if ( Hx > H ) H = Hx;
        Fl_Group::resizable(0);
        Fl_Group::size( W, H );
        init_sizes();
        Fl_Group::resizable(scroll);

            
        if ( tw )
        {
            int screen = Fl::screen_num( tw->x(), tw->y(),
                                         tw->w(), tw->h() );
            int minx, miny, maxW, maxH;
            Fl::screen_work_area( minx, miny, maxW, maxH, screen );

            int maxHeight  = maxH - 48;  // leave some headroom for topbar
            if ( H > maxHeight )  H = maxHeight;
        
            scroll->size( pack->w(), H-23 );
            init_sizes();  // needed to reset scroll size init size
            
            tw->resizable(0);
            tw->size( tw->w(), H+3 );
            tw->resizable( this );
        }
        else
        {
            if ( H > dock->h() ) H = dock->h();
            scroll->size( W, H );
        }
    }

// Constructors for docked/floating window
// WITH x, y co-ordinates
    ToolGroup::ToolGroup(DockGroup *dk, int floater, int x, int y, int w,
                         int h, const char *lbl)
        : Fl_Group(1, 1, w, h), tw( nullptr )
    {
	if((floater) && (dk)) // create floating
	{
            create_floating(dk, 1, x, y, w, h, lbl);
	}
	else if(dk) // create docked
	{
            create_docked(dk, lbl);
	}
//	else //do nothing...
    }

// WITHOUT x, y co-ordinates
    ToolGroup::ToolGroup(DockGroup *dk, int floater, int w, int h, const char *lbl)
        : Fl_Group(1, 1, w, h), tw( nullptr )
    {
	if((floater) && (dk)) // create floating
	{
            create_floating(dk, 0, 0, 0, w, h, lbl);
	}
	else if(dk) // create docked
	{
            create_docked(dk, lbl);
	}
//	else //do nothing...
    }

// construction function
    void ToolGroup::create_dockable_group(const char* lbl)
    {
        int width = w();
	int X = 3 * width / 270;
	int W = 20 * width / 270;

        
#ifdef LEFT_BUTTONS
        dismiss = new Fl_Button( X, 3, W, 20, kIcon);
        X += W;
        docker = new Fl_Button( X, 3, W, 20, kIcon);
        X += W;
	W = w() - dismiss->w() - docker->w() - 3;
	dragger = new DragButton( X, 3, W, 20, lbl);
#else
        dismiss = new Fl_Button(w()-W+3, 3, W, 20, kIcon);
        docker = new Fl_Button(w()-W*2+3, 3, W, 20, kIcon);
	dragger = new DragButton(3, 3, w()-W*2-6, 20, lbl);
#endif
        dismiss->labelcolor( FL_RED );
        docker->labelcolor( FL_YELLOW );
        
	dismiss->box(FL_NO_BOX);
	dismiss->tooltip(_("Dismiss"));
	dismiss->clear_visible_focus();
	dismiss->callback((Fl_Callback*)cb_dismiss, (void *)this);
	
	docker->box(FL_NO_BOX);
	docker->tooltip(_("Undock"));
	docker->callback((Fl_Callback*)cb_dock, (void *)this);
        
	dragger->type(FL_TOGGLE_BUTTON);
	dragger->box(FL_ENGRAVED_BOX);
	dragger->tooltip(_("Drag Box"));
	dragger->clear_visible_focus();
        dragger->align( FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
                        FL_ALIGN_IMAGE_NEXT_TO_TEXT );
	dragger->when(FL_WHEN_CHANGED);


        group = new Fl_Group( x(), 23, w(), 30);
        group->hide();
        group->end();

        scroll      = new Fl_Scroll( 3, 23, w()-3, h()-23 );
        scroll->type( Fl_Scroll::BOTH );
        scroll->begin();
        
	pack = new Pack(3, 23, w()-3, 1);
        pack->end();
        
        scroll->end();
        
        Fl_Group::resizable(scroll);
    }

    void ToolGroup::create_docked(DockGroup *dk, const char* lbl)
    {
	// create the group itself
	create_dockable_group(lbl);
        docker->tooltip( "Undock" );
	// place it in the dock
	dk->add(this);
	set_dock(dk); // define where the toolgroup is allowed to dock
	docked(1);	// docked
    }

    void ToolGroup::create_floating(DockGroup *dk, int full, int x, int y, int w, int h, const char *lbl)
    {
	// create the group itself
	create_dockable_group(lbl);
	// create a floating toolbar window
	// Ensure the window is not created as a child of its own inner group!
	Fl_Group::current(0);
	if(full)
            tw = new ToolWindow(x, y, w + 3, h + 3, lbl);
	else
            tw = new ToolWindow(w + 3, h + 3, lbl);
	tw->end();
	tw->add(this);  // move the tool group into the floating window
        tw->resizable(this);
	docked(0);		// NOT docked
	set_dock(dk);	// define where the toolgroup is allowed to dock
	tw->show();
	Fl_Group::current(pack); // leave this group open when we leave the constructor...
    }

// function for setting the docked state and checkbox
    void ToolGroup::docked(short r)
    { 
	_docked = r; 
    }

// methods for hiding/showing *all* the floating windows
// show all the active floating windows
    void ToolGroup::show_all(void)
    {
	ToolWindow::show_all();
    }

//! hide all the active floating windows
    void ToolGroup::hide_all(void)
    {
	ToolWindow::hide_all();
    }

} // namespace mrv


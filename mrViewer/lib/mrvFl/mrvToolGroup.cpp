
#include <iostream>

/* fltk includes */
#include <FL/Fl.H>

#include "mrvResizableBar.h"
#include "mrvToolGroup.h"
#include "mrvDropWindow.h"
#include "mrvDockGroup.h"

namespace mrv
{
    void cb_dock( Fl_Button* o, void* v )
    {
	ToolGroup *gp = (ToolGroup *)v;
        if ( !gp->docked() )
        {
            o->tooltip( "Undock" );
            gp->dock_grp( v );
        }
        else
        {
            o->tooltip( "Dock" );
            gp->undock_grp( v );
        }
    }

    void ToolGroup::refresh_dock( DockGroup* dk )
    {
        Fl_Group* g   = dk->parent();
        Fl_Flex* flex = static_cast< Fl_Flex* >( g->parent() );
        g->show();
        flex->layout();
        ResizableBar* bar = (ResizableBar*) g->child(0);
        bar->HandleDrag(0);
        dk->redraw();
    }
    
// function to handle the dock actions
    void ToolGroup::dock_grp(void* v) 
    { // dock CB
	ToolGroup *gp = (ToolGroup *)v;
	DockGroup *dock = gp->get_dock();

	// we can only dock a group that's not already docked 
        // and only if a dock exists for it
        if((!gp->docked()) && (dock))
           {	//re-dock the group
               ToolWindow *cur_parent = (ToolWindow *)gp->parent();
               dock->add(gp); // move the toolgroup into the dock
        
               gp->docked(-1);    // toolgroup is docked...
               // so we no longer need the tool window.
               cur_parent->hide();
               delete cur_parent;

               dock->redraw();
               //refresh_dock(dock);
           }
    }

// static CB to handle the undock actions
    void ToolGroup::undock_grp(void* v) 
    { // undock CB
	ToolGroup *gp = (ToolGroup *)v;
	DockGroup *dock = gp->get_dock();
	
	if(gp->docked())
	{	// undock the group into its own non-modal tool window
            int w = gp->w();
            int h = gp->h();
            Fl_Group::current(0);
            ToolWindow *new_parent = new ToolWindow(Fl::event_x_root() - 10, Fl::event_y_root() - 35, w + 3, h + 3);
            new_parent->end();
            dock->remove(gp);
            new_parent->add(gp);// move the tool group into the floating window
            new_parent->set_inner((void *)gp);
            gp->position(1, 1); // align group in floating window
            new_parent->resizable(gp);
            new_parent->show(); // show floating window
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
	    ToolWindow *cur_parent = (ToolWindow *)gp->parent();
            cur_parent->remove(gp);
            //delete cur_parent; // we no longer need the tool window.
            Fl::delete_widget(cur_parent);
            Fl::delete_widget(gp);
	}
    }

    void ToolGroup::end()
    {
        inner_group->end();
        inner_group->layout();
        Fl_Group::end();
        Fl_Group::size( w() + 3, inner_group->h()+23 );
        if ( tw ) {
            tw->resizable(0);
            tw->size( inner_group->w()+3, inner_group->h()+23 );
            tw->resizable( inner_group );
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
        
#if __APPLE__
        dismiss = new Fl_Button(3, 3, 11, 20, "@-4circle");
        docker = new Fl_Button(19, 3, 11, 20, "@-4circle");
	dragger = new DragButton(33, 3, w()-33, 20);
#else
        dismiss = new Fl_Button(w()-11, 3, 11, 20, "@-4circle");
        docker = new Fl_Button(w()-26, 3, 11, 20, "@-4circle");
	dragger = new DragButton(3, 3, w()-33, 20);
#endif
	dismiss->box(FL_NO_BOX);
	dismiss->tooltip("Dismiss");
        dismiss->labelcolor( FL_RED );
	dismiss->clear_visible_focus();
	dismiss->callback((Fl_Callback*)cb_dismiss, (void *)this);
	
	docker->box(FL_NO_BOX);
        docker->labelcolor( FL_YELLOW );
	docker->tooltip("Dock");
	docker->clear_visible_focus();
	docker->callback((Fl_Callback*)cb_dock, (void *)this);
        
	dragger->type(FL_TOGGLE_BUTTON);
	dragger->box(FL_ENGRAVED_BOX);
	dragger->tooltip("Drag Box");
	dragger->clear_visible_focus();
	dragger->when(FL_WHEN_CHANGED);
        
	inner_group = new Pack(3, 21, w()-3, 10, lbl);
        Fl_Group::resizable(0);  // we'll handle the resizing manually
    }

    void ToolGroup::create_docked(DockGroup *dk, const char* lbl)
    {
	// create the group itself
        refresh_dock(dk);
        
	create_dockable_group(lbl);
        docker->tooltip( "Undock" );
	// place it in the dock
	dk->add(this);
	set_dock(dk); // define where the toolgroup is allowed to dock
	docked(1);	// docked
	dk->redraw();
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
	tw->set_inner((void *)this);
	tw->show();
	Fl_Group::current(inner_group); // leave this group open when we leave the constructor...
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


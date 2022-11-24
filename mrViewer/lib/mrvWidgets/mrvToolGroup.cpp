#include <iostream>

/* fltk includes */
#include <FL/Fl.H>

#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvUtil.h"

#include "mrvResizableBar.h"
#include "mrvToolGroup.h"
#include "mrvDropWindow.h"
#include "mrvDockGroup.h"
#include "mrvCollapsibleGroup.h"

// On macOS, the buttons go to the left of the window
#ifdef __APPLE__
   #define LEFT_BUTTONS 1
#endif

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
               //re-dock the group
               ToolWindow *cur_parent = (ToolWindow *)gp->parent();
               // Make sure we turn off the toolgroup scroller, as we are going
               // to handle it with the dockgroup scroller
               gp->end();
               Pack* pack = gp->get_pack();
               Fl_Scroll* scroll = gp->get_scroll();
               int W = pack->w();
               if ( pack->h() > dock->h() )
               {
                   W -= scroll->scrollbar.w();
               }
               pack->size( W, pack->h() );
               scroll->size( W, pack->h() );
               scroll->scroll_to( 0, 0 );
               gp->debug("dock_grp");
               dock->add(gp); // move the toolgroup into the dock

               gp->docked(true);    // toolgroup is docked...
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
            int w = gp->w(); 
            int h = gp->h();
            Fl_Group::current(0);
            tw = new ToolWindow(Fl::event_x_root() - 10, Fl::event_y_root() - 35, w + 3, h + 3);
            tw->end();
            gp->docked(false);      // toolgroup is no longer docked
            gp->end();  // needed to adjust pack and scroll
            gp->debug("undock_grp");
            dock->remove(gp);
            tw->add(gp);// move the tool group into the floating window
            gp->position(1, 1); // align group in floating window
            tw->resizable(gp);
            tw->show(); // show floating window
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
            gp->docked(false);
            dock->redraw();     // update the dock, to show the group has gone...
            Fl::delete_widget(gp);
        }
        else
        {   // remove the group from the floating window,
            // and remove the floating window
            ToolWindow* cur_parent = gp->get_window();
            cur_parent->remove(gp);
            // we no longer need the tool window.
            Fl::delete_widget(cur_parent);
            Fl::delete_widget(gp);
        }
    }
    
    void ToolGroup::docked(bool r)
    {
        _docked = r;
        if ( r ) docker->tooltip( _("Undock") );
        else     docker->tooltip( _("Dock") );
    }

    void ToolGroup::resize( int X, int Y, int W, int H )
    {
        Fl_Group::resize( X, Y, W, H );

        pack->size(W - 3, pack->h());

        if ( docked() )
        {
            scroll->size( pack->w(), pack->h() );
        }
        
#ifdef LEFT_BUTTONS
        X = x() + 40 + 3;
        W = w() - 40 - 3;
        dragger->resize( X, dragger->y(), W, dragger->h() );
#else
        X = x();
        W = w() - 40 - 3;
        dragger->resize( X, dragger->y(), W, dragger->h() );
        W =20;
        X = x() + w() - W*2 - 3;
        docker->resize( X, docker->y(), 20, 20 );
        X = x() + w() - W - 3;
        dismiss->resize( X, dismiss->y(), 20, 20 );
#endif
            
        debug("RESiZE" );
    }

    void ToolGroup::end()
    {
        pack->end();
        Fl_Group::end();
        layout();
    }

    void ToolGroup::debug( const char* lbl ) const
    {
#ifdef DEBUG_COORDS
        std::cerr << "=============" << lbl << "============" << std::endl
                  << "       Y=" << y() << std::endl
                  << "  pack Y=" << pack->y() << std::endl
                  << "scroll Y=" << scroll->y() << std::endl
                  << "bar    Y=" << scroll->scrollbar.y() << std::endl;
        if ( tw )
            std::cerr << "tw     H=" << tw->y() << std::endl;
        std::cerr << "--------------------------------------" << std::endl
                  << "       H=" << h() << std::endl
                  << "  pack H=" << pack->h() << std::endl
                  << "scroll H=" << scroll->h() << std::endl
                  << "bar    H=" << scroll->scrollbar.h() << std::endl;
        if ( tw )
            std::cerr << "tw     H=" << tw->h() << std::endl;
        std::cerr << "======================================"
                  << std::endl;
#endif
    }
    
    void ToolGroup::layout()
    {
        pack->layout();
        Fl_Group* g = group;
        int GH = g && g->visible() ? g->h() : 0;
        int GY = g && g->visible() ? g->y() : 0;
        int W = w() - 3;
        int H = GH + dragger->h() + pack->h();

        Fl_Group::resizable(0);
        Fl_Group::size( W, H );
        Fl_Group::resizable(scroll);
        Fl_Group::init_sizes();

        if (!docked())
        {
            int screen = Fl::screen_num( tw->x(), tw->y(),
                                         tw->w(), tw->h() );
            int minx, miny, maxW, maxH;
            Fl::screen_work_area( minx, miny, maxW, maxH, screen );

            // leave some headroom for topbar
            maxH = maxH - docker->h() - GH - 23; // 23 of offset

            if ( H > maxH ) {
                std::cerr << "H > maxH " << H << " > " << maxH << std::endl;
                H = maxH;
            }
            scroll->size( pack->w(), H );
            scroll->scrollbar.size( scroll->scrollbar.w(), H );
            scroll->init_sizes();  // needed? to reset scroll size init size

            tw->resizable(0);
            tw->size( tw->w(), H+3 );
            tw->resizable( this );

            debug("END WINDOW");

        }
        else
        {
            debug("END DOCKED");
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


// construction function
    void ToolGroup::create_dockable_group(const char* lbl)
    {
        // Create a group to enclose the buttons and make it
        // not resizable.
        int W = 20;
        
#ifdef LEFT_BUTTONS
        int X = 3;
        Fl_Group* g = new Fl_Group( X, 3, W*2, 20 );
        dismiss = new Fl_Button( X, 3, W, 20, kIcon);
        X += W;
        docker = new Fl_Button( X, 3, W, 20, kIcon);

        g->end();

        
        X += W;
        W = w() - W * 2 - 3;
        
        dragger = new DragButton( X, 3, W, 20, lbl);
#else
        int X = x() + w() - W*2 - 3;
        Fl_Group* g = new Fl_Group( X, 3, W*2, 20 );
        docker = new Fl_Button(X, 3, W, 20, kIcon);
        X = x() + w() - W - 3;
        dismiss = new Fl_Button(X, 3, W, 20, kIcon);
        g->end();
        
        dragger = new DragButton(3, 3, w()-W*2-3, 20, lbl);
#endif
        dismiss->labelcolor( FL_RED );
        docker->labelcolor( FL_YELLOW );

        dismiss->box(FL_NO_BOX);
        dismiss->tooltip("Dismiss");
        dismiss->clear_visible_focus();
        dismiss->callback((Fl_Callback*)cb_dismiss, (void *)this);

        docker->box(FL_NO_BOX);
        docker->tooltip("Dock");
        docker->clear_visible_focus();
        docker->callback((Fl_Callback*)cb_dock, (void *)this);
        

        dragger->type(FL_TOGGLE_BUTTON);
        dragger->box(FL_ENGRAVED_BOX);
        dragger->tooltip("Drag Box");
        dragger->clear_visible_focus();
        dragger->align( FL_ALIGN_CENTER | FL_ALIGN_INSIDE |
                        FL_ALIGN_IMAGE_NEXT_TO_TEXT );
        dragger->when(FL_WHEN_CHANGED);

        g->resizable(0);

        group = new Fl_Group( x(), 35, w(), 30);
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

        set_dock(dk); // define where the toolgroup is allowed to dock
        // Create the group itself
        create_dockable_group(lbl);
        // place it in the dock
        dk->add(this);
        docked(true);	// docked
        debug("CREATE DOCKED");
    }

    void ToolGroup::create_floating(DockGroup *dk, int full, int x, int y, int w, int h, const char *lbl)
    {
        // create the group itself
        create_dockable_group(lbl);
        // create a floating toolbar window
        // Ensure the window is not created as a child of its own inner group!
        Fl_Group::current(0);
        tw = new ToolWindow(x, y, w + 3, h + 3, lbl);
        tw->end();
        scroll->size( w - 3, h - 3 );
        set_dock(dk);	// define where the toolgroup is allowed to dock
        docked(false);		// NOT docked
        tw->add(this);  // move the tool group into the floating window
        tw->resizable(this);
        tw->show();
        debug("CREATE FLOATING");
        Fl_Group::current(pack); // leave this group open when we leave the constructor...
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

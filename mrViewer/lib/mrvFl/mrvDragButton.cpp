#include <iostream>

#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>

#include "mrvDragButton.h"
#include "mrvEventHeader.h"
#include "mrvToolGroup.h"

#include "dock_grip_tile.xpm"

namespace mrv
{

    DragButton::DragButton(int x, int y, int w, int h, const char *l)
        : Fl_Box(x, y, w, h, l) 
    {
	was_docked = 0; // Assume we have NOT just undocked...
    }

    void DragButton::draw()
    {
	int xo = x();
	int yo = y();

	// Draw the button box
	draw_box(box(), color());

	// set the clip region so we only "tile" the box
	fl_push_clip(xo+1, yo+1, w()-3, h()-3);

	// tile the pixmap onto the button face... there must be a better way
	for(int i = 2; i <= w(); i += 6)
            for(int j = 2; j <= h(); j += 6)
                fl_draw_pixmap(grip_tile_xpm, (xo + i), (yo + j));

	fl_pop_clip();
    } // draw

    int DragButton::handle(int event) 
    {
 	ToolGroup *tg = (ToolGroup *)parent();
	int docked = tg->docked();
	int ret = 0;
	int x2 = 0, y2 = 0;
	int cx, cy;
        if ( event == FL_ENTER )
        {
            window()->cursor( FL_CURSOR_MOVE );
            ret = 1;
        }
        else if ( event == FL_LEAVE )
        {
            window()->cursor( FL_CURSOR_DEFAULT );
            ret = 1;
        }
	// If we are not docked, deal with dragging the toolwin around
	if (!docked)
	{
            // get the enclosing parent widget
            Fl_Widget *tw = (Fl_Widget *)(tg->parent());
            if(!tw) return 0;
	
            switch (event) 
            {
            case FL_PUSH: // downclick in button creates cursor offsets
                x1 = Fl::event_x_root();
                y1 = Fl::event_y_root();
                xoff = tw->x() - x1;
                yoff = tw->y() - y1;
                ret = 1;
                break;
		
            case FL_DRAG: // drag the button (and its parent window) around the screen
                if(was_docked)
                {	// Need to init offsets, we probably got here following a drag 
                        // from the dock, so the PUSH (above) will not have happened.
                        was_docked = 0;
                    x1 = Fl::event_x_root();
                    y1 = Fl::event_y_root();
                    xoff = tw->x() - x1;
                    yoff = tw->y() - y1;
                }
                tw->position(xoff + Fl::event_x_root(), yoff + Fl::event_y_root());
                tw->redraw();
                ret = 1;
                break;
		
            case FL_RELEASE:
                cx = Fl::event_x_root(); // Where did the release occur...
                cy = Fl::event_y_root();
                x2 = x1 - cx;
                y2 = y1 - cy;
                x2 = (x2 > 0) ? x2 : (-x2);
                y2 = (y2 > 0) ? y2 : (-y2);
                if ((x2 > 10) || (y2 > 10))
                {	// test for a docking event
                    // See if anyone is able to accept a dock with this widget
                    // How to find the dock window? Search 'em all for now...
                    for(Fl_Window *win = Fl::first_window(); win; win = Fl::next_window(win))
                    {
                        // Get the co-ordinates of each window
                        int ex = win->x_root();
                        int ey = win->y_root();
                        int ew = win->w();
                        int eh = win->h();
					
                        // Are we inside the boundary of the window?
                        if(win->visible() && (cx > ex) && (cy > ey)
                           && (cx < (ew + ex)) && (cy < (eh + ey)))
                        {	// Send the found window a message that we want to dock with it.
                            if(Fl::handle(FX_DROP_EVENT, win))
                            {
                                tg->dock_grp(tg);
                                break;
                            }
                        }
                    }
                }
                //show();
                ret = 1;
                break;
			
            default:
                break;
            }
            return(ret);
	}
	
	// OK, so we must be docked - are we being dragged out of the dock?
	switch(event) 
	{
	case FL_PUSH: // downclick in button creates cursor offsets
            x1 = Fl::event_x_root();
            y1 = Fl::event_y_root();
            ret = 1;
            break;

	case FL_DRAG:
            // IF the drag has moved further than the drag_min distance
            // THEN invoke an un-docking
            x2 = Fl::event_x_root() - x1;
            y2 = Fl::event_y_root() - y1;
            x2 = (x2 > 0) ? x2 : (-x2);
            y2 = (y2 > 0) ? y2 : (-y2);
            if ((x2 > 10) || (y2 > 10))
            {
                tg->undock_grp((void *)tg); // undock the window
                was_docked = -1; // note that we *just now* undocked
            }
            ret = 1;
            break;

	default:
            break;
	}
	return ret;
    } // handle

} // namespace mrv


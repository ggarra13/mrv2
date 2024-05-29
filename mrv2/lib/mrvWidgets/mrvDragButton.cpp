// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cassert>

#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/names.h>

#include "mrvWidgets/mrvDropWindow.h"
#include "mrvWidgets/mrvDragButton.h"
#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvPanelGroup.h"

namespace
{
    const int kDragMinDistance = 10;
}

namespace mrv
{

    DragButton::DragButton(int x, int y, int w, int h, const char* l) :
        Fl_Box(x, y, w, h, l)
    {
        was_docked = 0; // Assume we have NOT just undocked...
    }

    int DragButton::handle(int event)
    {
        int ret = Fl_Box::handle(event);
        
#ifndef NDEBUG
        if (event != FL_SHORTCUT && event != FL_NO_EVENT &&
            event != FL_MOVE)
            std::cerr << fl_eventnames[event] << std::endl;
#endif
        
        PanelGroup* tg = (PanelGroup*)parent();
        int docked = tg->docked();
        int x2 = 0, y2 = 0;
        int cx, cy;
        if (event == FL_ENTER || event == FL_MOVE)
        {
            if (window())
                window()->cursor(FL_CURSOR_MOVE);
            ret = 1;
        }
        else if (event == FL_LEAVE)
        {
            if (window())
                window()->cursor(FL_CURSOR_DEFAULT);
            ret = 1;
        }
        // If we are not docked, deal with dragging the toolwin around
        if (!docked)
        {
            // get the enclosing parent widget
            PanelWindow* tw = tg->get_window();
            assert(tw);

            switch (event)
            {
            case FL_PUSH: // downclick in button creates cursor offsets
                x1 = Fl::event_x_root();
                y1 = Fl::event_y_root();
                xoff = tw->x_root() - x1;
                yoff = tw->y_root() - y1;
#ifndef NDEBUG
                std::cerr << "\tx1, y1=" << x1 << " " << y1 << std::endl;
                std::cerr << "\ttw    =" << tw->x_root() << " " << tw->y_root()
                          << std::endl;
                std::cerr << "\toffsets=" << xoff << " " << yoff << std::endl;
#endif
                ret = 1;
                break;

            case FL_DRAG: // drag the button (and its parent window) around the
            {          // screen
                if (was_docked)
                {
                    // Need to init offsets, we probably got here following a
                    // drag from the dock, so the PUSH (above) will not have
                    // happened.
                    was_docked = 0;
                    x1 = Fl::event_x_root();
                    y1 = Fl::event_y_root();
                    xoff = tw->x_root() - x1;
                    yoff = tw->y_root() - y1;
                }
                else
                {
                    int dock_attempt = would_dock();
                    if (dock_attempt)
                    {
                        color_dock_group(FL_DARK_YELLOW);
                        show_dock_group();
                    }
                    else
                    {
                        hide_dock_group();
                    }
                }
                Fl_Window* top = top_window();
                
                int posX = Fl::event_x_root();
                int posY = Fl::event_y_root();
                
#ifdef __linux__
#    ifdef FLTK_USE_WAYLAND
                if (fl_wl_display())
                {
                    posX -= top->x_root();
                    posY -= top->y_root();
                }
                else
                {
#    ifdef PARENT_TO_TOP_WINDOW
                    posX -= top->x_root();
                    posY -= top->y_root();
#    endif
                }
#    endif
#endif
                posX += xoff;
                posY += yoff;
#ifndef NDEBUG
                std::cerr << "\tposition window with code at line="
                          << __LINE__
                          << std::endl
                          << "\t\torig=" << tw->x() << " " << tw->y()
                          << std::endl
                          << "\t\t pos=" << posX << " " << posY
                          << std::endl;
#endif
                tw->position(posX, posY);
                ret = 1;
                break;
            }
            case FL_RELEASE:
            {
                int dock_attempt = would_dock();
                
                if (dock_attempt)
                {
                    color_dock_group(FL_BACKGROUND_COLOR);
                    show_dock_group();
                    tg->dock_grp(tg);
                }
                ret = 1;
            }
            break;

            default:
                break;
            }
            return (ret);
        }

        if (tg->children() == 0)
            return ret;

        // OK, so we must be docked - are we being dragged out of the dock?
        switch (event)
        {
        case FL_PUSH: // downclick in button creates cursor offsets
            x1 = Fl::event_x_root();
            y1 = Fl::event_y_root();
            ret = 1;
            break;

        case FL_DRAG:
            // If the drag has moved further than the drag_min distance
            // then invoke an un-docking
            x2 = Fl::event_x_root() - x1;
            y2 = Fl::event_y_root() - y1;
            x2 = (x2 > 0) ? x2 : (-x2);
            y2 = (y2 > 0) ? y2 : (-y2);
            if ((x2 > kDragMinDistance) || (y2 > kDragMinDistance))
            {
                Fl_Window* top = top_window();
                assert(top);
                
                tg->undock_grp((void*)tg); // undock the window
                was_docked = -1;           // note that we *just now* undocked
                
                PanelWindow* tw = tg->get_window();
                assert(tw);
#ifndef NDEBUG
                std::cerr << "position window with code at line="
                          << __LINE__ << std::endl;
#endif
                int posX = Fl::event_x_root();
                int posY = Fl::event_y_root();
                
#ifdef __linux__
#    ifdef FLTK_USE_WAYLAND
                if (fl_wl_display())
                {
                    posX -= top->x_root();
                    posY -= top->y_root();
                }
                else
                {
#        ifdef PARENT_TO_TOP_WINDOW
                    posX -= top->x_root();
                    posY -= top->y_root();
#        endif
                }
#    endif
#endif
                tw->position(posX, posY);
            }
            ret = 1;
            break;

        default:
            break;
        }
        return ret;
    } // handle


    void DragButton::color_dock_group(Fl_Color c)
    {
        PanelGroup* tg = static_cast<PanelGroup*>(parent());
        DockGroup* uiDock = tg->get_dock();
        Fl_Widget* widget = uiDock->get_scroll();
        widget->color(c);
        widget->redraw();
    }
    
    void DragButton::show_dock_group()
    {
        PanelGroup* tg = static_cast<PanelGroup*>(parent());
        DockGroup* uiDock = tg->get_dock();
        const Pack* uiDockPack = uiDock->get_pack();
        auto uiDockGroup = uiDock->parent();
        
        // Show the dock group if it is hidden
        if (!uiDockGroup->visible())
        {
            uiDockGroup->show();
                            
            auto dropWindow = static_cast<DropWindow*>(uiDock->top_window());
            Fl_Flex* flex = dropWindow->workspace;
            flex->layout();
            flex->redraw();
        }
    }

    void DragButton::hide_dock_group()
    {
        PanelGroup* tg = static_cast<PanelGroup*>(parent());
        DockGroup* uiDock = tg->get_dock();
        const Pack* uiDockPack = uiDock->get_pack();
        auto uiDockGroup = uiDock->parent();

        // Color the dock area with the background color
        color_dock_group(FL_BACKGROUND_COLOR);
                        
        // Hide the panel if there are no panels 
        if (uiDockPack->children() == 0)
        {
            uiDockGroup->hide();
                            
            auto dropWindow = static_cast<DropWindow*>(uiDock->top_window());
            Fl_Flex* flex = dropWindow->workspace;
            flex->layout();
            flex->redraw();
        }
    }
    
    int DragButton::would_dock()
    {
        int x2 = 0, y2 = 0;
        int cx, cy;
        cx = Fl::event_x_root(); // Where did the release occur...
        cy = Fl::event_y_root();
        x2 = x1 - cx;
        y2 = y1 - cy;
        x2 = (x2 > 0) ? x2 : (-x2);
        y2 = (y2 > 0) ? y2 : (-y2);
#ifndef NDEBUG
        std::cerr << "\tcx, cy=" << cx << " " << cy << std::endl;
        std::cerr << "\tx1, y1=" << x1 << " " << y1 << std::endl;
        std::cerr << "\tx2, y2=" << x2 << " " << y2 << std::endl;
#endif
        if ((x2 > kDragMinDistance) || (y2 > kDragMinDistance))
        { // test for a docking event
            // See if anyone is able to accept a dock with this widget
            // How to find the dock window? Search 'em all for now...
            for (Fl_Window* win = Fl::first_window(); win;
                 win = Fl::next_window(win))
            {
                // Get the co-ordinates of each window
                int ex = win->x_root();
                int ey = win->y_root();
                int ew = win->w();
                int eh = win->h();

                // Are we inside the boundary of the window?
                if (win->visible() && (cx > ex) && (cy > ey) &&
                    (cx < (ew + ex)) && (cy < (eh + ey)))
                {
                    if (auto dropWindow = dynamic_cast<DropWindow*>(win))
                    {
                        return dropWindow->valid_drop();
                    }
                }
            }
        }
        return 0;
    }
    
} // namespace mrv

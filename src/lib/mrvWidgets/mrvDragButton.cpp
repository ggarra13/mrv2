// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cassert>

#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>

#include "mrvWidgets/mrvDropWindow.h"
#include "mrvWidgets/mrvDragButton.h"
#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvPanelConstants.h"
#include "mrvWidgets/mrvPanelGroup.h"

#include "mrvUI/mrvDesktop.h"

#define DEBUG_EVENTS 0

namespace
{
    // Minimum distance to move for undocking
    const int kDragMinDistance = 10;
}

namespace mrv
{
    DragButton::DragButton(int x, int y, int w, int h, const char* l) :
        Fl_Box(x, y, w, h, l)
    {
        was_docked = true; // Assume we have NOT just undocked...
    }
    
    DragButton::~DragButton()
    {
    }

    void DragButton::update_drag()
    {
        // This is the stable calculation.
        int current_mouse_x, current_mouse_y;
        get_global_coords(current_mouse_x, current_mouse_y);

        int new_x, new_y;
        new_x = winx + (current_mouse_x - fromx);
        new_y = winy + (current_mouse_y - fromy);
        
        window()->position(new_x, new_y);
        if (window()->parent())
            window()->parent()->init_sizes();
        
        // Update docking highlight feedback.
        if (would_dock()) {
            color_dock_group(FL_DARK_YELLOW);
            show_dock_group();
        } else {
            hide_dock_group();
        }
    }
    
    int DragButton::handle(int event)
    {
        int ret = Fl_Box::handle(event);
        
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
            switch (event)
            {
            case FL_PUSH: // downclick in button creates cursor offsets
                get_global_coords(fromx, fromy);
                get_window_coords(winx, winy);
                if (window()) _drag_screen = window()->screen_num();
                return 1;
            case FL_DRAG:
                
                if (was_docked)
                {
                    
                    was_docked = false;
                    get_global_coords(fromx, fromy);
                    get_window_coords(winx, winy);
                    if (window()) _drag_screen = window()->screen_num();
                }

#ifdef _WIN32
                if (window() && window()->screen_num() != _drag_screen)
                {
                    // The OS/FLTK just snapped the window into a new DPI space. 
                    // Re-anchor the drag to prevent the infinite scaling loop!
                    get_global_coords(fromx, fromy);
                    get_window_coords(winx, winy);
                    _drag_screen = window()->screen_num();
                }
#endif
                
                update_drag();
                return 1;
            case FL_RELEASE:
                
                update_drag();
                    
                // Finalize the dock state.
                if (would_dock())
                {
                    color_dock_group(FL_BACKGROUND_COLOR);
                    show_dock_group();
                    tg->dock_grp();
                }
                else
                {
                    hide_dock_group();
                }
                
                return 1;                    
            default:
                break; // Ignore other events.
            }
            return ret;
        }

        // OK, so we must be docked - are we being dragged out of the dock?
        switch (event)
        {
        case FL_PUSH: // downclick in button creates cursor offsets
            get_global_coords(fromx, fromy);
            ret = 1;
            break;

        case FL_DRAG:
            // If the drag has moved further than the drag_min distance
            // then invoke an un-docking
            get_global_coords(x2, y2);
            x2 -= fromx;
            y2 -= fromy;
            x2 = (x2 > 0) ? x2 : (-x2);
            y2 = (y2 > 0) ? y2 : (-y2);
            if ((x2 > kDragMinDistance) || (y2 > kDragMinDistance))
            {
                tg->undock_grp(false); // undock the window
                was_docked = true;     // note that we *just now* undocked

                int posX, posY;
                get_global_coords(posX, posY);
                
                PanelWindow* tw = tg->get_window();
                tw->position(posX, posY);
                if (tw->parent())
                    tw->parent()->init_sizes();

                // Save requested position (same coord system as mouse events)
                undock_x = posX;
                undock_y = posY;
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
        if (c != widget->color())
        {
            widget->color(c);
            widget->redraw();
        }
    }
    
    void DragButton::show_dock_group()
    {
        PanelGroup* tg = static_cast<PanelGroup*>(parent());
        DockGroup* uiDock = tg->get_dock();
        auto uiDockGroup = uiDock->parent();
        
        // Show the dock group if it is hidden
        if (!uiDockGroup->visible())
        {
            uiDockGroup->show();

            auto dropWindow =
                static_cast<DropWindow*>(uiDockGroup->top_window());
            Fl_Flex* flex = dropWindow->workspace;
            flex->layout();
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
        if (uiDockPack->children() == 0 &&
            uiDockGroup->visible())
        {
            uiDockGroup->hide();

            auto dropWindow =
                static_cast<DropWindow*>(uiDockGroup->top_window());
            Fl_Flex* flex = dropWindow->workspace;
            flex->layout();
        }
    }
    
    int DragButton::would_dock()
    {
        int X, Y;
        get_window_coords(X, Y);
        
        for (Fl_Window* win = Fl::first_window(); win;
             win = Fl::next_window(win))
        {
            if (auto dropWindow = dynamic_cast<DropWindow*>(win))
            {
                return dropWindow->valid_drop(X, Y);
            }
        }
        return 0;
    }

    
    void DragButton::get_global_coords(int& X, int& Y)
    {
        X = Fl::event_x_root();
        Y = Fl::event_y_root();
    }

    void DragButton::get_window_coords(int& X, int& Y)
    {
        PanelGroup* tg = (PanelGroup*)parent();
        if (tg->docked())
        {
            X = 0;
            Y = 0;
        }
        else
        {
            PanelWindow* tw = tg->get_window();
            assert(tw);
            X = tw->x_root();
            Y = tw->y_root();
        }
    }
    
} // namespace mrv

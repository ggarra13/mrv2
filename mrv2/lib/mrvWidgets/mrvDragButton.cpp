// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cassert>

#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/names.h>

#include "mrvWidgets/mrvDropWindow.h"
#include "mrvWidgets/mrvDragButton.h"
#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvPanelConstants.h"
#include "mrvWidgets/mrvPanelGroup.h"

#include "mrvUI/mrvDesktop.h"


namespace
{
    const float kTimeout = 0.025;
    const int kDragMinDistance = 10;
}

namespace mrv
{

    void move_cb(DragButton* v)
    {
        v->set_position();
        Fl::repeat_timeout(kTimeout, (Fl_Timeout_Handler)move_cb, v);   
    }

    DragButton::DragButton(int x, int y, int w, int h, const char* l) :
        Fl_Box(x, y, w, h, l)
    {
        was_docked = 0; // Assume we have NOT just undocked...
        if (desktop::Wayland())
            use_timeout = true;
    }
    
    DragButton::~DragButton()
    {
        if (use_timeout)
            Fl::remove_timeout((Fl_Timeout_Handler)move_cb, this);
    }

    void DragButton::set_position()
    {
        PanelGroup* tg = (PanelGroup*)parent();
        if (tg->docked()) return;
        
        PanelWindow* tw = tg->get_window();
        if (!tw) return;

        if (tw->x() == currentX && tw->y() == currentY)
            return;

        // Avoid Wayland's compositor going crazy
        int moveX = std::abs(currentX - lastX);
        int moveY = std::abs(currentY - lastY);
        if (moveX > kMaxMove || moveY > kMaxMove)
            return;
        tw->position(currentX, currentY);
        lastX = currentX;
        lastY = currentY;
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
                get_global_coords(x1, y1);
                get_window_coords(xoff, yoff);

                currentX = lastX = xoff;
                currentY = lastY = yoff;
                
                xoff -= x1;
                yoff -= y1;
                ret = 1;
        
                if (use_timeout)
                {
                    if (!Fl::has_timeout((Fl_Timeout_Handler)move_cb, this))
                        Fl::add_timeout(
                            kTimeout, (Fl_Timeout_Handler)move_cb, this);
                }
                break;

            case FL_DRAG: // drag the button (and its parent window) around the
            {          // screen
                if (was_docked)
                {
                    // Need to init offsets, we probably got here following a
                    // drag from the dock, so the PUSH (above) will not have
                    // happened.
                    was_docked = 0;
                    get_global_coords(x1, y1);
                    get_window_coords(xoff, yoff);

                    currentX = xoff;
                    currentY = yoff;
                
                    xoff -= x1;
                    yoff -= y1;
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

                int posX, posY;
                get_global_coords(posX, posY);

                posX += xoff;
                posY += yoff;


                if (use_timeout)
                {
                    currentX = posX;
                    currentY = posY;
                }
                else
                {
                    PanelWindow* tw = tg->get_window();
                    assert(tw);
                    tw->position(posX, posY);
                    lastX = posX;
                    lastY = posY;
                }
                ret = 1;
                break;
            }
            case FL_RELEASE:
            {
                if (use_timeout)
                    Fl::remove_timeout((Fl_Timeout_Handler)move_cb, this);

                
                int dock_attempt = would_dock();
                    
                if (dock_attempt)
                {
                    color_dock_group(FL_BACKGROUND_COLOR);
                    show_dock_group();
                    tg->dock_grp();
                }
                ret = 1;
            }
            break;

            default:
                break;
            }
            return (ret);
        }

        // OK, so we must be docked - are we being dragged out of the dock?
        if (use_timeout)
            Fl::remove_timeout((Fl_Timeout_Handler)move_cb, this);
        
        switch (event)
        {
        case FL_PUSH: // downclick in button creates cursor offsets
            get_global_coords(x1, y1);
            ret = 1;
            break;

        case FL_DRAG:
            // If the drag has moved further than the drag_min distance
            // then invoke an un-docking
            get_global_coords(x2, y2);
            x2 -= x1;
            y2 -= y1;
            x2 = (x2 > 0) ? x2 : (-x2);
            y2 = (y2 > 0) ? y2 : (-y2);
            if ((x2 > kDragMinDistance) || (y2 > kDragMinDistance))
            {
                tg->undock_grp(); // undock the window
                was_docked = -1;  // note that we *just now* undocked
                
                int posX, posY;
                get_global_coords(posX, posY);
            
                if (use_timeout)
                {
                    currentX = lastX = posX;
                    currentY = lastY = posY;
                    if (!Fl::has_timeout((Fl_Timeout_Handler)move_cb, this))
                        Fl::add_timeout(kTimeout, (Fl_Timeout_Handler)move_cb,
                                        this);
                }
                else
                {
                    PanelWindow* tw = tg->get_window();
                    assert(tw);
                    tw->position(posX, posY);
                    lastX = posX;
                    lastY = posY;
                }                
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
            flex->init_sizes();
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
        if (uiDockPack->children() == 0 &&
            uiDockGroup->visible())
        {
            uiDockGroup->hide();
                            
            auto dropWindow = static_cast<DropWindow*>(uiDock->top_window());
            Fl_Flex* flex = dropWindow->workspace;
            flex->layout();
            flex->init_sizes();
            flex->redraw();
        }
    }
    
    int DragButton::would_dock()
    {
        // See if anyone is able to accept a dock with this widget
        // How to find the dock window? Search 'em all for now...
        for (Fl_Window* win = Fl::first_window(); win;
             win = Fl::next_window(win))
        {
            if (auto dropWindow = dynamic_cast<DropWindow*>(win))
            {
                return dropWindow->valid_drop(lastX - xoff, lastY - yoff);
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

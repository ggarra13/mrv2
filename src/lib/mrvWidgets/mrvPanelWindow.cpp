// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cassert>
#include <algorithm>

#include <FL/Fl_Button.H>
#include <FL/Fl.H>

#include "mrvWidgets/mrvPanelConstants.h"
#include "mrvWidgets/mrvPanelWindow.h"
#include "mrvWidgets/mrvPanelGroup.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvPanels/mrvPanelsAux.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvApp.h"

#include "mrViewer.h"

namespace mrv
{

    std::vector<PanelWindow*> PanelWindow::active_list;
    
    // Dummy close button callback
    static void cb_ignore(void)
    {
        // Just shrug off the close callback...
    }

    // constructors
    PanelWindow::PanelWindow(int x, int y, int w, int h, const char* l) :
        Fl_Double_Window(x, y, w, h, l)
    {
        allow_expand_outside_parent();
        
        newX = x;
        newY = y;
        newW = w;
        newH = h;
        
        create_dockable_window();
        box(FL_FLAT_BOX);
    }

    PanelWindow::PanelWindow(int w, int h, const char* l) :
        Fl_Double_Window(w, h, l)
    {
        allow_expand_outside_parent();
        
        newX = x();
        newY = y();
        newW = w;
        newH = h;
                
        create_dockable_window();
        box(FL_FLAT_BOX);
    }

    PanelWindow::~PanelWindow()
    {
        active_list.erase(std::remove(active_list.begin(), active_list.end(),
                                      this), active_list.end());
    }
    
    void PanelWindow::create_dockable_window()
    {
        clear_border();
        set_non_modal();
        resizable(this);
        callback((Fl_Callback*)cb_ignore);
        active_list.push_back(this);
    }

    void PanelWindow::resize(int X, int Y, int W, int H)
    {
        if (App::ui->uiMain->is_wayland_resize())
        {
            W = w();
            H = h();


            //
            // Sanity check.  Try to avoid positioning windows outside the
            // screen work area.
            //
            Fl_Window* top = top_window();
            int screen_num = top->screen_num();
            int minX, minY, maxW, maxH;
            Fl::screen_work_area(minX, minY, maxW, maxH, screen_num);
            
            if (X > minX + maxW)
            {
                X -= ((X - maxW) + W);
            }
            if (Y > minY + maxH)
            {
                Y -= ((Y - maxH) + H);
            }
        }
        Fl_Double_Window::resize(X, Y, W, H);
    }

    void PanelWindow::show_all(void)
    {
        for (auto window : active_list)
        {
            window->show();
        }
    }

    void PanelWindow::hide_all(void)
    {
        for (auto window : active_list)
        {
            window->hide();
        }
    }

    void PanelWindow::set_cursor(int ex, int ey)
    {
        valid = Direction::kNone;
        const int rdir = x_root() + w() - kMargin;
        const int ldir = x_root() + kMargin;
        const int bdir = y_root() + h() - kMargin;
        const int tdir = y_root() + kMargin;
        const int dragbar = y_root() + kTitleBar;
        if (ex >= rdir)
            valid |= Direction::kRight;
        if (ex <= ldir)
            valid |= Direction::kLeft;
        if (ey >= bdir)
            valid |= Direction::kBottom;
        if (ey <= tdir)
            valid |= Direction::kTop;
        if (valid == Direction::kRight || valid == Direction::kLeft)
            cursor(FL_CURSOR_WE);
        else if (valid == Direction::kTop || valid == Direction::kBottom)
            cursor(FL_CURSOR_NS);
        else if (valid == Direction::kTopRight)
            cursor(FL_CURSOR_NESW);
        else if (valid == Direction::kTopLeft)
            cursor(FL_CURSOR_NWSE);
        else if (valid == Direction::kBottomRight)
            cursor(FL_CURSOR_NWSE);
        else if (valid == Direction::kBottomLeft)
            cursor(FL_CURSOR_NESW);
        else if (ey >= dragbar)
        {
            cursor(FL_CURSOR_DEFAULT);
        }
    }

    int PanelWindow::handle(int event)
    {
        int ret = Fl_Double_Window::handle(event);
        
        int ex = Fl::event_x_root();
        int ey = Fl::event_y_root();
        switch (event)
        {
        case FL_UNFOCUS:
        case FL_FOCUS:
            return 1;
        case FL_ENTER:
        {
            set_cursor(ex, ey);
            return 1;
        }
        case FL_LEAVE:
            cursor(FL_CURSOR_DEFAULT);
            return 1;
        case FL_MOVE:
        {
            int ret = Fl_Double_Window::handle(event);
            set_cursor(ex, ey);
            return ret;
        }
        case FL_PUSH:
        {
            set_cursor(ex, ey);
            if (valid != Direction::kNone)
            {
                newX = x();
                newY = y();
                newW = w();
                newH = h();
            
                last_x = ex;
                last_y = ey;

                return 1;
            }
            break;
        }
        case FL_DRAG:
        {
            int diffX = ex - last_x;
            int diffY = ey - last_y;
            if (valid == Direction::kRight)
            {
                if (diffX == 0)
                    return 0;
                newW += diffX;
            }
            else if (valid == Direction::kLeft)
            {
                if (diffX == 0)
                    return 0;
                newX += diffX;
                newW -= diffX;
            }
            else if (valid == Direction::kBottom)
            {
                if (diffY == 0)
                    return 0;
                newH += diffY;
            }
            else if (valid == Direction::kBottomRight)
            {
                if (diffY == 0 && diffX == 0)
                    return 0;
                newW += diffX;
                newH += diffY;
            }
            else if (valid == Direction::kBottomLeft)
            {
                if (diffY == 0 && diffX == 0)
                    return 0;
                newX += diffX;
                newW -= diffX;
                newH += diffY;
            }
            else if (valid == Direction::kTop)
            {
                if (diffY == 0)
                    return 0;
                newY += diffY;
                newH -= diffY;
            }
            else if (valid == Direction::kTopRight)
            {
                if (diffY == 0 && diffX == 0)
                    return 0;
                newY += diffY;
                newW += diffX;
                newH -= diffY;
            }
            else if (valid == Direction::kTopLeft)
            {
                if (diffY == 0 && diffX == 0)
                    return 0;
                newX += diffX;
                newY += diffY;
                newW -= diffX;
                newH -= diffY;
            }

            if (valid != Direction::kNone)
            {
                if (newW < kMinWidth)
                {
                    newX = x();
                    newW = kMinWidth;
                }
                if (newH < kMinHeight)
                {
                    newY = y();
                    newH = kMinHeight;
                }
                    
                resize(newX, newY, newW, newH);
                if (parent())
                    parent()->init_sizes();
            }
            
            last_x = ex;
            last_y = ey;
            return 1;
        }
        case FL_RELEASE:
        {
            auto settings = App::app->settings();
            PanelGroup* gp = static_cast< PanelGroup* >(child(0));
            auto dragger = gp->get_dragger();
            const std::string label = gp->label();
            std::string prefix = "gui/" + label;
            std::string key;

            key = prefix + "/WindowW";
            settings->setValue(key, w());

            key = prefix + "/WindowH";

            // Only store height if it is not a growing panel/window, like
            // the Files, Compare or Playlist panels.
            if (panel::isPanelWithHeight(label))
            {
                settings->setValue(key, h());
            }
            else
            {
                settings->setValue(key, 0);
            }

            return 1;
        }
        }
        return ret;
    }
} // namespace mrv

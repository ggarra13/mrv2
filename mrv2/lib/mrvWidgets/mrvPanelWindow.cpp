// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Button.H>

#include "mrvPanelWindow.h"
#include "mrvEventHeader.h"

#include "mrvWidgets/mrvPanelGroup.h"

#include "mrvPanels/mrvPanelsAux.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/App.h"

namespace mrv
{

    // HACK:: This just stores the PanelWindowdows in a static array. I'm too
    // lazy
    //        to make a proper linked list to store these in...
    PanelWindow* PanelWindow::active_list[TW_MAX_FLOATERS]; // list of active
                                                            // PanelWindows
    short PanelWindow::active = 0; // count of active tool windows
    constexpr int kMinWidth = 150;
    constexpr int kMinHeight = 150;

    // Dummy close button callback
    static void cb_ignore(void)
    {
        // Just shrug off the close callback...
    }

    // constructors
    PanelWindow::PanelWindow(int x, int y, int w, int h, const char* l) :
        Fl_Double_Window(x, y, w, h, l)
    {
        create_dockable_window();
        box(FL_FLAT_BOX);
    }

    PanelWindow::PanelWindow(int w, int h, const char* l) :
        Fl_Double_Window(w, h, l)
    {
        create_dockable_window();
        box(FL_FLAT_BOX);
    }

    // destructor
    PanelWindow::~PanelWindow()
    {
        active_list[idx] = nullptr;
        active--;
    }

    // construction function
    void PanelWindow::create_dockable_window()
    {
        static int first_window = 1;
        // window list intialisation...
        // this is a nasty hack, should make a proper list
        if (first_window)
        {
            first_window = 0;
            for (short i = 0; i < TW_MAX_FLOATERS; i++)
                active_list[i] = nullptr;
        }
        // find an empty index
        for (short i = 0; i < TW_MAX_FLOATERS; i++)
        {
            if (!active_list[i])
            {
                idx = i;
                active_list[idx] = this;
                active++;
                clear_border();
                set_non_modal();
                resizable(this);
                callback((Fl_Callback*)cb_ignore);
                return;
            }
        }
        // if we get here, the list is probably full, what a hack.
        // FIX THIS:: At present, we will get a non-modal window with
        // decorations as a default instead...
        set_non_modal();
    }

    // show all the active floating windows
    void PanelWindow::show_all(void)
    {
        if (active)
        {
            for (short i = 0; i < TW_MAX_FLOATERS; i++)
            {
                if (active_list[i])
                    active_list[i]->show();
            }
        }
    }

    // hide all the active floating windows
    void PanelWindow::hide_all(void)
    {
        if (active)
        {
            for (short i = 0; i < TW_MAX_FLOATERS; i++)
            {
                if (active_list[i])
                    active_list[i]->hide();
            }
        }
    }

    constexpr int kDiff = 3;

    void PanelWindow::set_cursor(int ex, int ey)
    {
        cursor(FL_CURSOR_DEFAULT);
        valid = Direction::kNone;
        int rdir = x() + w() - kDiff;
        int ldir = x() + kDiff;
        int bdir = y() + h() - kDiff;
        int tdir = y() + kDiff;
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
    }

    void PanelWindow::resize(int X, int Y, int W, int H)
    {
        int screen = Fl::screen_num(X, Y);
        int minX, minY, maxW, maxH;
        Fl::screen_work_area(minX, minY, maxW, maxH, screen);

        // Don't allow taking the window beyond the titlebar
        // -20 for the size of the dragger in the group.
        const int DH = 20;
        int maxY = minY + maxH - DH;
        if (Y < minY)
            Y = minY;
        else if (Y > maxY)
            Y = maxY;

        // Don't allow scaling the window beyond the bottom of the screen
        if (Y + H > maxY)
            H = maxY - Y;

        assert(W > 0);
        assert(H > 0);

        Fl_Double_Window::resize(X, Y, W, H);
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
            set_cursor(ex, ey);
            return 1;
        }
        case FL_PUSH:
        {
            set_cursor(ex, ey);
            if (valid != Direction::kNone)
            {
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
                if (w() + diffX > kMinWidth)
                    size(w() + diffX, h());
            }
            else if (valid == Direction::kLeft)
            {
                if (diffX == 0)
                    return 0;
                if (w() - diffX > kMinWidth)
                    resize(x() + diffX, y(), w() - diffX, h());
            }
            else if (valid == Direction::kBottom)
            {
                if (diffY == 0)
                    return 0;
                if (h() + diffY > kMinHeight)
                    size(w(), h() + diffY);
            }
            else if (valid == Direction::kBottomRight)
            {
                if (diffY == 0 && diffX == 0)
                    return 0;
                if (h() + diffY > kMinHeight && w() + diffX > kMinWidth)
                    size(w() + diffX, h() + diffY);
            }
            else if (valid == Direction::kBottomLeft)
            {
                if (diffY == 0 && diffX == 0)
                    return 0;
                if (h() + diffY > kMinHeight && w() - diffX > kMinWidth)
                    resize(x() + diffX, y(), w() - diffX, h() + diffY);
            }
            else if (valid == Direction::kTop)
            {
                if (diffY == 0)
                    return 0;
                if (h() - diffY > kMinHeight)
                    resize(x(), y() + diffY, w(), h() - diffY);
            }
            else if (valid == Direction::kTopRight)
            {
                if (diffY == 0 && diffX == 0)
                    return 0;
                if (h() - diffY > kMinHeight && w() + diffX > kMinWidth)
                    resize(x(), y() + diffY, w() + diffX, h() - diffY);
            }
            else if (valid == Direction::kTopLeft)
            {
                if (diffY == 0 && diffX == 0)
                    return 0;
                if (h() - diffY > kMinHeight && w() - diffX > kMinWidth)
                    resize(x() + diffX, y() + diffY, w() - diffX, h() - diffY);
            }
            last_x = ex;
            last_y = ey;
            return 1;
        }
        case FL_RELEASE:
        {
            auto settings = App::app->settings();
            PanelGroup* gp = static_cast< PanelGroup* >(child(0));
            // gp->layout();
            auto dragger = gp->get_dragger();
            const std::string label = gp->label();
            std::string prefix = "gui/" + label;
            std::string key;

            key = prefix + "/WindowW";
            settings->setValue(key, w());

            key = prefix + "/WindowH";

            // Only store height if it is not a growing panel/window, like
            // the Files, Compare or Playlist panel.
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

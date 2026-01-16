// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvApp/mrvApp.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvWidgets/mrvStatusBar.h"

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvOS.h"

namespace mrv
{
    void StatusBar::clear_cb(StatusBar* o)
    {
        o->clear();
    }

    void StatusBar::all_ok_cb(StatusBar* o)
    {
        o->restore_message();
    }

    StatusBar::StatusBar(int X, int Y, int W, int H, const char* L) :
        Fl_Group(X, Y, W, H, L)
    {
        box(FL_FLAT_BOX);
    }

    void StatusBar::timeout(float seconds)
    {
        seconds_ = seconds;
    }

    void StatusBar::restore_message()
    {
        std::string label = _("Everything OK. ");
        label += os::getDesktop();
        const std::string gpu = os::getGPUVendor();
        label += " - " + gpu;
        if (gpu == _("GPU: Unknown"))
            Fl::repeat_timeout(1.0, (Fl_Timeout_Handler)all_ok_cb, this);
        
        copy_label(label.c_str());
    }

    void StatusBar::default_message()
    {
        Fl::add_timeout(1.0, (Fl_Timeout_Handler)all_ok_cb, this);
    }

    void StatusBar::save_colors()
    {
        color_ = color();
        labelcolor_ = labelcolor();
    }

    void StatusBar::restore_colors()
    {
        color(color_);
        labelcolor(labelcolor_);
        redraw();
    }

    void StatusBar::clear()
    {
        Fl_Group::copy_label("");
        box(FL_FLAT_BOX);
        restore_colors();
    }

    void StatusBar::error(const char* msg)
    {
        // Mark the message on a red background
        if (strlen(msg) == 0)
        {
            restore_colors();
        }
        else
        {
            color(0xFF000000);
            labelcolor(FL_BLACK);
        }
        Fl_Group::copy_label(msg);
        redraw();
        Fl::remove_timeout((Fl_Timeout_Handler)clear_cb, this);
        Fl::remove_timeout((Fl_Timeout_Handler)all_ok_cb, this);
        Fl::add_timeout(seconds_, (Fl_Timeout_Handler)clear_cb, this);
        Fl::add_timeout(seconds_ * 2, (Fl_Timeout_Handler)all_ok_cb, this);
    }

    void StatusBar::warning(const char* msg)
    {
        // Mark the message on an orange background
        if (strlen(msg) == 0)
        {
            restore_colors();
        }
        else
        {
            color(0xFF800000);
            labelcolor(FL_BLACK);
        }
        Fl_Group::copy_label(msg);
        redraw();
        Fl::remove_timeout((Fl_Timeout_Handler)clear_cb, this);
        Fl::remove_timeout((Fl_Timeout_Handler)all_ok_cb, this);
        Fl::add_timeout(seconds_, (Fl_Timeout_Handler)clear_cb, this);
        Fl::add_timeout(seconds_ * 2, (Fl_Timeout_Handler)all_ok_cb, this);
    }

    int StatusBar::handle(int event)
    {
        if (event == FL_PUSH)
        {
            if (Fl::event_button1())
            {
                if (Fl::event_clicks() > 0)
                {
                    panel::logs_panel_cb(nullptr, App::ui);
                    Fl::event_clicks(0);
                }
                return 1;
            }
        }
        
        return Fl_Group::handle(event);
    }
    
} // namespace mrv

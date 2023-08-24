// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>

#include <tlCore/Vector.h>

#include "mrvCore/mrvString.h"
#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvCallbacks.h"

#include "mrvWidgets/mrvFileButton.h"
#include "mrvWidgets/mrvFileDragger.h"

#include "mrvEdit/mrvEditCallbacks.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"

namespace
{
} // namespace

namespace mrv
{
    FileButton::FileButton(int X, int Y, int W, int H, const char* L) :
        ClipButton(X, Y, W, H, L)
    {
    }

    int FileButton::handle(int event)
    {
        switch (event)
        {
        case FL_FOCUS:
        case FL_UNFOCUS:
            return 1;
        case FL_ENTER:
            take_focus();
            break;
        case FL_LEAVE:
            Fl::focus(0);
            break;
        case FL_KEYDOWN:
        case FL_KEYUP:
        {
            if (value())
            {
                unsigned rawkey = Fl::event_key();
                if (Fl::focus() == this &&
                    (rawkey == FL_Delete || rawkey == FL_BackSpace))
                {
                    close_current_cb(this, App::ui);
                    return 1;
                }
                return 0;
                break;
            }
        }
        case FL_RELEASE:
        {
            if (drag)
            {
                int X = Fl::event_x_root();
                int Y = Fl::event_y_root();
                math::Vector2i pos(X, Y);

                ViewerUI* ui = App::ui;
                math::Box2i box(
                    ui->uiTimeline->x() + ui->uiMain->x(),
                    ui->uiTimeline->y() + ui->uiMain->y(), ui->uiTimeline->w(),
                    ui->uiTimeline->h());

                delete drag;
                drag = nullptr;

                if (box.contains(pos))
                {
                    const std::string text = label();
                    stringArray lines;
                    split_string(lines, text, "\n");
                    std::string filename = lines[0] + lines[1];
                    add_clip_to_timeline(filename, index, ui);
                }
            }
            break;
        }
        case FL_DRAG:
        {
            if (Fl::event_button1())
            {
                if (!drag)
                {
                    drag = FileDragger::create();
                    drag->image(image());
                    auto window = drag->window();
                    window->always_on_top(true);
                }
                int X = Fl::event_x_root();
                int Y = Fl::event_y_root();
                auto window = drag->window();
                window->position(X, Y);
            }
            break;
        }
        case FL_PUSH:
            if (value() && Fl::event_button3())
            {
                Fl_Menu_Button menu(x(), y(), w(), h());
                menu.type(Fl_Menu_Button::POPUP3);
                menu.add(
                    _("&File/&Clone"), 0, (Fl_Callback*)clone_file_cb, 0, 0);
                menu.add(
                    _("&File/&Refresh Cache"), 0,
                    (Fl_Callback*)refresh_file_cache_cb, 0, 0);
                menu.add(
                    _("&Copy/&Filename"), 0, (Fl_Callback*)copy_filename_cb, 0,
                    0);
                menu.add(
                    _("&Show/In &File Manager"), 0,
                    (Fl_Callback*)file_manager_cb, 0, 0);

                menu.popup();
                return 1;
            }
            break;
        }
        return Fl_Button::handle(event);
    }
} // namespace mrv

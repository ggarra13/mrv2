// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>

#include <tlCore/Vector.h>

#include "mrvCore/mrvString.h"
#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvIO.h"

#include "mrvWidgets/mrvFileButton.h"
#include "mrvWidgets/mrvFileDragger.h"

#include "mrvEdit/mrvEditCallbacks.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "filebutton";
} // namespace

namespace mrv
{
    using namespace panel;

    struct FileButton::Private
    {
        size_t index = 0;
        FileDragger* drag = nullptr;
        math::Vector2i push;
    };

    FileButton::FileButton(int X, int Y, int W, int H, const char* L) :
        _p(new Private),
        ClipButton(X, Y, W, H, L)
    {
    }

    FileButton::~FileButton()
    {
        TLRENDER_P();
        delete p.drag;
        p.drag = nullptr;
    }

    void FileButton::setIndex(size_t value)
    {
        _p->index = value;
    }

    int FileButton::handle(int event)
    {
        TLRENDER_P();

        switch (event)
        {
        case FL_FOCUS:
        case FL_UNFOCUS:
            return 1;
        case FL_ENTER:
            take_focus();
            return 1;
        case FL_LEAVE:
            Fl::focus(App::ui->uiView);
            // If user pressed button3 while dragging, the window would freeze.
            // This should avoid that.
            if (Fl::event_button3())
            {
                delete p.drag;
                p.drag = nullptr;
            }
            return 1;
        case FL_KEYDOWN:
        case FL_KEYUP:
        {
            if (value() && Fl::focus() == this)
            {
                unsigned rawkey = Fl::event_key();
                if (rawkey == FL_Delete || rawkey == FL_BackSpace)
                {
                    close_current_cb(this, App::ui);
                    return 1;
                }
                return 0;
            }
            return 0;
        }
        case FL_RELEASE:
        {
            if (p.drag)
            {
                int X = Fl::event_x_root();
                int Y = Fl::event_y_root();
                math::Vector2i pos(X, Y);

                ViewerUI* ui = App::ui;
                auto model = ui->app->filesModel();
                math::Box2i box(
                    ui->uiTimeline->x() + ui->uiMain->x(),
                    ui->uiTimeline->y() + ui->uiMain->y(), ui->uiTimeline->w(),
                    ui->uiTimeline->h());

                delete p.drag;
                p.drag = nullptr;

                if (box.contains(pos))
                {
                    add_clip_to_timeline(p.index, ui);
                    return 1;
                }

                if (playlistPanel)
                {
                    math::Box2i box = playlistPanel->box();
                    if (playlistPanel->is_panel())
                    {
                        auto w = window();
                        pos.x -= w->x();
                        pos.y -= w->y();
                    }

                    if (box.contains(pos))
                    {
                        playlistPanel->add(pos, p.index, ui);
                        return 1;
                    }
                }
                return 1;
            }
            break;
        }
        case FL_DRAG:
        {
            math::Vector2i cursorPos(Fl::event_x_root(), Fl::event_y_root());
            if (std::abs(cursorPos.y - p.push.y) < 2)
            {
                return 1;
            }
            if (Fl::event_button1())
            {
                const std::string text = label();
                auto lines = string::split(text, '\n');
                std::string filename = lines[0] + lines[1];

                if (!p.drag)
                {
                    p.drag = FileDragger::create();
                    p.drag->image(image());
                    auto window = p.drag->window();
                    window->always_on_top(true);
                }

                if (p.drag)
                {
                    value(1);
                    redraw();
                    int X = Fl::event_x_root();
                    int Y = Fl::event_y_root();
                    auto window = p.drag->window();
                    window->position(X, Y);
                    return 1;
                }
            }
            break;
        }
        case FL_PUSH:
            if (value() && Fl::event_button3() && !p.drag)
            {
                Fl_Menu_Button menu(x(), y(), w(), h());
                menu.type(Fl_Menu_Button::POPUP3);
                menu.textsize(12);
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
            if (Fl::event_button1())
            {
                p.push = math::Vector2i(Fl::event_x_root(), Fl::event_y_root());
            }
            if (p.drag)
                return 1;
            break;
        }
        return ClipButton::handle(event);
    }
} // namespace mrv

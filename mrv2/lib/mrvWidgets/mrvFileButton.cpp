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
    struct FileButton::Private
    {
        size_t index = 0;
        FileDragger* drag = nullptr;
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
        case FL_ENTER:
        case FL_LEAVE:
            return 1;
        case FL_KEYDOWN:
        case FL_KEYUP:
        {
            if (value())
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

                if (playlistPanel)
                {
                    ViewerUI* ui = App::ui;

                    math::Box2i box = playlistPanel->box();
                    if (playlistPanel->is_panel())
                    {
                        auto w = window();
                        pos.x -= w->x();
                        pos.y -= w->y();
                    }

                    delete p.drag;
                    p.drag = nullptr;

                    if (box.contains(pos))
                    {
                        const std::string text = label();
                        stringArray lines;
                        split_string(lines, text, "\n");
                        std::string filename = lines[0] + lines[1];
                        playlistPanel->add(pos, filename, p.index, ui);
                        // add_clip_to_timeline(filename, p.index, ui);
                        return 1;
                    }
                }

                ViewerUI* ui = App::ui;
                math::Box2i box(
                    ui->uiTimeline->x() + ui->uiMain->x(),
                    ui->uiTimeline->y() + ui->uiMain->y(), ui->uiTimeline->w(),
                    ui->uiTimeline->h());

                delete p.drag;
                p.drag = nullptr;

                if (box.contains(pos))
                {
                    const std::string text = label();
                    stringArray lines;
                    split_string(lines, text, "\n");
                    std::string filename = lines[0] + lines[1];
                    add_clip_to_timeline(filename, p.index, ui);
                    return 1;
                }
                return 0;
            }
            break;
        }
        case FL_DRAG:
        {
            if (Fl::event_button1())
            {
                const std::string text = label();
                stringArray lines;
                split_string(lines, text, "\n");
                std::string filename = lines[0] + lines[1];
                file::Path path(filename);
                auto extension = string::toLower(path.getExtension());

                if (extension != ".otio")
                {
                    if (!p.drag)
                    {
                        p.drag = FileDragger::create();
                        p.drag->image(image());
                        auto window = p.drag->window();
                        window->always_on_top(true);
                    }
                    int X = Fl::event_x_root();
                    int Y = Fl::event_y_root();
                    auto window = p.drag->window();
                    window->position(X, Y);
                    return 1;
                }
                return 0;
            }
            break;
        }
        case FL_PUSH:
            if (value() && Fl::event_button3() && !p.drag)
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
        return ClipButton::handle(event);
    }
} // namespace mrv

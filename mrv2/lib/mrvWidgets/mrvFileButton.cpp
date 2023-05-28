// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>

#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvCallbacks.h"

#include "mrvWidgets/mrvFileButton.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"

namespace
{
    void clone_cb(Fl_Menu_* m, void* d)
    {
        auto app = mrv::App::application();
        auto model = app->filesModel();
        auto ui = app->ui;
        if (model->observeFiles()->getSize() < 1)
            return;

        auto item = model->observeA()->get();
        app->open(item->path.get(), item->audioPath.get());

        auto newItem = model->observeA()->get();
        newItem->inOutRange = item->inOutRange;
        newItem->speed = item->speed;
        newItem->audioOffset = item->audioOffset;
        newItem->videoLayer = item->videoLayer;
        newItem->loop = item->loop;
        newItem->playback = item->playback;
        newItem->currentTime = item->currentTime;
        newItem->annotations = item->annotations;

        auto player = ui->uiView->getTimelinePlayer();
        player->setAllAnnotations(newItem->annotations);
    }
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
        case FL_PUSH:
            if (value() && Fl::event_button3())
            {
                Fl_Menu_Button menu(x(), y(), w(), h());
                menu.type(Fl_Menu_Button::POPUP3);
                menu.add(_("&Clone/&File"), 0, (Fl_Callback*)clone_cb, 0, 0);
                menu.popup();
                return 1;
            }
            break;
        }
        return Fl_Button::handle(event);
    }
} // namespace mrv

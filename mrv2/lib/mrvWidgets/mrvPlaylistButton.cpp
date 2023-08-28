// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>

#include <tlCore/Vector.h>

#include "mrvCore/mrvString.h"
#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvCallbacks.h"

#include "mrvWidgets/mrvPlaylistButton.h"
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
    struct PlaylistButton::Private
    {
        size_t index = 0;
        FileDragger* drag = nullptr;
    };

    PlaylistButton::PlaylistButton(int X, int Y, int W, int H, const char* L) :
        _p(new Private),
        ClipButton(X, Y, W, H, L)
    {
    }

    PlaylistButton::~PlaylistButton()
    {
        TLRENDER_P();
        delete p.drag;
        p.drag = nullptr;
    }

    void PlaylistButton::setIndex(size_t value)
    {
        _p->index = value;
    }

    int PlaylistButton::handle(int event)
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
        }
        return ClipButton::handle(event);
    }
} // namespace mrv

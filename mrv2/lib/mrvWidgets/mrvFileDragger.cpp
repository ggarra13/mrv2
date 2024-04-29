// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>

#include "mrvWidgets/mrvMainWindow.h"
#include "mrvWidgets/mrvFileDragger.h"

#include "mrViewer.h"

#include <FL/platform.H>

namespace mrv
{
    struct FileDragger::Private
    {
        MainWindow* parent;
        MainWindow* window;
        Fl_Box* box;
    };

    FileDragger::FileDragger() :
        _p(new Private)
    {
        TLRENDER_P();

        int X = Fl::event_x_root();
        int Y = Fl::event_y_root();
#ifdef FLTK_USE_WAYLAND
        if (fl_wl_display())
        {
            p.parent = App::ui->uiMain;

            Fl_Group::current(p.parent);

            X -= p.parent->x();
            Y -= p.parent->y();
        }
        else
#endif
        {
            Fl_Group::current(0);
        }

        const int W = 128;
        const int H = 80;

        p.window = new MainWindow(X, Y, W, H);
        p.window->border(0);
        p.window->begin();
        p.box = new Fl_Box(0, 0, W, H);
        p.window->end();
        p.window->show();
        Fl_Group::current(0);
    }

    FileDragger::~FileDragger()
    {
        TLRENDER_P();

        p.window->hide();
        delete p.window;
        p.window = nullptr;
    }

    FileDragger* FileDragger::create()
    {
        return new FileDragger();
    }

    void FileDragger::image(Fl_Image* image)
    {
        TLRENDER_P();

        p.box->image(image);
        p.box->redraw();
    }

    MainWindow* FileDragger::window() const
    {
        return _p->window;
    }

} // namespace mrv

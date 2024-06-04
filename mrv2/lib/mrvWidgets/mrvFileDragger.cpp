// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>

#include "mrvWidgets/mrvFileDragger.h"

#include "mrvUI/mrvDesktop.h"

#include "mrViewer.h"

#include <FL/platform.H>

namespace mrv
{
    struct FileDragger::Private
    {
        MainWindow* window;
        Fl_Box* box;
    };

    FileDragger::FileDragger() :
        _p(new Private)
    {
        TLRENDER_P();

        int X = Fl::event_x_root();
        int Y = Fl::event_y_root();

        if (desktop::Wayland())
        {
            Fl_Group::current(App::ui->uiMain);
        }
        else
        {
            Fl_Group::current(0);
        }

        const int W = 128;
        const int H = 80;

        p.window = new MainWindow(X, Y, W, H);
        p.window->allow_expand_outside_parent();
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

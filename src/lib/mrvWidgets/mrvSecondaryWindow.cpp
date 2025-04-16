// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvSecondaryWindow.h"
#include "mrvWidgets/mrvMainWindow.h"

#include "mrvApp/mrvApp.h"
#include "mrvApp/mrvSettingsObject.h"

#include "mrViewer.h"

namespace mrv
{
    struct SecondaryWindow::Private
    {
        ViewerUI* ui = nullptr;
        MainWindow* mainWindow = nullptr;
        MyViewport* viewport = nullptr;
    };

    SecondaryWindow::SecondaryWindow(ViewerUI* ui) :
        _p(new Private)
    {
        TLRENDER_P();

        p.ui = ui;

        int X = 30, Y = 30, W = 1280, H = 720;

        SettingsObject* settings = ui->app->settings();
        std::string key;
        std_any value;

        key = "gui/Secondary/WindowX";
        value = settings->getValue<std::any>(key);
        X = std_any_empty(value) ? X : std_any_cast<int>(value);

        key = "gui/Secondary/WindowY";
        value = settings->getValue<std::any>(key);
        Y = std_any_empty(value) ? Y : std_any_cast<int>(value);

        key = "gui/Secondary/WindowW";
        value = settings->getValue<std::any>(key);
        W = std_any_empty(value) ? W : std_any_cast<int>(value);

        key = "gui/Secondary/WindowH";
        value = settings->getValue<std::any>(key);
        H = std_any_empty(value) ? H : std_any_cast<int>(value);

        Fl_Group::current(0);
        p.mainWindow = new MainWindow(X, Y, W, H, "SecondaryWindow");
        p.mainWindow->labeltype(FL_NO_LABEL);
        p.mainWindow->begin();

        p.viewport = new MyViewport(0, 0, W, H);
        p.viewport->main(ui); // needed
        p.viewport->end();

        p.viewport->setContext(ui->app->getContext());
        p.viewport->setFrameView(true);

        p.mainWindow->resizable(p.viewport);

        p.mainWindow->end();

        std::string label = "Secondary ";
        label += ui->uiMain->label();
        p.mainWindow->copy_label(label.c_str());

        p.mainWindow->callback(
            [](Fl_Widget* w, void* d)
            {
                ViewerUI* ui = (ViewerUI*)d;
                toggle_secondary_cb(nullptr, ui);
            },
            ui);
    }

    SecondaryWindow::~SecondaryWindow()
    {
        TLRENDER_P();

        SettingsObject* settings = p.ui->app->settings();

        auto player = p.viewport->getTimelinePlayer();
        timeline::Playback playback = timeline::Playback::Forward;
        if (player)
            playback = player->playback();

        p.viewport->stop();
        player = p.viewport->getTimelinePlayer();
        if (player)
            player->setPlayback(playback);

        delete p.mainWindow;
        p.mainWindow = nullptr;
        p.viewport = nullptr;
    }

    void SecondaryWindow::save() const
    {
        TLRENDER_P();

        SettingsObject* settings = p.ui->app->settings();

        std::string key = "gui/Secondary/Window/Visible";
        MainWindow* w = p.mainWindow;
        int visible = w->visible();
        settings->setValue(key, visible);

        if (visible)
        {
            key = "gui/Secondary/WindowX";
            settings->setValue(key, w->x());

            key = "gui/Secondary/WindowY";
            settings->setValue(key, w->y());

            key = "gui/Secondary/WindowW";
            settings->setValue(key, w->w());

            key = "gui/Secondary/WindowH";
            settings->setValue(key, w->h());
        }
    }

    MainWindow* SecondaryWindow::window() const
    {
        return _p->mainWindow;
    }

    MyViewport* SecondaryWindow::viewport() const
    {
        return _p->viewport;
    }

} // namespace mrv

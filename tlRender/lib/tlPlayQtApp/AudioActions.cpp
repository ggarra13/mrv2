// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/AudioActions.h>

#include <tlPlayQtApp/App.h>

#include <tlPlay/AudioModel.h>

namespace tl
{
    namespace play_qt
    {
        struct AudioActions::Private
        {
            App* app = nullptr;

            QMap<QString, QAction*> actions;
            QScopedPointer<QMenu> menu;

            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
        };

        AudioActions::AudioActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["VolumeUp"] = new QAction(this);
            p.actions["VolumeUp"]->setText(tr("Volume Up"));
            p.actions["VolumeUp"]->setShortcut(QKeySequence(Qt::Key_Period));

            p.actions["VolumeDown"] = new QAction(this);
            p.actions["VolumeDown"]->setText(tr("Volume Down"));
            p.actions["VolumeDown"]->setShortcut(QKeySequence(Qt::Key_Comma));

            p.actions["Mute"] = new QAction(this);
            p.actions["Mute"]->setCheckable(true);
            p.actions["Mute"]->setText(tr("Mute"));
            QIcon muteIcon;
            muteIcon.addFile(
                ":/Icons/Volume.svg", QSize(20, 20), QIcon::Normal, QIcon::Off);
            muteIcon.addFile(
                ":/Icons/Mute.svg", QSize(20, 20), QIcon::Normal, QIcon::On);
            p.actions["Mute"]->setIcon(muteIcon);
            p.actions["Mute"]->setShortcut(QKeySequence(Qt::Key_M));
            p.actions["Mute"]->setToolTip(tr("Mute the audio"));

            p.menu.reset(new QMenu);
            p.menu->setTitle(tr("&Audio"));
            p.menu->addAction(p.actions["VolumeUp"]);
            p.menu->addAction(p.actions["VolumeDown"]);
            p.menu->addAction(p.actions["Mute"]);

            _actionsUpdate();

            connect(
                p.actions["VolumeUp"], &QAction::triggered,
                [app] { app->audioModel()->volumeUp(); });

            connect(
                p.actions["VolumeDown"], &QAction::triggered,
                [app] { app->audioModel()->volumeDown(); });

            connect(
                p.actions["Mute"], &QAction::toggled,
                [app](bool value) { app->audioModel()->setMute(value); });

            p.muteObserver = observer::ValueObserver<bool>::create(
                app->audioModel()->observeMute(),
                [this](bool) { _actionsUpdate(); });
        }

        AudioActions::~AudioActions() {}

        const QMap<QString, QAction*>& AudioActions::actions() const
        {
            return _p->actions;
        }

        QMenu* AudioActions::menu() const
        {
            return _p->menu.get();
        }

        void AudioActions::_actionsUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.actions["Mute"]);
                p.actions["Mute"]->setChecked(p.app->audioModel()->isMuted());
            }
        }
    } // namespace play_qt
} // namespace tl

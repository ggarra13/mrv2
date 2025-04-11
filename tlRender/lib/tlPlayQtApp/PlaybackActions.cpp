// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/PlaybackActions.h>

#include <tlPlayQtApp/App.h>

#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>

#include <QActionGroup>

namespace tl
{
    namespace play_qt
    {
        struct PlaybackActions::Private
        {
            App* app = nullptr;

            QSharedPointer<qt::TimelinePlayer> player;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;

            QScopedPointer<QMenu> menu;
            QScopedPointer<QMenu> speedMenu;
        };

        PlaybackActions::PlaybackActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["Stop"] = new QAction(parent);
            p.actions["Stop"]->setData(QVariant::fromValue<timeline::Playback>(
                timeline::Playback::Stop));
            p.actions["Stop"]->setCheckable(true);
            p.actions["Stop"]->setText(tr("Stop Playback"));
            p.actions["Stop"]->setIcon(QIcon(":/Icons/PlaybackStop.svg"));
            p.actions["Stop"]->setShortcut(QKeySequence(Qt::Key_K));
            p.actions["Stop"]->setToolTip(tr("Stop playback"));
            p.actions["Forward"] = new QAction(parent);
            p.actions["Forward"]->setData(
                QVariant::fromValue<timeline::Playback>(
                    timeline::Playback::Forward));
            p.actions["Forward"]->setCheckable(true);
            p.actions["Forward"]->setText(tr("Forward Playback"));
            p.actions["Forward"]->setIcon(QIcon(":/Icons/PlaybackForward.svg"));
            p.actions["Forward"]->setShortcut(QKeySequence(Qt::Key_L));
            p.actions["Forward"]->setToolTip(tr("Forward playback"));
            p.actions["Reverse"] = new QAction(parent);
            p.actions["Reverse"]->setData(
                QVariant::fromValue<timeline::Playback>(
                    timeline::Playback::Reverse));
            p.actions["Reverse"]->setCheckable(true);
            p.actions["Reverse"]->setText(tr("Reverse Playback"));
            p.actions["Reverse"]->setIcon(QIcon(":/Icons/PlaybackReverse.svg"));
            p.actions["Reverse"]->setShortcut(QKeySequence(Qt::Key_J));
            p.actions["Reverse"]->setToolTip(tr("Reverse playback"));
            p.actionGroups["Playback"] = new QActionGroup(this);
            p.actionGroups["Playback"]->setExclusive(true);
            p.actionGroups["Playback"]->addAction(p.actions["Stop"]);
            p.actionGroups["Playback"]->addAction(p.actions["Forward"]);
            p.actionGroups["Playback"]->addAction(p.actions["Reverse"]);
            p.actions["Toggle"] = new QAction(parent);
            p.actions["Toggle"]->setText(tr("Toggle Playback"));
            p.actions["Toggle"]->setShortcut(QKeySequence(Qt::Key_Space));

            p.actions["JumpBack1s"] = new QAction(parent);
            p.actions["JumpBack1s"]->setText(tr("Jump Back 1s"));
            p.actions["JumpBack1s"]->setShortcut(
                QKeySequence(Qt::SHIFT | Qt::Key_J));
            p.actions["JumpBack1s"]->setToolTip(tr("Jump back 1 second"));
            p.actions["JumpBack10s"] = new QAction(parent);
            p.actions["JumpBack10s"]->setText(tr("Jump Back 10s"));
            p.actions["JumpBack10s"]->setShortcut(
                QKeySequence(Qt::CTRL | Qt::Key_J));
            p.actions["JumpBack10s"]->setToolTip(tr("Jump back 10 seconds"));
            p.actions["JumpForward1s"] = new QAction(parent);
            p.actions["JumpForward1s"]->setText(tr("Jump Forward 1s"));
            p.actions["JumpForward1s"]->setShortcut(
                QKeySequence(Qt::SHIFT | Qt::Key_L));
            p.actions["JumpForward1s"]->setToolTip(tr("Jump forward 1 second"));
            p.actions["JumpForward10s"] = new QAction(parent);
            p.actions["JumpForward10s"]->setText(tr("Jump Forward 10s"));
            p.actions["JumpForward10s"]->setShortcut(
                QKeySequence(Qt::CTRL | Qt::Key_L));
            p.actions["JumpForward10s"]->setToolTip(
                tr("Jump forward 10 seconds"));

            p.actions["Loop"] = new QAction(parent);
            p.actions["Loop"]->setData(
                QVariant::fromValue<timeline::Loop>(timeline::Loop::Loop));
            p.actions["Loop"]->setCheckable(true);
            p.actions["Loop"]->setText(tr("Loop Playback"));
            p.actions["Once"] = new QAction(parent);
            p.actions["Once"]->setData(
                QVariant::fromValue<timeline::Loop>(timeline::Loop::Once));
            p.actions["Once"]->setCheckable(true);
            p.actions["Once"]->setText(tr("Playback Once"));
            p.actions["PingPong"] = new QAction(parent);
            p.actions["PingPong"]->setData(
                QVariant::fromValue<timeline::Loop>(timeline::Loop::PingPong));
            p.actions["PingPong"]->setCheckable(true);
            p.actions["PingPong"]->setText(tr("Ping-Pong Playback"));
            p.actionGroups["Loop"] = new QActionGroup(this);
            p.actionGroups["Loop"]->setExclusive(true);
            p.actionGroups["Loop"]->addAction(p.actions["Loop"]);
            p.actionGroups["Loop"]->addAction(p.actions["Once"]);
            p.actionGroups["Loop"]->addAction(p.actions["PingPong"]);

            p.actions["TimeUnits/Frames"] = new QAction(parent);
            p.actions["TimeUnits/Frames"]->setData(
                static_cast<int>(timeline::TimeUnits::Frames));
            p.actions["TimeUnits/Frames"]->setCheckable(true);
            p.actions["TimeUnits/Frames"]->setText(tr("Frames"));
            p.actions["TimeUnits/Seconds"] = new QAction(parent);
            p.actions["TimeUnits/Seconds"]->setData(
                static_cast<int>(timeline::TimeUnits::Seconds));
            p.actions["TimeUnits/Seconds"]->setCheckable(true);
            p.actions["TimeUnits/Seconds"]->setText(tr("Seconds"));
            p.actions["TimeUnits/Timecode"] = new QAction(parent);
            p.actions["TimeUnits/Timecode"]->setData(
                static_cast<int>(timeline::TimeUnits::Timecode));
            p.actions["TimeUnits/Timecode"]->setCheckable(true);
            p.actions["TimeUnits/Timecode"]->setText(tr("Timecode"));
            p.actionGroups["TimeUnits"] = new QActionGroup(this);
            p.actionGroups["TimeUnits"]->addAction(
                p.actions["TimeUnits/Frames"]);
            p.actionGroups["TimeUnits"]->addAction(
                p.actions["TimeUnits/Seconds"]);
            p.actionGroups["TimeUnits"]->addAction(
                p.actions["TimeUnits/Timecode"]);

            const QList<double> speeds = {1.0,  3.0,  6.0,   9.0,  12.0,
                                          16.0, 18.0, 23.98, 24.0, 29.97,
                                          30.0, 48.0, 59.94, 60.0, 120.0};
            for (auto i : speeds)
            {
                const QString key = QString("Speed/%1").arg(i);
                p.actions[key] = new QAction(parent);
                p.actions[key]->setData(i);
                p.actions[key]->setText(QString("%1").arg(i, 0, 'f', 2));
            }
            p.actions["Speed/Default"] = new QAction(parent);
            p.actions["Speed/Default"]->setData(0.F);
            p.actions["Speed/Default"]->setText(tr("Default"));
            p.actions["Speed/Default"]->setToolTip(
                tr("Default timeline speed"));
            p.actionGroups["Speed"] = new QActionGroup(this);
            for (auto i : speeds)
            {
                const QString key = QString("Speed/%1").arg(i);
                p.actionGroups["Speed"]->addAction(p.actions[key]);
            }
            p.actionGroups["Speed"]->addAction(p.actions["Speed/Default"]);

            p.menu.reset(new QMenu);
            p.menu->setTitle(tr("&Playback"));
            p.menu->addAction(p.actions["Stop"]);
            p.menu->addAction(p.actions["Forward"]);
            p.menu->addAction(p.actions["Reverse"]);
            p.menu->addAction(p.actions["Toggle"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["JumpBack1s"]);
            p.menu->addAction(p.actions["JumpBack10s"]);
            p.menu->addAction(p.actions["JumpForward1s"]);
            p.menu->addAction(p.actions["JumpForward10s"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["Loop"]);
            p.menu->addAction(p.actions["Once"]);
            p.menu->addAction(p.actions["PingPong"]);

            p.speedMenu.reset(new QMenu);
            for (auto i : speeds)
            {
                const QString key = QString("Speed/%1").arg(i);
                p.speedMenu->addAction(p.actions[key]);
            }
            p.speedMenu->addSeparator();
            p.speedMenu->addAction(p.actions["Speed/Default"]);

            _playerUpdate(app->player());
            _actionsUpdate();

            connect(
                p.actions["Toggle"], &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->togglePlayback();
                    }
                });

            connect(
                p.actions["JumpBack1s"], &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->timeAction(
                            timeline::TimeAction::JumpBack1s);
                    }
                });
            connect(
                p.actions["JumpBack10s"], &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->timeAction(
                            timeline::TimeAction::JumpBack10s);
                    }
                });
            connect(
                p.actions["JumpForward1s"], &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->timeAction(
                            timeline::TimeAction::JumpForward1s);
                    }
                });
            connect(
                p.actions["JumpForward10s"], &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->timeAction(
                            timeline::TimeAction::JumpForward10s);
                    }
                });

            connect(
                p.actionGroups["Playback"], &QActionGroup::triggered,
                [this](QAction* action)
                {
                    if (_p->player)
                    {
                        _p->player->setPlayback(
                            action->data().value<timeline::Playback>());
                    }
                });

            connect(
                p.actionGroups["Loop"], &QActionGroup::triggered,
                [this](QAction* action)
                {
                    if (_p->player)
                    {
                        _p->player->setLoop(
                            action->data().value<timeline::Loop>());
                    }
                });

            connect(
                p.actionGroups["TimeUnits"], &QActionGroup::triggered,
                [app](QAction* action)
                {
                    app->timeObject()->setTimeUnits(
                        static_cast<timeline::TimeUnits>(
                            action->data().toInt()));
                });

            connect(
                p.actionGroups["Speed"], &QActionGroup::triggered,
                [this](QAction* action)
                {
                    if (_p->player)
                    {
                        const float speed = action->data().toFloat();
                        _p->player->setSpeed(
                            speed > 0.F ? speed : _p->player->defaultSpeed());
                    }
                });

            connect(
                app, &App::playerChanged,
                [this](const QSharedPointer<qt::TimelinePlayer>& value)
                { _playerUpdate(value); });
        }

        PlaybackActions::~PlaybackActions() {}

        const QMap<QString, QAction*>& PlaybackActions::actions() const
        {
            return _p->actions;
        }

        QMenu* PlaybackActions::menu() const
        {
            return _p->menu.get();
        }

        QMenu* PlaybackActions::speedMenu() const
        {
            return _p->speedMenu.get();
        }

        void PlaybackActions::_playbackCallback(timeline::Playback value)
        {
            TLRENDER_P();
            const QSignalBlocker blocker(p.actionGroups["Playback"]);
            for (auto action : p.actionGroups["Playback"]->actions())
            {
                if (action->data().value<timeline::Playback>() == value)
                {
                    action->setChecked(true);
                    break;
                }
            }
        }

        void PlaybackActions::_loopCallback(timeline::Loop value)
        {
            TLRENDER_P();
            const QSignalBlocker blocker(p.actionGroups["Loop"]);
            for (auto action : p.actionGroups["Loop"]->actions())
            {
                if (action->data().value<timeline::Loop>() == value)
                {
                    action->setChecked(true);
                    break;
                }
            }
        }

        void PlaybackActions::_playerUpdate(
            const QSharedPointer<qt::TimelinePlayer>& player)
        {
            TLRENDER_P();
            if (p.player)
            {
                disconnect(
                    p.player.get(),
                    SIGNAL(playbackChanged(tl::timeline::Playback)), this,
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                disconnect(
                    p.player.get(), SIGNAL(loopChanged(tl::timeline::Loop)),
                    this, SLOT(_loopCallback(tl::timeline::Loop)));
            }

            p.player = player;

            if (p.player)
            {
                connect(
                    p.player.get(),
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                connect(
                    p.player.get(), SIGNAL(loopChanged(tl::timeline::Loop)),
                    SLOT(_loopCallback(tl::timeline::Loop)));
            }

            _actionsUpdate();
        }

        void PlaybackActions::_actionsUpdate()
        {
            TLRENDER_P();

            QList<QString> keys = p.actions.keys();
            for (auto i : keys)
            {
                p.actions[i]->setEnabled(p.player.get());
            }

            if (p.player)
            {
                {
                    QSignalBlocker blocker(p.actionGroups["Playback"]);
                    for (auto action : p.actionGroups["Playback"]->actions())
                    {
                        if (action->data().value<timeline::Playback>() ==
                            p.player->playback())
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
                {
                    QSignalBlocker blocker(p.actionGroups["Loop"]);
                    for (auto action : p.actionGroups["Loop"]->actions())
                    {
                        if (action->data().value<timeline::Loop>() ==
                            p.player->loop())
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
            }
            else
            {
                {
                    QSignalBlocker blocker(p.actionGroups["Playback"]);
                    p.actions["Stop"]->setChecked(true);
                }
                {
                    QSignalBlocker blocker(p.actionGroups["Loop"]);
                    p.actions["Loop"]->setChecked(true);
                }
            }

            {
                QSignalBlocker blocker(p.actionGroups["TimeUnits"]);
                for (auto action : p.actionGroups["TimeUnits"]->actions())
                {
                    if (static_cast<timeline::TimeUnits>(
                            action->data().toInt()) ==
                        p.app->timeObject()->timeUnits())
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
        }
    } // namespace play_qt
} // namespace tl

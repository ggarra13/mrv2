// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/TimelineActions.h>

#include <tlPlayQtApp/MainWindow.h>

#include <tlQtWidget/TimelineWidget.h>

#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>

#include <QActionGroup>

namespace tl
{
    namespace play_qt
    {
        struct TimelineActions::Private
        {
            MainWindow* mainWindow = nullptr;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;

            QScopedPointer<QMenu> menu;
        };

        TimelineActions::TimelineActions(
            MainWindow* mainWindow, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.mainWindow = mainWindow;

            p.actions["Input"] = new QAction(parent);
            p.actions["Input"]->setCheckable(true);
            p.actions["Input"]->setText(tr("Enable Input"));

            p.actions["Editable"] = new QAction(parent);
            p.actions["Editable"]->setCheckable(true);
            p.actions["Editable"]->setText(tr("Editable"));

            p.actions["EditAssociatedClips"] = new QAction(parent);
            p.actions["EditAssociatedClips"]->setCheckable(true);
            p.actions["EditAssociatedClips"]->setText(
                tr("Edit Associated Clips"));

            p.actions["FrameView"] = new QAction(parent);
            p.actions["FrameView"]->setCheckable(true);
            p.actions["FrameView"]->setText(tr("Frame Timeline View"));

            p.actions["ScrollToCurrentFrame"] = new QAction(parent);
            p.actions["ScrollToCurrentFrame"]->setCheckable(true);
            p.actions["ScrollToCurrentFrame"]->setText(
                tr("Scroll To Current Frame"));

            p.actions["StopOnScrub"] = new QAction(parent);
            p.actions["StopOnScrub"]->setCheckable(true);
            p.actions["StopOnScrub"]->setText(
                tr("Stop Playback When Scrubbing"));

            p.actions["FirstTrack"] = new QAction(parent);
            p.actions["FirstTrack"]->setCheckable(true);
            p.actions["FirstTrack"]->setText(tr("First Track Only"));

            p.actions["TrackInfo"] = new QAction(parent);
            p.actions["TrackInfo"]->setCheckable(true);
            p.actions["TrackInfo"]->setText(tr("Track Information"));

            p.actions["ClipInfo"] = new QAction(parent);
            p.actions["ClipInfo"]->setCheckable(true);
            p.actions["ClipInfo"]->setText(tr("Clip Information"));

            p.actions["Thumbnails"] = new QAction(parent);
            p.actions["Thumbnails"]->setCheckable(true);
            p.actions["Thumbnails"]->setText(tr("Thumbnails"));
            p.actions["ThumbnailsSize/Small"] = new QAction(this);
            p.actions["ThumbnailsSize/Small"]->setData(100);
            p.actions["ThumbnailsSize/Small"]->setCheckable(true);
            p.actions["ThumbnailsSize/Small"]->setText(tr("Small"));
            p.actions["ThumbnailsSize/Medium"] = new QAction(this);
            p.actions["ThumbnailsSize/Medium"]->setData(200);
            p.actions["ThumbnailsSize/Medium"]->setCheckable(true);
            p.actions["ThumbnailsSize/Medium"]->setText(tr("Medium"));
            p.actions["ThumbnailsSize/Large"] = new QAction(this);
            p.actions["ThumbnailsSize/Large"]->setData(300);
            p.actions["ThumbnailsSize/Large"]->setCheckable(true);
            p.actions["ThumbnailsSize/Large"]->setText(tr("Large"));
            p.actionGroups["ThumbnailsSize"] = new QActionGroup(this);
            p.actionGroups["ThumbnailsSize"]->addAction(
                p.actions["ThumbnailsSize/Small"]);
            p.actionGroups["ThumbnailsSize"]->addAction(
                p.actions["ThumbnailsSize/Medium"]);
            p.actionGroups["ThumbnailsSize"]->addAction(
                p.actions["ThumbnailsSize/Large"]);

            p.actions["Transitions"] = new QAction(parent);
            p.actions["Transitions"]->setCheckable(true);
            p.actions["Transitions"]->setText(tr("Transitions"));

            p.actions["Markers"] = new QAction(parent);
            p.actions["Markers"]->setCheckable(true);
            p.actions["Markers"]->setText(tr("Markers"));

            p.menu.reset(new QMenu);
            p.menu->setTitle(tr("&Timeline"));
            p.menu->addAction(p.actions["Input"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["Editable"]);
            p.menu->addAction(p.actions["EditAssociatedClips"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["FrameView"]);
            p.menu->addAction(p.actions["ScrollToCurrentFrame"]);
            p.menu->addAction(p.actions["StopOnScrub"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["FirstTrack"]);
            p.menu->addAction(p.actions["TrackInfo"]);
            p.menu->addAction(p.actions["ClipInfo"]);
            p.menu->addAction(p.actions["Thumbnails"]);
            auto thumbnailsSizeMenu = p.menu->addMenu(tr("Thumbnails Size"));
            thumbnailsSizeMenu->addAction(p.actions["ThumbnailsSize/Small"]);
            thumbnailsSizeMenu->addAction(p.actions["ThumbnailsSize/Medium"]);
            thumbnailsSizeMenu->addAction(p.actions["ThumbnailsSize/Large"]);
            p.menu->addAction(p.actions["Transitions"]);
            p.menu->addAction(p.actions["Markers"]);

            _actionsUpdate();

            connect(
                p.actions["Input"], &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto options = timelineWidget->itemOptions();
                    options.inputEnabled = value;
                    timelineWidget->setItemOptions(options);
                });

            connect(
                p.actions["Editable"], &QAction::toggled,
                [mainWindow](bool value)
                { mainWindow->timelineWidget()->setEditable(value); });

            connect(
                p.actions["EditAssociatedClips"], &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto options = timelineWidget->itemOptions();
                    options.editAssociatedClips = value;
                    timelineWidget->setItemOptions(options);
                });

            connect(
                p.actions["FrameView"], &QAction::toggled,
                [mainWindow](bool value)
                { mainWindow->timelineWidget()->setFrameView(value); });

            connect(
                p.actions["ScrollToCurrentFrame"], &QAction::toggled,
                [mainWindow](bool value) {
                    mainWindow->timelineWidget()->setScrollToCurrentFrame(
                        value);
                });

            connect(
                p.actions["StopOnScrub"], &QAction::toggled,
                [mainWindow](bool value)
                { mainWindow->timelineWidget()->setStopOnScrub(value); });

            connect(
                p.actions["FirstTrack"], &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto options = timelineWidget->displayOptions();
                    options.tracks.clear();
                    if (value)
                    {
                        options.tracks.push_back(0);
                    }
                    timelineWidget->setDisplayOptions(options);
                });

            connect(
                p.actions["TrackInfo"], &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto options = timelineWidget->displayOptions();
                    options.trackInfo = value;
                    timelineWidget->setDisplayOptions(options);
                });

            connect(
                p.actions["ClipInfo"], &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto options = timelineWidget->displayOptions();
                    options.clipInfo = value;
                    timelineWidget->setDisplayOptions(options);
                });

            connect(
                p.actions["Thumbnails"], &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto options = timelineWidget->displayOptions();
                    options.thumbnails = value;
                    timelineWidget->setDisplayOptions(options);
                });

            connect(
                p.actionGroups["ThumbnailsSize"], &QActionGroup::triggered,
                [mainWindow](QAction* action)
                {
                    const int value = action->data().toInt();
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto options = timelineWidget->displayOptions();
                    options.thumbnailHeight = value;
                    timelineWidget->setDisplayOptions(options);
                });

            connect(
                p.actions["Transitions"], &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto options = timelineWidget->displayOptions();
                    options.transitions = value;
                    timelineWidget->setDisplayOptions(options);
                });

            connect(
                p.actions["Markers"], &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto options = timelineWidget->displayOptions();
                    options.markers = value;
                    timelineWidget->setDisplayOptions(options);
                });
        }

        TimelineActions::~TimelineActions() {}

        const QMap<QString, QAction*>& TimelineActions::actions() const
        {
            return _p->actions;
        }

        QMenu* TimelineActions::menu() const
        {
            return _p->menu.get();
        }

        void TimelineActions::_actionsUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.actions["Input"]);
                const auto options =
                    p.mainWindow->timelineWidget()->itemOptions();
                p.actions["Input"]->setChecked(options.inputEnabled);
            }
            {
                QSignalBlocker blocker(p.actions["Editable"]);
                p.actions["Editable"]->setChecked(
                    p.mainWindow->timelineWidget()->isEditable());
            }
            {
                QSignalBlocker blocker(p.actions["EditAssociatedClips"]);
                const auto options =
                    p.mainWindow->timelineWidget()->itemOptions();
                p.actions["EditAssociatedClips"]->setChecked(
                    options.editAssociatedClips);
            }
            {
                QSignalBlocker blocker(p.actions["FrameView"]);
                p.actions["FrameView"]->setChecked(
                    p.mainWindow->timelineWidget()->hasFrameView());
            }
            {
                QSignalBlocker blocker(p.actions["ScrollToCurrentFrame"]);
                p.actions["ScrollToCurrentFrame"]->setChecked(
                    p.mainWindow->timelineWidget()->hasScrollToCurrentFrame());
            }
            {
                QSignalBlocker blocker(p.actions["StopOnScrub"]);
                p.actions["StopOnScrub"]->setChecked(
                    p.mainWindow->timelineWidget()->hasStopOnScrub());
            }
            {
                QSignalBlocker blocker(p.actions["FirstTrack"]);
                const auto options =
                    p.mainWindow->timelineWidget()->displayOptions();
                p.actions["FirstTrack"]->setChecked(!options.tracks.empty());
            }
            {
                QSignalBlocker blocker(p.actions["TrackInfo"]);
                const auto options =
                    p.mainWindow->timelineWidget()->displayOptions();
                p.actions["TrackInfo"]->setChecked(options.trackInfo);
            }
            {
                QSignalBlocker blocker(p.actions["ClipInfo"]);
                const auto options =
                    p.mainWindow->timelineWidget()->displayOptions();
                p.actions["ClipInfo"]->setChecked(options.clipInfo);
            }
            {
                QSignalBlocker blocker(p.actions["Thumbnails"]);
                const auto options =
                    p.mainWindow->timelineWidget()->displayOptions();
                p.actions["Thumbnails"]->setChecked(options.thumbnails);
            }
            {
                QSignalBlocker blocker(p.actionGroups["ThumbnailsSize"]);
                p.actions["ThumbnailsSize/Small"]->setChecked(false);
                p.actions["ThumbnailsSize/Medium"]->setChecked(false);
                p.actions["ThumbnailsSize/Large"]->setChecked(false);
                const auto options =
                    p.mainWindow->timelineWidget()->displayOptions();
                for (auto action : p.actionGroups["ThumbnailsSize"]->actions())
                {
                    if (action->data().toInt() == options.thumbnailHeight)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            {
                QSignalBlocker blocker(p.actions["Transitions"]);
                const auto options =
                    p.mainWindow->timelineWidget()->displayOptions();
                p.actions["Transitions"]->setChecked(options.transitions);
            }
            {
                QSignalBlocker blocker(p.actions["Markers"]);
                const auto options =
                    p.mainWindow->timelineWidget()->displayOptions();
                p.actions["Markers"]->setChecked(options.markers);
            }
        }
    } // namespace play_qt
} // namespace tl

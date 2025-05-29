// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/TimelineActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <tlTimelineUI/TimelineWidget.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct TimelineActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void TimelineActions::_init(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.actions["Input"] = std::make_shared<ui::Action>(
                "Enable Input",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options =
                            mainWindow->getTimelineWidget()->getItemOptions();
                        options.inputEnabled = value;
                        mainWindow->getTimelineWidget()->setItemOptions(
                            options);
                    }
                });

            p.actions["Editable"] = std::make_shared<ui::Action>(
                "Editable",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineWidget()->setEditable(value);
                    }
                });

            p.actions["EditAssociatedClips"] = std::make_shared<ui::Action>(
                "Edit Associated Clips",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options =
                            mainWindow->getTimelineWidget()->getItemOptions();
                        options.editAssociatedClips = value;
                        mainWindow->getTimelineWidget()->setItemOptions(
                            options);
                    }
                });

            p.actions["FrameView"] = std::make_shared<ui::Action>(
                "Frame Timeline View",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineWidget()->setFrameView(value);
                    }
                });

            p.actions["ScrollToCurrentFrame"] = std::make_shared<ui::Action>(
                "Scroll To Current Frame",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineWidget()
                            ->setScrollToCurrentFrame(value);
                    }
                });

            p.actions["StopOnScrub"] = std::make_shared<ui::Action>(
                "Stop Playback When Scrubbing",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineWidget()->setStopOnScrub(value);
                    }
                });

            p.actions["FirstTrack"] = std::make_shared<ui::Action>(
                "First Track Only",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()
                                           ->getDisplayOptions();
                        options.tracks.clear();
                        if (value)
                        {
                            options.tracks.push_back(0);
                        }
                        mainWindow->getTimelineWidget()->setDisplayOptions(
                            options);
                    }
                });

            p.actions["TrackInfo"] = std::make_shared<ui::Action>(
                "Track Information",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()
                                           ->getDisplayOptions();
                        options.trackInfo = value;
                        mainWindow->getTimelineWidget()->setDisplayOptions(
                            options);
                    }
                });

            p.actions["ClipInfo"] = std::make_shared<ui::Action>(
                "Clip Information",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()
                                           ->getDisplayOptions();
                        options.clipInfo = value;
                        mainWindow->getTimelineWidget()->setDisplayOptions(
                            options);
                    }
                });

            p.actions["Thumbnails"] = std::make_shared<ui::Action>(
                "Thumbnails",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()
                                           ->getDisplayOptions();
                        options.thumbnails = value;
                        mainWindow->getTimelineWidget()->setDisplayOptions(
                            options);
                    }
                });

            p.actions["Thumbnails100"] = std::make_shared<ui::Action>(
                "Small",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()
                                           ->getDisplayOptions();
                        options.thumbnailHeight = 100;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setDisplayOptions(
                            options);
                    }
                });

            p.actions["Thumbnails200"] = std::make_shared<ui::Action>(
                "Medium",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()
                                           ->getDisplayOptions();
                        options.thumbnailHeight = 200;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setDisplayOptions(
                            options);
                    }
                });

            p.actions["Thumbnails300"] = std::make_shared<ui::Action>(
                "Large",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()
                                           ->getDisplayOptions();
                        options.thumbnailHeight = 300;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setDisplayOptions(
                            options);
                    }
                });

            p.actions["Transitions"] = std::make_shared<ui::Action>(
                "Transitions",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()
                                           ->getDisplayOptions();
                        options.transitions = value;
                        mainWindow->getTimelineWidget()->setDisplayOptions(
                            options);
                    }
                });

            p.actions["Markers"] = std::make_shared<ui::Action>(
                "Markers",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()
                                           ->getDisplayOptions();
                        options.markers = value;
                        mainWindow->getTimelineWidget()->setDisplayOptions(
                            options);
                    }
                });
        }

        TimelineActions::TimelineActions() :
            _p(new Private)
        {
        }

        TimelineActions::~TimelineActions() {}

        std::shared_ptr<TimelineActions> TimelineActions::create(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<TimelineActions>(new TimelineActions);
            out->_init(mainWindow, app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >&
        TimelineActions::getActions() const
        {
            return _p->actions;
        }
    } // namespace play_app
} // namespace tl

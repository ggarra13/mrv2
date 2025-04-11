// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/TimelineMenu.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <tlTimelineUI/TimelineWidget.h>

namespace tl
{
    namespace play_app
    {
        struct TimelineMenu::Private
        {
            std::weak_ptr<MainWindow> mainWindow;

            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::map<int, std::shared_ptr<ui::Action> > thumbnailsSizeItems;
            std::map<std::string, std::shared_ptr<ui::Menu> > menus;

            std::shared_ptr<observer::ValueObserver<bool> > editableObserver;
            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<observer::ValueObserver<bool> >
                scrollToCurrentFrameObserver;
            std::shared_ptr<observer::ValueObserver<bool> > stopOnScrubObserver;
            std::shared_ptr<observer::ValueObserver<timelineui::ItemOptions> >
                itemOptionsObserver;
            std::shared_ptr<
                observer::ValueObserver<timelineui::DisplayOptions> >
                displayOptionsObserver;
        };

        void TimelineMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.mainWindow = mainWindow;

            p.actions = actions;
            addItem(p.actions["Input"]);
            addDivider();
            addItem(p.actions["Editable"]);
            addItem(p.actions["EditAssociatedClips"]);
            addDivider();
            addItem(p.actions["FrameView"]);
            addItem(p.actions["ScrollToCurrentFrame"]);
            addItem(p.actions["StopOnScrub"]);
            addDivider();
            addItem(p.actions["FirstTrack"]);
            addItem(p.actions["TrackInfo"]);
            addItem(p.actions["ClipInfo"]);
            addItem(p.actions["Thumbnails"]);
            p.menus["ThumbnailSize"] = addSubMenu("Thumbnails Size");
            p.menus["ThumbnailSize"]->addItem(p.actions["Thumbnails100"]);
            p.menus["ThumbnailSize"]->addItem(p.actions["Thumbnails200"]);
            p.menus["ThumbnailSize"]->addItem(p.actions["Thumbnails300"]);
            addItem(p.actions["Transitions"]);
            addItem(p.actions["Markers"]);

            p.thumbnailsSizeItems[100] = p.actions["Thumbnails100"];
            p.thumbnailsSizeItems[200] = p.actions["Thumbnails200"];
            p.thumbnailsSizeItems[300] = p.actions["Thumbnails300"];

            _thumbnailsSizeUpdate();

            p.editableObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeEditable(),
                [this](bool value)
                { setItemChecked(_p->actions["Editable"], value); });

            p.frameViewObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeFrameView(),
                [this](bool value)
                { setItemChecked(_p->actions["FrameView"], value); });

            p.scrollToCurrentFrameObserver =
                observer::ValueObserver<bool>::create(
                    mainWindow->getTimelineWidget()
                        ->observeScrollToCurrentFrame(),
                    [this](bool value) {
                        setItemChecked(
                            _p->actions["ScrollToCurrentFrame"], value);
                    });

            p.stopOnScrubObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeStopOnScrub(),
                [this](bool value)
                { setItemChecked(_p->actions["StopOnScrub"], value); });

            p.itemOptionsObserver =
                observer::ValueObserver<timelineui::ItemOptions>::create(
                    mainWindow->getTimelineWidget()->observeItemOptions(),
                    [this](const timelineui::ItemOptions& value)
                    {
                        setItemChecked(
                            _p->actions["EditAssociatedClips"],
                            value.editAssociatedClips);
                        setItemChecked(
                            _p->actions["Input"], value.inputEnabled);
                    });

            p.displayOptionsObserver =
                observer::ValueObserver<timelineui::DisplayOptions>::create(
                    mainWindow->getTimelineWidget()->observeDisplayOptions(),
                    [this](const timelineui::DisplayOptions& value)
                    {
                        setItemChecked(
                            _p->actions["FirstTrack"], !value.tracks.empty());
                        setItemChecked(
                            _p->actions["TrackInfo"], value.trackInfo);
                        setItemChecked(_p->actions["ClipInfo"], value.clipInfo);
                        setItemChecked(
                            _p->actions["Thumbnails"], value.thumbnails);
                        _thumbnailsSizeUpdate();
                        setItemChecked(
                            _p->actions["Transitions"], value.transitions);
                        setItemChecked(_p->actions["Markers"], value.markers);
                    });
        }

        TimelineMenu::TimelineMenu() :
            _p(new Private)
        {
        }

        TimelineMenu::~TimelineMenu() {}

        std::shared_ptr<TimelineMenu> TimelineMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineMenu>(new TimelineMenu);
            out->_init(actions, mainWindow, app, context, parent);
            return out;
        }

        void TimelineMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            for (const auto& menu : p.menus)
            {
                menu.second->close();
            }
        }

        void TimelineMenu::_thumbnailsSizeUpdate()
        {
            TLRENDER_P();
            if (auto mainWindow = p.mainWindow.lock())
            {
                const auto options =
                    mainWindow->getTimelineWidget()->getDisplayOptions();
                auto i = p.thumbnailsSizeItems.find(options.thumbnailHeight);
                if (i == p.thumbnailsSizeItems.end())
                {
                    i = p.thumbnailsSizeItems.begin();
                }
                for (auto item : p.thumbnailsSizeItems)
                {
                    const bool checked = item == *i;
                    p.menus["ThumbnailSize"]->setItemChecked(
                        item.second, checked);
                }
            }
        }
    } // namespace play_app
} // namespace tl

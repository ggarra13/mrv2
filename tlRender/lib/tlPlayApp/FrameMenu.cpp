// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FrameMenu.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

namespace tl
{
    namespace play_app
    {
        struct FrameMenu::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void FrameMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.actions = actions;

            addItem(p.actions["Start"]);
            addItem(p.actions["End"]);
            addDivider();
            addItem(p.actions["Prev"]);
            addItem(p.actions["PrevX10"]);
            addItem(p.actions["PrevX100"]);
            addDivider();
            addItem(p.actions["Next"]);
            addItem(p.actions["NextX10"]);
            addItem(p.actions["NextX100"]);
            addDivider();
            addItem(p.actions["FocusCurrent"]);
        }

        FrameMenu::FrameMenu() :
            _p(new Private)
        {
        }

        FrameMenu::~FrameMenu() {}

        std::shared_ptr<FrameMenu> FrameMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FrameMenu>(new FrameMenu);
            out->_init(actions, app, context, parent);
            return out;
        }
    } // namespace play_app
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Menu.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace play_app
    {
        class App;
        class MainWindow;

        //! Frame menu.
        class FrameMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(FrameMenu);

        protected:
            void _init(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            FrameMenu();

        public:
            ~FrameMenu();

            static std::shared_ptr<FrameMenu> create(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Menu.h>

namespace tl
{
    namespace play_app
    {
        class App;
        class MainWindow;

        //! Window menu.
        class WindowMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(WindowMenu);

        protected:
            void _init(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<MainWindow>&, const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            WindowMenu();

        public:
            ~WindowMenu();

            static std::shared_ptr<WindowMenu> create(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<MainWindow>&, const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void close() override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl

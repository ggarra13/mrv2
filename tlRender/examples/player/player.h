// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUIApp/App.h>
#include <tlUIApp/Window.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace examples
    {

        namespace player
        {
            //! Example player application.
            class App : public ui_app::App
            {
                TLRENDER_NON_COPYABLE(App);

            protected:
                void _init(
                    const std::vector<std::string>&,
                    const std::shared_ptr<system::Context>&);

                App();

            public:
                ~App();

                static std::shared_ptr<App> create(
                    const std::vector<std::string>&,
                    const std::shared_ptr<system::Context>&);

            protected:
                void _tick() override;

            private:
                std::string _fileName;
                std::shared_ptr<timeline::Player> _player;
                std::shared_ptr<ui_app::Window> _window;
            };
        } // namespace player
    } // namespace examples
} // namespace tl

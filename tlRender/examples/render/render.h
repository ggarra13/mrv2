// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlBaseApp/BaseApp.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>

namespace tl
{
    namespace gl
    {
        class GLFWWindow;
    }

    namespace examples
    {
        //! Example rendering application.
        namespace render
        {
            //! Application options.
            struct Options
            {
                std::string compareFileName;
                math::Size2i windowSize = math::Size2i(1920, 1080);
                bool fullscreen = false;
                bool hud = true;
                timeline::Playback playback = timeline::Playback::Forward;
                otime::RationalTime seek = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                timeline::OCIOOptions ocioOptions;
                timeline::LUTOptions lutOptions;
            };

            //! Application.
            class App : public app::BaseApp
            {
                TLRENDER_NON_COPYABLE(App);

            protected:
                void _init(
                    const std::vector<std::string>&,
                    const std::shared_ptr<system::Context>&);

                App();

            public:
                ~App();

                //! Create a new application.
                static std::shared_ptr<App> create(
                    const std::vector<std::string>&,
                    const std::shared_ptr<system::Context>&);

                //! Run the application.
                int run();

                //! Exit the application.
                void exit();

            private:
                void _keyCallback(int, int, int, int);

                void _printShortcutsHelp();

                void _tick();

                void _draw();
                void _drawViewport(
                    const math::Box2i&, uint16_t fontSize,
                    const timeline::CompareOptions&, float rotation);

                void _hudCallback(bool);

                void _playbackCallback(timeline::Playback);

                std::string _input;
                Options _options;

                std::shared_ptr<timeline::Player> _player;

                std::shared_ptr<gl::GLFWWindow> _window;
                math::Size2i _frameBufferSize;
                math::Vector2f _contentScale = math::Vector2f(1.F, 1.F);
                timeline::CompareOptions _compareOptions;
                float _rotation = 0.F;
                bool _hud = false;
                std::shared_ptr<timeline::IRender> _render;
                bool _renderDirty = true;
                std::vector<timeline::VideoData> _videoData;
                std::shared_ptr<observer::ListObserver<timeline::VideoData> >
                    _videoDataObserver;
                std::chrono::steady_clock::time_point _startTime;

                bool _running = true;
            };
        } // namespace render
    } // namespace examples
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "render.h"

#include <tlTimelineGL/Render.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWWindow.h>

#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <array>
#include <cmath>

namespace tl
{
    namespace examples
    {
        namespace render
        {
            void App::_init(
                const std::vector<std::string>& argv,
                const std::shared_ptr<system::Context>& context)
            {
                BaseApp::_init(
                    argv, context, "render", "Example rendering application.",
                    {app::CmdLineValueArg<std::string>::create(
                        _input, "input", "The input timeline.")},
                    {app::CmdLineValueOption<std::string>::create(
                         _options.compareFileName, {"-compare", "-b"},
                         "A/B comparison \"B\" file name."),
                     app::CmdLineValueOption<math::Size2i>::create(
                         _options.windowSize, {"-windowSize", "-ws"},
                         "Window size.",
                         string::Format("{0}x{1}")
                             .arg(_options.windowSize.w)
                             .arg(_options.windowSize.h)),
                     app::CmdLineFlagOption::create(
                         _options.fullscreen, {"-fullscreen", "-fs"},
                         "Enable full screen mode."),
                     app::CmdLineValueOption<bool>::create(
                         _options.hud, {"-hud"},
                         "Enable the HUD (heads up display).",
                         string::Format("{0}").arg(_options.hud), "0, 1"),
                     app::CmdLineValueOption<timeline::Playback>::create(
                         _options.playback, {"-playback", "-p"},
                         "Playback mode.",
                         string::Format("{0}").arg(_options.playback),
                         string::join(timeline::getPlaybackLabels(), ", ")),
                     app::CmdLineValueOption<otime::RationalTime>::create(
                         _options.seek, {"-seek"}, "Seek to the given time."),
                     app::CmdLineValueOption<otime::TimeRange>::create(
                         _options.inOutRange, {"-inOutRange"},
                         "Set the in/out points range."),
                     app::CmdLineValueOption<std::string>::create(
                         _options.ocioOptions.fileName, {"-ocio"},
                         "OpenColorIO configuration file name (e.g., "
                         "config.ocio)."),
                     app::CmdLineValueOption<std::string>::create(
                         _options.ocioOptions.input, {"-ocioInput"},
                         "OpenColorIO input name."),
                     app::CmdLineValueOption<std::string>::create(
                         _options.ocioOptions.display, {"-ocioDisplay"},
                         "OpenColorIO display name."),
                     app::CmdLineValueOption<std::string>::create(
                         _options.ocioOptions.view, {"-ocioView"},
                         "OpenColorIO view name."),
                     app::CmdLineValueOption<std::string>::create(
                         _options.ocioOptions.look, {"-ocioLook"},
                         "OpenColorIO look name."),
                     app::CmdLineValueOption<std::string>::create(
                         _options.lutOptions.fileName, {"-lut"},
                         "LUT file name."),
                     app::CmdLineValueOption<timeline::LUTOrder>::create(
                         _options.lutOptions.order, {"-lutOrder"},
                         "LUT operation order.",
                         string::Format("{0}").arg(_options.lutOptions.order),
                         string::join(timeline::getLUTOrderLabels(), ", "))});
            }

            App::App() {}

            App::~App() {}

            std::shared_ptr<App> App::create(
                const std::vector<std::string>& argv,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(argv, context);
                return out;
            }

            int App::run()
            {
                if (0 == _exit)
                {
                    // Read the timelines.
                    auto timeline =
                        timeline::Timeline::create(_input, _context);
                    _player = timeline::Player::create(timeline, _context);
                    std::vector<std::shared_ptr<timeline::Timeline> > compare;
                    if (!_options.compareFileName.empty())
                    {
                        compare.push_back(timeline::Timeline::create(
                            _options.compareFileName, _context));
                    }
                    _player->setCompare(compare);
                    _videoDataObserver =
                        observer::ListObserver<timeline::VideoData>::create(
                            _player->observeCurrentVideo(),
                            [this](
                                const std::vector<timeline::VideoData>& value)
                            {
                                _videoData = value;
                                _renderDirty = true;
                            });

                    // Create the window.
                    _window = gl::GLFWWindow::create(
                        "render", _options.windowSize, _context);
                    _frameBufferSize = _window->getFrameBufferSize();
                    _contentScale = _window->getContentScale();
                    _window->setFullScreen(_options.fullscreen);
                    _window->setFrameBufferSizeCallback(
                        [this](const math::Size2i& value)
                        {
                            _frameBufferSize = value;
                            _renderDirty = true;
                        });
                    _window->setContentScaleCallback(
                        [this](const math::Vector2f& value)
                        {
                            _contentScale = value;
                            _renderDirty = true;
                        });
                    _window->setKeyCallback(
                        [this](int key, int scanCode, int action, int mods)
                        { _keyCallback(key, scanCode, action, mods); });

                    // Create the renderer.
                    _render = timeline_gl::Render::create(_context);

                    // Print the shortcuts help.
                    _printShortcutsHelp();

                    // Start the main loop.
                    _hud = _options.hud;
                    if (time::isValid(_options.inOutRange))
                    {
                        _player->setInOutRange(_options.inOutRange);
                        _player->seek(_options.inOutRange.start_time());
                    }
                    if (time::isValid(_options.seek))
                    {
                        _player->seek(_options.seek);
                    }
                    _player->setPlayback(_options.playback);
                    _startTime = std::chrono::steady_clock::now();
                    while (_running && !_window->shouldClose())
                    {
                        glfwPollEvents();
                        _tick();
                    }
                }
                return _exit;
            }

            void App::exit()
            {
                _running = false;
            }

            void App::_keyCallback(int key, int scanCode, int action, int mods)
            {
                if (GLFW_RELEASE == action || GLFW_REPEAT == action)
                {
                    switch (key)
                    {
                    case GLFW_KEY_ESCAPE:
                        exit();
                        break;
                    case GLFW_KEY_U:
                        _window->setFullScreen(!_window->isFullScreen());
                        break;
                    case GLFW_KEY_H:
                        _hudCallback(!_hud);
                        break;
                    case GLFW_KEY_SPACE:
                        _playbackCallback(
                            timeline::Playback::Stop ==
                                    _player->observePlayback()->get()
                                ? timeline::Playback::Forward
                                : timeline::Playback::Stop);
                        break;
                    case GLFW_KEY_HOME:
                        _player->start();
                        break;
                    case GLFW_KEY_END:
                        _player->end();
                        break;
                    case GLFW_KEY_LEFT:
                        _player->framePrev();
                        break;
                    case GLFW_KEY_RIGHT:
                        _player->frameNext();
                        break;
                    }
                }
            }

            void App::_printShortcutsHelp()
            {
                _print("\n"
                       "Keyboard shortcuts:\n"
                       "\n"
                       "    Escape - Exit\n"
                       "    U      - Fullscreen mode\n"
                       "    H      - HUD enabled\n"
                       "    Space  - Start/stop playback\n"
                       "    Home   - Go to the start time\n"
                       "    End    - Go to the end time\n"
                       "    Left   - Go to the previous frame\n"
                       "    Right  - Go to the next frame\n");
            }

            void App::_tick()
            {
                const auto t0 = std::chrono::steady_clock::now();

                // Update.
                _context->tick();
                _player->tick();

                // Render the video.
                if (_renderDirty)
                {
                    _render->begin(_frameBufferSize);
                    _render->setOCIOOptions(_options.ocioOptions);
                    _render->setLUTOptions(_options.lutOptions);
                    _draw();
                    _render->end();
                    _window->swap();
                    _renderDirty = false;
                }

                // Update the animation.
                const auto t1 = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = t1 - _startTime;
                const float v = (sinf(diff.count()) + 1.F) / 2.F;
                _compareOptions.wipeCenter.x = v;
                _compareOptions.overlay = v;
                _rotation = diff.count() * 2.F;

                // Sleep.
                time::sleep(std::chrono::milliseconds(5), t0, t1);
            }

            void App::_draw()
            {
                const int fontSize = math::clamp(
                    ceilf(14 * _contentScale.y), 0.F,
                    static_cast<float>(std::numeric_limits<uint16_t>::max()));
                const int viewportSpacing = fontSize / 2;
                const math::Vector2i viewportSize(
                    (_frameBufferSize.w - viewportSpacing * 2) / 3,
                    (_frameBufferSize.h - viewportSpacing * 2) / 3);

                _compareOptions.mode = timeline::CompareMode::A;
                _drawViewport(
                    math::Box2i(0, 0, viewportSize.x, viewportSize.y), fontSize,
                    _compareOptions, 0.F);
                _compareOptions.mode = timeline::CompareMode::A;
                _drawViewport(
                    math::Box2i(
                        viewportSize.x + viewportSpacing, 0, viewportSize.x,
                        viewportSize.y),
                    fontSize, _compareOptions, _rotation);
                _compareOptions.mode = timeline::CompareMode::B;
                _drawViewport(
                    math::Box2i(
                        viewportSize.x * 2 + viewportSpacing * 2, 0,
                        viewportSize.x, viewportSize.y),
                    fontSize, _compareOptions, _rotation);

                _compareOptions.mode = timeline::CompareMode::Wipe;
                _drawViewport(
                    math::Box2i(
                        0, viewportSize.y + viewportSpacing, viewportSize.x,
                        viewportSize.y),
                    fontSize, _compareOptions, _rotation);
                _compareOptions.mode = timeline::CompareMode::Overlay;
                _drawViewport(
                    math::Box2i(
                        viewportSize.x + viewportSpacing,
                        viewportSize.y + viewportSpacing, viewportSize.x,
                        viewportSize.y),
                    fontSize, _compareOptions, _rotation);
                _compareOptions.mode = timeline::CompareMode::Difference;
                _drawViewport(
                    math::Box2i(
                        viewportSize.x * 2 + viewportSpacing * 2,
                        viewportSize.y + viewportSpacing, viewportSize.x,
                        viewportSize.y),
                    fontSize, _compareOptions, _rotation);

                _compareOptions.mode = timeline::CompareMode::Horizontal;
                _drawViewport(
                    math::Box2i(
                        0, viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x, viewportSize.y),
                    fontSize, _compareOptions, _rotation);
                _compareOptions.mode = timeline::CompareMode::Vertical;
                _drawViewport(
                    math::Box2i(
                        viewportSize.x + viewportSpacing,
                        viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x, viewportSize.y),
                    fontSize, _compareOptions, _rotation);
                _compareOptions.mode = timeline::CompareMode::Tile;
                _drawViewport(
                    math::Box2i(
                        viewportSize.x * 2 + viewportSpacing * 2,
                        viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x, viewportSize.y),
                    fontSize, _compareOptions, _rotation);
            }

            void App::_drawViewport(
                const math::Box2i& box, uint16_t fontSize,
                const timeline::CompareOptions& compareOptions, float rotation)
            {
                const math::Size2i viewportSize = box.getSize();
                const float viewportAspect = viewportSize.getAspect();
                const math::Size2i renderSize =
                    timeline::getRenderSize(compareOptions.mode, _videoData);
                const float renderSizeAspect = renderSize.getAspect();
                image::Size transformSize;
                math::Vector2f transformOffset;
                if (renderSizeAspect > 1.F)
                {
                    transformSize.w = renderSize.w;
                    transformSize.h = renderSize.w / viewportAspect;
                    transformOffset.x = renderSize.w / 2.F;
                    transformOffset.y = renderSize.w / viewportAspect / 2.F;
                }
                else
                {
                    transformSize.w = renderSize.h * viewportAspect;
                    transformSize.h = renderSize.h;
                    transformOffset.x = renderSize.h * viewportAspect / 2.F;
                    transformOffset.y = renderSize.h / 2.F;
                }

                _render->setClipRectEnabled(true);
                _render->setViewport(box);
                _render->setClipRect(box);
                _render->clearViewport(image::Color4f(0.F, 0.F, 0.F));

                _render->setTransform(
                    math::ortho(
                        0.F, static_cast<float>(transformSize.w),
                        static_cast<float>(transformSize.h), 0.F, -1.F, 1.F) *
                    math::translate(math::Vector3f(
                        transformOffset.x, transformOffset.y, 0.F)) *
                    math::rotateZ(rotation) *
                    math::translate(math::Vector3f(
                        -renderSize.w / 2, -renderSize.h / 2, 0.F)));
                _render->drawVideo(
                    _videoData,
                    timeline::getBoxes(compareOptions.mode, _videoData), {}, {},
                    compareOptions);

                if (_hud)
                {
                    _render->setTransform(math::ortho(
                        0.F, static_cast<float>(viewportSize.w),
                        static_cast<float>(viewportSize.h), 0.F, -1.F, 1.F));

                    image::FontInfo fontInfo;
                    fontInfo.size = fontSize;
                    auto fontSystem = _context->getSystem<image::FontSystem>();
                    auto fontMetrics = fontSystem->getMetrics(fontInfo);
                    std::string text = timeline::getLabel(compareOptions.mode);
                    math::Size2i textSize = fontSystem->getSize(text, fontInfo);
                    _render->drawRect(
                        math::Box2i(
                            0, 0, viewportSize.w, fontMetrics.lineHeight),
                        image::Color4f(0.F, 0.F, 0.F, .7F));
                    _render->drawText(
                        fontSystem->getGlyphs(text, fontInfo),
                        math::Vector2i(fontSize / 5, fontMetrics.ascender),
                        image::Color4f(1.F, 1.F, 1.F));
                }

                _render->setClipRectEnabled(false);
            }

            void App::_hudCallback(bool value)
            {
                _hud = value;
                _renderDirty = true;
                _log(string::Format("HUD: {0}").arg(_hud));
            }

            void App::_playbackCallback(timeline::Playback value)
            {
                _player->setPlayback(value);
                _log(string::Format("Playback: {0}")
                         .arg(_player->observePlayback()->get()));
            }
        } // namespace render
    } // namespace examples
} // namespace tl

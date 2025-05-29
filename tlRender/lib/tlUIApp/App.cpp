// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUIApp/App.h>

#include <tlUIApp/Window.h>

#include <tlUI/IClipboard.h>

#include <tlGL/GLFWWindow.h>

#include <tlCore/StringFormat.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace tl
{
    namespace ui_app
    {
        namespace
        {
            const std::chrono::milliseconds tickTimeout(5);

            class Clipboard : public ui::IClipboard
            {
                TLRENDER_NON_COPYABLE(Clipboard);

            public:
                Clipboard() {}

            public:
                virtual ~Clipboard() {}

                static std::shared_ptr<Clipboard> create(
                    GLFWwindow* glfwWindow,
                    const std::shared_ptr<system::Context>& context)
                {
                    auto out = std::shared_ptr<Clipboard>(new Clipboard);
                    out->_init(context);
                    return out;
                }

                void setWindow(GLFWwindow* glfwWindow)
                {
                    _glfwWindow = glfwWindow;
                }

                std::string getText() const override
                {
                    return _glfwWindow ? glfwGetClipboardString(_glfwWindow)
                                       : std::string();
                }

                void setText(const std::string& value) override
                {
                    if (_glfwWindow)
                    {
                        glfwSetClipboardString(_glfwWindow, value.c_str());
                    }
                }

            private:
                GLFWwindow* _glfwWindow = nullptr;
            };
        } // namespace

        struct App::Private
        {
            std::shared_ptr<ui::Style> style;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<image::FontSystem> fontSystem;
            std::shared_ptr<Clipboard> clipboard;
            std::vector<std::shared_ptr<Window> > windows;
            std::shared_ptr<Window> clipboardWindow;
            std::vector<std::shared_ptr<Window> > windowsToRemove;
            bool running = true;

            std::map<
                std::shared_ptr<Window>,
                std::shared_ptr<observer::ValueObserver<bool> > >
                closeObservers;
        };

        void App::_init(
            const std::vector<std::string>& argv,
            const std::shared_ptr<system::Context>& context,
            const std::string& cmdLineName, const std::string& cmdLineSummary,
            const std::vector<std::shared_ptr<app::ICmdLineArg> >& cmdLineArgs,
            const std::vector<std::shared_ptr<app::ICmdLineOption> >&
                cmdLineOptions)
        {
            TLRENDER_P();
            if (const GLFWvidmode* monitorMode =
                    glfwGetVideoMode(glfwGetPrimaryMonitor()))
            {
                _uiOptions.windowSize.w = monitorMode->width * .7F;
                _uiOptions.windowSize.h = monitorMode->height * .7F;
            }
            std::vector<std::shared_ptr<app::ICmdLineOption> > cmdLineOptions2 =
                cmdLineOptions;
            cmdLineOptions2.push_back(
                app::CmdLineValueOption<math::Size2i>::create(
                    _uiOptions.windowSize, {"-windowSize", "-ws"},
                    "Window size.",
                    string::Format("{0}x{1}")
                        .arg(_uiOptions.windowSize.w)
                        .arg(_uiOptions.windowSize.h)));
            cmdLineOptions2.push_back(app::CmdLineFlagOption::create(
                _uiOptions.fullscreen, {"-fullscreen", "-fs"},
                "Enable full screen mode."));
            app::BaseApp::_init(
                argv, context, cmdLineName, cmdLineSummary, cmdLineArgs,
                cmdLineOptions2);
            if (_exit != 0)
            {
                return;
            }

            p.style = ui::Style::create(_context);
            p.iconLibrary = ui::IconLibrary::create(_context);
            p.fontSystem = context->getSystem<image::FontSystem>();
            p.clipboard = Clipboard::create(nullptr, _context);
        }

        App::App() :
            _p(new Private)
        {
        }

        App::~App() {}

        int App::run()
        {
            TLRENDER_P();
            while (0 == _exit && p.running && !p.windows.empty())
            {
                const auto t0 = std::chrono::steady_clock::now();

                glfwPollEvents();

                // Tick the various objects.
                _context->tick();
                _tick();
                ui::TickEvent tickEvent(p.style, p.iconLibrary, p.fontSystem);
                for (const auto& window : p.windows)
                {
                    _tickRecursive(
                        window, window->isVisible(false),
                        window->isEnabled(false), tickEvent);
                }

                // Remove closed windows
                for (const auto& window : p.windowsToRemove)
                {
                    _removeWindow(window);
                }
                p.windowsToRemove.clear();

                // Sleep for a bit.
                const auto t1 = std::chrono::steady_clock::now();
                time::sleep(tickTimeout, t0, t1);
            }
            return _exit;
        }

        void App::exit(int r)
        {
            _exit = r;
            _p->running = false;
        }

        const std::shared_ptr<ui::Style> App::getStyle() const
        {
            return _p->style;
        }

        int App::getScreenCount() const
        {
            int glfwMonitorsCount = 0;
            glfwGetMonitors(&glfwMonitorsCount);
            return glfwMonitorsCount;
        }

        void App::addWindow(const std::shared_ptr<Window>& window)
        {
            TLRENDER_P();
            window->setClipboard(p.clipboard);
            p.windows.push_back(window);

            p.clipboardWindow = window;
            p.clipboard->setWindow(window->getGLFWWindow()->getGLFW());

            p.closeObservers[window] = observer::ValueObserver<bool>::create(
                window->observeClose(),
                [this, window](bool value)
                {
                    if (value)
                    {
                        _p->windowsToRemove.push_back(window);
                    }
                });
        }

        void App::removeWindow(const std::shared_ptr<Window>& window)
        {
            TLRENDER_P();
            p.windowsToRemove.push_back(window);
        }

        void App::_tick() {}

        void App::_tickRecursive(
            const std::shared_ptr<ui::IWidget>& widget, bool visible,
            bool enabled, const ui::TickEvent& event)
        {
            TLRENDER_P();
            const bool parentsVisible = visible && widget->isVisible(false);
            const bool parentsEnabled = enabled && widget->isEnabled(false);
            for (const auto& child : widget->getChildren())
            {
                _tickRecursive(child, parentsVisible, parentsEnabled, event);
            }
            widget->tickEvent(visible, enabled, event);
        }

        void App::_removeWindow(const std::shared_ptr<Window>& window)
        {
            TLRENDER_P();
            const auto i =
                std::find(p.windows.begin(), p.windows.end(), window);
            if (i != p.windows.end())
            {
                if (*i == p.clipboardWindow)
                {
                    p.clipboard->setWindow(nullptr);
                    p.clipboardWindow.reset();
                }
                p.windows.erase(i);
            }
            const auto j = p.closeObservers.find(window);
            if (j != p.closeObservers.end())
            {
                p.closeObservers.erase(j);
            }
            if (!p.clipboardWindow && !p.windows.empty())
            {
                p.clipboardWindow = p.windows.front();
                p.clipboard->setWindow(
                    p.clipboardWindow->getGLFWWindow()->getGLFW());
            }
        }
    } // namespace ui_app
} // namespace tl

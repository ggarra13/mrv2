// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGL/GLFWWindow.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWSystem.h>
#include <tlGL/Init.h>

#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Vector.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

namespace tl
{
    namespace gl
    {
        namespace
        {
#if defined(TLRENDER_API_GL_4_1_Debug)
            void APIENTRY glDebugOutput(
                GLenum source, GLenum type, GLuint id, GLenum severity,
                GLsizei length, const GLchar* message, const void* userParam)
            {
                //! \todo Send output to the log instead of cerr?
                switch (severity)
                {
                case GL_DEBUG_SEVERITY_HIGH:
                    std::cerr << "GL HIGH: " << message << std::endl;
                    break;
                case GL_DEBUG_SEVERITY_MEDIUM:
                    std::cerr << "GL MEDIUM: " << message << std::endl;
                    break;
                case GL_DEBUG_SEVERITY_LOW:
                    std::cerr << "GL LOW: " << message << std::endl;
                    break;
                    // case GL_DEBUG_SEVERITY_NOTIFICATION:
                    //     std::cerr << "GL NOTIFICATION: " << message <<
                    //     std::endl; break;
                default:
                    break;
                }
            }
#endif // TLRENDER_API_GL_4_1_Debug
        } // namespace

        void windowHint(int flag, int value)
        {
            if (!gl::isWayland())
            {
                glfwWindowHint(flag, value);
            }
            else
            {
                // \@bug: NVidia drivers currently return EGL error if
                //        GLFW_DOUBLEBUFFER is GLFW_FALSE
                if (flag == GLFW_DOUBLEBUFFER)
                {
                    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
                }
            }
        }

        struct GLFWWindow::Private
        {
            std::weak_ptr<system::Context> context;
            GLFWwindow* glfwWindow = nullptr;
            bool gladInit = true;
            math::Size2i size;
            math::Vector2i pos;
            math::Size2i frameBufferSize;
            math::Vector2f contentScale;
            bool fullScreen = false;
            math::Size2i restoreSize;
            bool floatOnTop = false;

            std::function<void(const math::Size2i)> sizeCallback;
            std::function<void(const math::Size2i)> frameBufferSizeCallback;
            std::function<void(const math::Vector2f)> contentScaleCallback;
            std::function<void(void)> refreshCallback;
            std::function<void(bool)> cursorEnterCallback;
            std::function<void(const math::Vector2f&)> cursorPosCallback;
            std::function<void(int, int, int)> buttonCallback;
            std::function<void(const math::Vector2f&)> scrollCallback;
            std::function<void(int, int, int, int)> keyCallback;
            std::function<void(unsigned int)> charCallback;
            std::function<void(int, const char**)> dropCallback;
        };

        void GLFWWindow::_init(
            const std::string& name, const math::Size2i& size,
            const std::shared_ptr<system::Context>& context, int options,
            const std::shared_ptr<GLFWWindow>& share)
        {
            TLRENDER_P();

            p.context = context;

#if defined(TLRENDER_API_GL_4_1)
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#elif defined(TLRENDER_API_GLES_2)
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
#endif // TLRENDER_API_GL_4_1

            glfwWindowHint(
                GLFW_VISIBLE,
                options & static_cast<int>(GLFWWindowOptions::Visible));
            gl::windowHint(
                GLFW_DOUBLEBUFFER,
                options & static_cast<int>(GLFWWindowOptions::DoubleBuffer));
#if defined(TLRENDER_API_GL_4_1_Debug)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // TLRENDER_API_GL_4_1_Debug
            context->log(
                "tl::gl::GLFWWindow",
                string::Format("Create window: {0}").arg(size));
            p.glfwWindow = glfwCreateWindow(
                size.w, size.h, name.c_str(), nullptr, nullptr);
            if (!p.glfwWindow)
            {
                throw std::runtime_error("Cannot create window");
            }

            glfwGetWindowSize(_p->glfwWindow, &p.size.w, &p.size.h);
            glfwGetFramebufferSize(
                _p->glfwWindow, &p.frameBufferSize.w, &p.frameBufferSize.h);
            glfwGetWindowContentScale(
                _p->glfwWindow, &p.contentScale.x, &p.contentScale.y);
            context->log(
                "tl::gl::GLFWWindow",
                string::Format("Window size: {0}").arg(p.size));
            context->log(
                "tl::gl::GLFWWindow", string::Format("Frame buffer size: {0}")
                                          .arg(p.frameBufferSize));
            context->log(
                "tl::gl::GLFWWindow",
                string::Format("Content scale: {0}").arg(p.contentScale));

            glfwSetWindowUserPointer(p.glfwWindow, this);
            glfwSetWindowSizeCallback(p.glfwWindow, _sizeCallback);
            glfwSetFramebufferSizeCallback(
                p.glfwWindow, _frameBufferSizeCallback);
            glfwSetWindowContentScaleCallback(
                p.glfwWindow, _windowContentScaleCallback);
            glfwSetWindowRefreshCallback(p.glfwWindow, _windowRefreshCallback);
            glfwSetCursorEnterCallback(p.glfwWindow, _cursorEnterCallback);
            glfwSetCursorPosCallback(p.glfwWindow, _cursorPosCallback);
            glfwSetMouseButtonCallback(p.glfwWindow, _mouseButtonCallback);
            glfwSetScrollCallback(p.glfwWindow, _scrollCallback);
            glfwSetKeyCallback(p.glfwWindow, _keyCallback);
            glfwSetCharCallback(p.glfwWindow, _charCallback);
            glfwSetDropCallback(p.glfwWindow, _dropCallback);

            if (options & static_cast<int>(GLFWWindowOptions::MakeCurrent))
            {
                makeCurrent();
            }

            const int glMajor =
                glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
            const int glMinor =
                glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
            const int glRevision =
                glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_REVISION);
            context->log(
                "tl::gl::GLFWWindow",
                string::Format("OpenGL version: {0}.{1}.{2}")
                    .arg(glMajor)
                    .arg(glMinor)
                    .arg(glRevision));
        }

        GLFWWindow::GLFWWindow() :
            _p(new Private)
        {
        }

        GLFWWindow::~GLFWWindow()
        {
            TLRENDER_P();
            if (p.glfwWindow)
            {
                glfwDestroyWindow(p.glfwWindow);
            }
        }

        std::shared_ptr<GLFWWindow> GLFWWindow::create(
            const std::string& name, const math::Size2i& size,
            const std::shared_ptr<system::Context>& context, int options,
            const std::shared_ptr<GLFWWindow>& share)
        {
            auto out = std::shared_ptr<GLFWWindow>(new GLFWWindow);
            out->_init(name, size, context, options, share);
            return out;
        }

        GLFWwindow* GLFWWindow::getGLFW() const
        {
            return _p->glfwWindow;
        }

        const math::Size2i& GLFWWindow::getSize() const
        {
            return _p->size;
        }

        void GLFWWindow::setSize(const math::Size2i& value)
        {
            glfwSetWindowSize(_p->glfwWindow, value.w, value.h);
        }

        const math::Size2i& GLFWWindow::getFrameBufferSize() const
        {
            return _p->frameBufferSize;
        }

        const math::Vector2f& GLFWWindow::getContentScale() const
        {
            return _p->contentScale;
        }

        void GLFWWindow::show()
        {
            glfwShowWindow(_p->glfwWindow);
        }

        void GLFWWindow::hide()
        {
            glfwHideWindow(_p->glfwWindow);
        }

        void GLFWWindow::makeCurrent()
        {
            TLRENDER_P();
            glfwMakeContextCurrent(p.glfwWindow);
            if (p.gladInit)
            {
                p.gladInit = false;
                initGLAD();
#if defined(TLRENDER_API_GL_4_1_Debug)
                GLint flags = 0;
                glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
                if (flags & static_cast<GLint>(GL_CONTEXT_FLAG_DEBUG_BIT))
                {
                    glEnable(GL_DEBUG_OUTPUT);
                    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                    glDebugMessageCallback(glDebugOutput, nullptr);
                    glDebugMessageControl(
                        static_cast<GLenum>(GL_DONT_CARE),
                        static_cast<GLenum>(GL_DONT_CARE),
                        static_cast<GLenum>(GL_DONT_CARE), 0, nullptr, GL_TRUE);
                }
#endif // TLRENDER_API_GL_4_1_Debug
            }
        }

        void GLFWWindow::doneCurrent()
        {
            glfwMakeContextCurrent(nullptr);
        }

        bool GLFWWindow::shouldClose() const
        {
            return glfwWindowShouldClose(_p->glfwWindow);
        }

        int GLFWWindow::getScreen() const
        {
            TLRENDER_P();

            math::Vector2i windowPos;
            math::Size2i windowSize;
            glfwGetWindowPos(p.glfwWindow, &windowPos.x, &windowPos.y);
            glfwGetWindowSize(_p->glfwWindow, &windowSize.w, &windowSize.h);
            const math::Box2i windowBox(windowPos, windowSize);

            struct MonitorData
            {
                int index = 0;
                int width = 0;
                int height = 0;
                int refreshRate = 0;
                math::Box2i intersect;
            };
            std::vector<MonitorData> monitorData;
            int glfwMonitorsCount = 0;
            GLFWmonitor** glfwMonitors = glfwGetMonitors(&glfwMonitorsCount);
            for (int i = 0; i < glfwMonitorsCount; ++i)
            {
                math::Vector2i monitorPos;
                glfwGetMonitorPos(
                    glfwMonitors[i], &monitorPos.x, &monitorPos.y);
                const GLFWvidmode* glfwVidmode =
                    glfwGetVideoMode(glfwMonitors[i]);
                const math::Box2i monitorBox(
                    monitorPos.x, monitorPos.y, glfwVidmode->width,
                    glfwVidmode->height);
                monitorData.push_back(
                    {i, glfwVidmode->width, glfwVidmode->height,
                     glfwVidmode->refreshRate,
                     windowBox.intersect(monitorBox)});
            }
            std::sort(
                monitorData.begin(), monitorData.end(),
                [](const MonitorData& a, const MonitorData& b)
                { return a.intersect.getSize() > b.intersect.getSize(); });

            return !monitorData.empty() ? monitorData.front().index : -1;
        }

        bool GLFWWindow::isFullScreen() const
        {
            return _p->fullScreen;
        }

        void GLFWWindow::setFullScreen(bool value, int screen)
        {
            TLRENDER_P();
            if (value == p.fullScreen)
                return;
            p.fullScreen = value;
            if (p.fullScreen)
            {
                math::Vector2i windowPos;
                math::Size2i windowSize;
                glfwGetWindowPos(p.glfwWindow, &windowPos.x, &windowPos.y);
                glfwGetWindowSize(_p->glfwWindow, &windowSize.w, &windowSize.h);
                const math::Box2i windowBox(windowPos, windowSize);
                p.pos = windowPos;
                p.restoreSize = windowSize;

                int glfwMonitorsCount = 0;
                GLFWmonitor** glfwMonitors =
                    glfwGetMonitors(&glfwMonitorsCount);
                const int monitor = screen < 0 ? getScreen() : screen;
                if (monitor >= 0 && monitor < glfwMonitorsCount)
                {
                    const GLFWvidmode* glfwVidmode =
                        glfwGetVideoMode(glfwMonitors[monitor]);
                    glfwSetWindowMonitor(
                        p.glfwWindow, glfwMonitors[monitor], 0, 0,
                        glfwVidmode->width, glfwVidmode->height,
                        glfwVidmode->refreshRate);
                }
            }
            else
            {
                glfwSetWindowMonitor(
                    p.glfwWindow, nullptr, p.pos.x, p.pos.y, p.restoreSize.w,
                    p.restoreSize.h, 0);
            }
        }

        bool GLFWWindow::isFloatOnTop() const
        {
            return _p->floatOnTop;
        }

        void GLFWWindow::setFloatOnTop(bool value)
        {
            TLRENDER_P();
            if (value == p.floatOnTop)
                return;
            p.floatOnTop = value;
            glfwSetWindowAttrib(
                p.glfwWindow, GLFW_FLOATING,
                p.floatOnTop ? GLFW_TRUE : GLFW_FALSE);
        }

        void GLFWWindow::swap()
        {
            glfwSwapBuffers(_p->glfwWindow);
        }

        void GLFWWindow::setSizeCallback(
            const std::function<void(const math::Size2i)>& value)
        {
            _p->sizeCallback = value;
        }

        void GLFWWindow::setFrameBufferSizeCallback(
            const std::function<void(const math::Size2i)>& value)
        {
            _p->frameBufferSizeCallback = value;
        }

        void GLFWWindow::setContentScaleCallback(
            const std::function<void(const math::Vector2f)>& value)
        {
            _p->contentScaleCallback = value;
        }

        void
        GLFWWindow::setRefreshCallback(const std::function<void(void)>& value)
        {
            _p->refreshCallback = value;
        }

        void GLFWWindow::setCursorEnterCallback(
            const std::function<void(bool)>& value)
        {
            _p->cursorEnterCallback = value;
        }

        void GLFWWindow::setCursorPosCallback(
            const std::function<void(const math::Vector2f&)>& value)
        {
            _p->cursorPosCallback = value;
        }

        void GLFWWindow::setButtonCallback(
            const std::function<void(int, int, int)>& value)
        {
            _p->buttonCallback = value;
        }

        void GLFWWindow::setScrollCallback(
            const std::function<void(const math::Vector2f&)>& value)
        {
            _p->scrollCallback = value;
        }

        void GLFWWindow::setKeyCallback(
            const std::function<void(int, int, int, int)>& value)
        {
            _p->keyCallback = value;
        }

        void GLFWWindow::setCharCallback(
            const std::function<void(unsigned int)>& value)
        {
            _p->charCallback = value;
        }

        void GLFWWindow::setDropCallback(
            const std::function<void(int, const char**)>& value)
        {
            _p->dropCallback = value;
        }

        void GLFWWindow::_sizeCallback(GLFWwindow* glfwWindow, int w, int h)
        {
            GLFWWindow* window = reinterpret_cast<GLFWWindow*>(
                glfwGetWindowUserPointer(glfwWindow));
            window->_p->size.w = w;
            window->_p->size.h = h;
            if (window->_p->sizeCallback)
            {
                window->_p->sizeCallback(window->_p->size);
            }
        }

        void GLFWWindow::_frameBufferSizeCallback(
            GLFWwindow* glfwWindow, int w, int h)
        {
            GLFWWindow* window = reinterpret_cast<GLFWWindow*>(
                glfwGetWindowUserPointer(glfwWindow));
            window->_p->frameBufferSize.w = w;
            window->_p->frameBufferSize.h = h;
            if (window->_p->frameBufferSizeCallback)
            {
                window->_p->frameBufferSizeCallback(
                    window->_p->frameBufferSize);
            }
        }

        void GLFWWindow::_windowContentScaleCallback(
            GLFWwindow* glfwWindow, float x, float y)
        {
            GLFWWindow* window = reinterpret_cast<GLFWWindow*>(
                glfwGetWindowUserPointer(glfwWindow));
            window->_p->contentScale.x = x;
            window->_p->contentScale.y = y;
            if (window->_p->contentScaleCallback)
            {
                window->_p->contentScaleCallback(window->_p->contentScale);
            }
        }

        void GLFWWindow::_windowRefreshCallback(GLFWwindow* glfwWindow)
        {
            GLFWWindow* window = reinterpret_cast<GLFWWindow*>(
                glfwGetWindowUserPointer(glfwWindow));
            if (window->_p->refreshCallback)
            {
                window->_p->refreshCallback();
            }
        }

        void GLFWWindow::_cursorEnterCallback(GLFWwindow* glfwWindow, int value)
        {
            GLFWWindow* window = reinterpret_cast<GLFWWindow*>(
                glfwGetWindowUserPointer(glfwWindow));
            if (window->_p->cursorEnterCallback)
            {
                window->_p->cursorEnterCallback(GLFW_TRUE == value);
            }
        }

        void GLFWWindow::_cursorPosCallback(
            GLFWwindow* glfwWindow, double x, double y)
        {
            GLFWWindow* window = reinterpret_cast<GLFWWindow*>(
                glfwGetWindowUserPointer(glfwWindow));
            if (window->_p->cursorPosCallback)
            {
                window->_p->cursorPosCallback(math::Vector2f(x, y));
            }
        }

        void GLFWWindow::_mouseButtonCallback(
            GLFWwindow* glfwWindow, int button, int action, int mods)
        {
            GLFWWindow* window = reinterpret_cast<GLFWWindow*>(
                glfwGetWindowUserPointer(glfwWindow));
            if (window->_p->buttonCallback)
            {
                window->_p->buttonCallback(button, action, mods);
            }
        }

        void
        GLFWWindow::_scrollCallback(GLFWwindow* glfwWindow, double x, double y)
        {
            GLFWWindow* window = reinterpret_cast<GLFWWindow*>(
                glfwGetWindowUserPointer(glfwWindow));
            if (window->_p->scrollCallback)
            {
                window->_p->scrollCallback(math::Vector2f(x, y));
            }
        }

        void GLFWWindow::_keyCallback(
            GLFWwindow* glfwWindow, int key, int scanCode, int action, int mods)
        {
            GLFWWindow* window = reinterpret_cast<GLFWWindow*>(
                glfwGetWindowUserPointer(glfwWindow));
            if (window->_p->keyCallback)
            {
                window->_p->keyCallback(key, scanCode, action, mods);
            }
        }

        void GLFWWindow::_charCallback(GLFWwindow* glfwWindow, unsigned int c)
        {
            GLFWWindow* window = reinterpret_cast<GLFWWindow*>(
                glfwGetWindowUserPointer(glfwWindow));
            if (window->_p->charCallback)
            {
                window->_p->charCallback(c);
            }
        }

        void GLFWWindow::_dropCallback(
            GLFWwindow* glfwWindow, int count, const char** fileNames)
        {
            GLFWWindow* window = reinterpret_cast<GLFWWindow*>(
                glfwGetWindowUserPointer(glfwWindow));
            if (window->_p->dropCallback)
            {
                window->_p->dropCallback(count, fileNames);
            }
        }
    } // namespace gl
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>
#include <tlCore/Size.h>

struct GLFWwindow;

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace gl
    {
        //! GLFW window options.
        enum class GLFWWindowOptions {
            None = 0,
            Visible = 1,
            DoubleBuffer = 2,
            MakeCurrent = 4
        };

        void windowHint(int flag, int value);

        //! GLFW window wrapper.
        class GLFWWindow : public std::enable_shared_from_this<GLFWWindow>
        {
            TLRENDER_NON_COPYABLE(GLFWWindow);

        protected:
            void _init(
                const std::string& name, const math::Size2i&,
                const std::shared_ptr<system::Context>&, int options,
                const std::shared_ptr<GLFWWindow>& share);

            GLFWWindow();

        public:
            virtual ~GLFWWindow();

            //! Create a new window.
            static std::shared_ptr<GLFWWindow> create(
                const std::string& name, const math::Size2i&,
                const std::shared_ptr<system::Context>&,
                int options =
                    static_cast<int>(GLFWWindowOptions::Visible) |
                    static_cast<int>(GLFWWindowOptions::DoubleBuffer) |
                    static_cast<int>(GLFWWindowOptions::MakeCurrent),
                const std::shared_ptr<GLFWWindow>& share = nullptr);

            //! Get the GLFW window pointer.
            GLFWwindow* getGLFW() const;

            //! Get the window size.
            const math::Size2i& getSize() const;

            //! Set the window size.
            void setSize(const math::Size2i&);

            //! Get the frame buffer size.
            const math::Size2i& getFrameBufferSize() const;

            //! Get the window content scale.
            const math::Vector2f& getContentScale() const;

            //! Show the window.
            void show();

            //! Hide the window.
            void hide();

            //! Make the OpenGL context current.
            void makeCurrent();

            //! Release the OpenGL context.
            void doneCurrent();

            //! Get whether the window should close.
            bool shouldClose() const;

            //! Get which screen the window is on.
            int getScreen() const;

            //! Get whether the window is in full screen mode.
            bool isFullScreen() const;

            //! Set whether the window is in full screen mode.
            void setFullScreen(bool, int screen = -1);

            //! Get whether the window is floating on top.
            bool isFloatOnTop() const;

            //! Set whether the window is floating on top.
            void setFloatOnTop(bool);

            //! Swap the buffers.
            void swap();

            //! Set the window size callback.
            void
            setSizeCallback(const std::function<void(const math::Size2i)>&);

            //! Set the frame buffer size callback.
            void setFrameBufferSizeCallback(
                const std::function<void(const math::Size2i)>&);

            //! Set the content scale callback.
            void setContentScaleCallback(
                const std::function<void(const math::Vector2f)>&);

            //! Set the window refresh callback.
            void setRefreshCallback(const std::function<void(void)>&);

            //! Set the cursor enter callback.
            void setCursorEnterCallback(const std::function<void(bool)>&);

            //! Set the cursor position callback.
            void setCursorPosCallback(
                const std::function<void(const math::Vector2f&)>&);

            //! Set the mouse button callback.
            void setButtonCallback(const std::function<void(int, int, int)>&);

            //! Set the scroll callback.
            void setScrollCallback(
                const std::function<void(const math::Vector2f&)>&);

            //! Set the key callback.
            void setKeyCallback(const std::function<void(int, int, int, int)>&);

            //! Set the character callback.
            void setCharCallback(const std::function<void(unsigned int)>&);

            //! Set the drop callback.
            void setDropCallback(const std::function<void(int, const char**)>&);

        private:
            static void _sizeCallback(GLFWwindow*, int, int);
            static void _frameBufferSizeCallback(GLFWwindow*, int, int);
            static void _windowContentScaleCallback(GLFWwindow*, float, float);
            static void _windowRefreshCallback(GLFWwindow*);
            static void _cursorEnterCallback(GLFWwindow*, int);
            static void _cursorPosCallback(GLFWwindow*, double, double);
            static void _mouseButtonCallback(GLFWwindow*, int, int, int);
            static void _scrollCallback(GLFWwindow*, double, double);
            static void _keyCallback(GLFWwindow*, int, int, int, int);
            static void _charCallback(GLFWwindow*, unsigned int);
            static void _dropCallback(GLFWwindow*, int, const char**);

            TLRENDER_PRIVATE();
        };
    } // namespace gl
} // namespace tl

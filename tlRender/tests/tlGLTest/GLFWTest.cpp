// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGLTest/GLFWTest.h>

#include <tlGL/GLFWWindow.h>
#include <tlGL/GL.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

using namespace tl::gl;

namespace tl
{
    namespace gl_tests
    {
        GLFWTest::GLFWTest(const std::shared_ptr<system::Context>& context) :
            ITest("gl_tests::GLFWTest", context)
        {
        }

        std::shared_ptr<GLFWTest>
        GLFWTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<GLFWTest>(new GLFWTest(context));
        }

        void GLFWTest::run()
        {
            std::shared_ptr<GLFWWindow> window;
            try
            {
                window = GLFWWindow::create(
                    "GLFWTest", math::Size2i(1, 1), _context,
                    static_cast<int>(GLFWWindowOptions::MakeCurrent));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            if (window)
            {
                TLRENDER_ASSERT(window->getGLFW());
                _print(
                    string::Format("Window size: {0}").arg(window->getSize()));
                _print(string::Format("Frame buffer size: {0}")
                           .arg(window->getFrameBufferSize()));
                _print(string::Format("Content scale: {0}")
                           .arg(window->getContentScale()));

                window->setSizeCallback(
                    [this](const math::Size2i& value)
                    { _print(string::Format("Window size: {0}").arg(value)); });
                window->setFrameBufferSizeCallback(
                    [this](const math::Size2i& value) {
                        _print(string::Format("Frame buffer size: {0}")
                                   .arg(value));
                    });
                window->setContentScaleCallback(
                    [this](const math::Vector2f& value) {
                        _print(string::Format("Content scale: {0}").arg(value));
                    });
                window->setRefreshCallback([this]
                                           { _print("Window refresh"); });
                window->setCursorEnterCallback(
                    [this](bool value) {
                        _print(string::Format("Cursor enter: {0}").arg(value));
                    });
                window->setCursorPosCallback(
                    [this](const math::Vector2f& value) {
                        _print(
                            string::Format("Cursor position: {0}").arg(value));
                    });
                window->setButtonCallback(
                    [this](int button, int action, int mods)
                    {
                        _print(string::Format("Button: {0} {1} {2}")
                                   .arg(button)
                                   .arg(action)
                                   .arg(mods));
                    });
                window->setScrollCallback(
                    [this](const math::Vector2f& value)
                    { _print(string::Format("Scroll: {0}").arg(value)); });
                window->setKeyCallback(
                    [this](int key, int scanCode, int action, int mods)
                    {
                        _print(string::Format("Key: {0}")
                                   .arg(key)
                                   .arg(scanCode)
                                   .arg(action)
                                   .arg(mods));
                    });
                window->setCharCallback(
                    [this](unsigned int value)
                    { _print(string::Format("Char: {0}").arg(value)); });
                window->setDropCallback(
                    [this](int size, const char** value)
                    {
                        std::vector<std::string> s;
                        for (int i = 0; i < size; ++i)
                        {
                            s.push_back(value[i]);
                        }
                        _print(string::Format("Drop: {0}")
                                   .arg(string::join(s, ", ")));
                    });

                window->show();
                window->makeCurrent();
                window->setSize(math::Size2i(100, 100));
                window->setSize(math::Size2i(100, 100));
                TLRENDER_ASSERT(!window->shouldClose());

                window->setFullScreen(true);
                window->setFullScreen(true);
                TLRENDER_ASSERT(window->isFullScreen());
                window->setFullScreen(false);

                window->setFloatOnTop(true);
                window->setFloatOnTop(true);
                TLRENDER_ASSERT(window->isFloatOnTop());
                window->setFloatOnTop(false);

                window->swap();
                window->doneCurrent();
            }
        }
    } // namespace gl_tests
} // namespace tl

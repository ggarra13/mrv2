// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUIApp/Window.h>

#include <tlTimelineGL/Render.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWWindow.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Util.h>
#if defined(TLRENDER_API_GLES_2)
#    include <tlGL/Mesh.h>
#    include <tlGL/Shader.h>
#endif // TLRENDER_API_GLES_2

#include <tlCore/StringFormat.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <codecvt>
#include <locale>

namespace tl
{
    namespace ui_app
    {
        namespace
        {
            int fromGLFWModifiers(int value)
            {
                int out = 0;
                if (value & GLFW_MOD_SHIFT)
                {
                    out |= static_cast<int>(ui::KeyModifier::Shift);
                }
                if (value & GLFW_MOD_CONTROL)
                {
                    out |= static_cast<int>(ui::KeyModifier::Control);
                }
                if (value & GLFW_MOD_ALT)
                {
                    out |= static_cast<int>(ui::KeyModifier::Alt);
                }
                if (value & GLFW_MOD_SUPER)
                {
                    out |= static_cast<int>(ui::KeyModifier::Super);
                }
                return out;
            }

            ui::Key fromGLFWKey(int key)
            {
                ui::Key out = ui::Key::Unknown;
                switch (key)
                {
                case GLFW_KEY_SPACE:
                    out = ui::Key::Space;
                    break;
                case GLFW_KEY_APOSTROPHE:
                    out = ui::Key::Apostrophe;
                    break;
                case GLFW_KEY_COMMA:
                    out = ui::Key::Comma;
                    break;
                case GLFW_KEY_MINUS:
                    out = ui::Key::Minus;
                    break;
                case GLFW_KEY_PERIOD:
                    out = ui::Key::Period;
                    break;
                case GLFW_KEY_SLASH:
                    out = ui::Key::Slash;
                    break;
                case GLFW_KEY_0:
                    out = ui::Key::_0;
                    break;
                case GLFW_KEY_1:
                    out = ui::Key::_1;
                    break;
                case GLFW_KEY_2:
                    out = ui::Key::_2;
                    break;
                case GLFW_KEY_3:
                    out = ui::Key::_3;
                    break;
                case GLFW_KEY_4:
                    out = ui::Key::_4;
                    break;
                case GLFW_KEY_5:
                    out = ui::Key::_5;
                    break;
                case GLFW_KEY_6:
                    out = ui::Key::_6;
                    break;
                case GLFW_KEY_7:
                    out = ui::Key::_7;
                    break;
                case GLFW_KEY_8:
                    out = ui::Key::_8;
                    break;
                case GLFW_KEY_9:
                    out = ui::Key::_9;
                    break;
                case GLFW_KEY_SEMICOLON:
                    out = ui::Key::Semicolon;
                    break;
                case GLFW_KEY_EQUAL:
                    out = ui::Key::Equal;
                    break;
                case GLFW_KEY_A:
                    out = ui::Key::A;
                    break;
                case GLFW_KEY_B:
                    out = ui::Key::B;
                    break;
                case GLFW_KEY_C:
                    out = ui::Key::C;
                    break;
                case GLFW_KEY_D:
                    out = ui::Key::D;
                    break;
                case GLFW_KEY_E:
                    out = ui::Key::E;
                    break;
                case GLFW_KEY_F:
                    out = ui::Key::F;
                    break;
                case GLFW_KEY_G:
                    out = ui::Key::G;
                    break;
                case GLFW_KEY_H:
                    out = ui::Key::H;
                    break;
                case GLFW_KEY_I:
                    out = ui::Key::I;
                    break;
                case GLFW_KEY_J:
                    out = ui::Key::J;
                    break;
                case GLFW_KEY_K:
                    out = ui::Key::K;
                    break;
                case GLFW_KEY_L:
                    out = ui::Key::L;
                    break;
                case GLFW_KEY_M:
                    out = ui::Key::M;
                    break;
                case GLFW_KEY_N:
                    out = ui::Key::N;
                    break;
                case GLFW_KEY_O:
                    out = ui::Key::O;
                    break;
                case GLFW_KEY_P:
                    out = ui::Key::P;
                    break;
                case GLFW_KEY_Q:
                    out = ui::Key::Q;
                    break;
                case GLFW_KEY_R:
                    out = ui::Key::R;
                    break;
                case GLFW_KEY_S:
                    out = ui::Key::S;
                    break;
                case GLFW_KEY_T:
                    out = ui::Key::T;
                    break;
                case GLFW_KEY_U:
                    out = ui::Key::U;
                    break;
                case GLFW_KEY_V:
                    out = ui::Key::V;
                    break;
                case GLFW_KEY_W:
                    out = ui::Key::W;
                    break;
                case GLFW_KEY_X:
                    out = ui::Key::X;
                    break;
                case GLFW_KEY_Y:
                    out = ui::Key::Y;
                    break;
                case GLFW_KEY_Z:
                    out = ui::Key::Z;
                    break;
                case GLFW_KEY_LEFT_BRACKET:
                    out = ui::Key::LeftBracket;
                    break;
                case GLFW_KEY_BACKSLASH:
                    out = ui::Key::Backslash;
                    break;
                case GLFW_KEY_RIGHT_BRACKET:
                    out = ui::Key::RightBracket;
                    break;
                case GLFW_KEY_GRAVE_ACCENT:
                    out = ui::Key::GraveAccent;
                    break;
                case GLFW_KEY_ESCAPE:
                    out = ui::Key::Escape;
                    break;
                case GLFW_KEY_ENTER:
                    out = ui::Key::Enter;
                    break;
                case GLFW_KEY_TAB:
                    out = ui::Key::Tab;
                    break;
                case GLFW_KEY_BACKSPACE:
                    out = ui::Key::Backspace;
                    break;
                case GLFW_KEY_INSERT:
                    out = ui::Key::Insert;
                    break;
                case GLFW_KEY_DELETE:
                    out = ui::Key::Delete;
                    break;
                case GLFW_KEY_RIGHT:
                    out = ui::Key::Right;
                    break;
                case GLFW_KEY_LEFT:
                    out = ui::Key::Left;
                    break;
                case GLFW_KEY_DOWN:
                    out = ui::Key::Down;
                    break;
                case GLFW_KEY_UP:
                    out = ui::Key::Up;
                    break;
                case GLFW_KEY_PAGE_UP:
                    out = ui::Key::PageUp;
                    break;
                case GLFW_KEY_PAGE_DOWN:
                    out = ui::Key::PageDown;
                    break;
                case GLFW_KEY_HOME:
                    out = ui::Key::Home;
                    break;
                case GLFW_KEY_END:
                    out = ui::Key::End;
                    break;
                case GLFW_KEY_CAPS_LOCK:
                    out = ui::Key::CapsLock;
                    break;
                case GLFW_KEY_SCROLL_LOCK:
                    out = ui::Key::ScrollLock;
                    break;
                case GLFW_KEY_NUM_LOCK:
                    out = ui::Key::NumLock;
                    break;
                case GLFW_KEY_PRINT_SCREEN:
                    out = ui::Key::PrintScreen;
                    break;
                case GLFW_KEY_PAUSE:
                    out = ui::Key::Pause;
                    break;
                case GLFW_KEY_F1:
                    out = ui::Key::F1;
                    break;
                case GLFW_KEY_F2:
                    out = ui::Key::F2;
                    break;
                case GLFW_KEY_F3:
                    out = ui::Key::F3;
                    break;
                case GLFW_KEY_F4:
                    out = ui::Key::F4;
                    break;
                case GLFW_KEY_F5:
                    out = ui::Key::F5;
                    break;
                case GLFW_KEY_F6:
                    out = ui::Key::F6;
                    break;
                case GLFW_KEY_F7:
                    out = ui::Key::F7;
                    break;
                case GLFW_KEY_F8:
                    out = ui::Key::F8;
                    break;
                case GLFW_KEY_F9:
                    out = ui::Key::F9;
                    break;
                case GLFW_KEY_F10:
                    out = ui::Key::F10;
                    break;
                case GLFW_KEY_F11:
                    out = ui::Key::F11;
                    break;
                case GLFW_KEY_F12:
                    out = ui::Key::F12;
                    break;
                case GLFW_KEY_LEFT_SHIFT:
                    out = ui::Key::LeftShift;
                    break;
                case GLFW_KEY_LEFT_CONTROL:
                    out = ui::Key::LeftControl;
                    break;
                case GLFW_KEY_LEFT_ALT:
                    out = ui::Key::LeftAlt;
                    break;
                case GLFW_KEY_LEFT_SUPER:
                    out = ui::Key::LeftSuper;
                    break;
                case GLFW_KEY_RIGHT_SHIFT:
                    out = ui::Key::RightShift;
                    break;
                case GLFW_KEY_RIGHT_CONTROL:
                    out = ui::Key::RightControl;
                    break;
                case GLFW_KEY_RIGHT_ALT:
                    out = ui::Key::RightAlt;
                    break;
                case GLFW_KEY_RIGHT_SUPER:
                    out = ui::Key::RightSuper;
                    break;
                }
                return out;
            }

#if defined(_WINDOWS)
            //! \bug
            //! https://social.msdn.microsoft.com/Forums/vstudio/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
            typedef unsigned int tl_char_t;
#else  // _WINDOWS
            typedef char32_t tl_char_t;
#endif // _WINDOWS
        } // namespace

        struct Window::Private
        {
            std::shared_ptr<observer::Value<math::Size2i> > windowSize;
            std::shared_ptr<observer::Value<bool> > visible;
            std::shared_ptr<observer::Value<bool> > fullScreen;
            std::shared_ptr<observer::Value<bool> > floatOnTop;
            std::shared_ptr<observer::Value<bool> > close;
            std::shared_ptr<observer::Value<image::PixelType> > colorBuffer;

            std::shared_ptr<gl::GLFWWindow> glfwWindow;
            math::Size2i frameBufferSize;
            float displayScale = 1.F;
            bool refresh = false;
            int modifiers = 0;
            std::shared_ptr<timeline_gl::TextureCache> textureCache;
            std::shared_ptr<timeline_gl::Render> render;
            std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;
#if defined(TLRENDER_API_GLES_2)
            std::shared_ptr<gl::Shader> shader;
#endif // TLRENDER_API_GLES_2
        };

        void Window::_init(
            const std::string& name,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<Window>& share)
        {
            IWindow::_init("tl::ui::Window", context, nullptr);
            TLRENDER_P();

            p.windowSize =
                observer::Value<math::Size2i>::create(math::Size2i(1920, 1080));
            p.visible = observer::Value<bool>::create(false);
            p.fullScreen = observer::Value<bool>::create(false);
            p.floatOnTop = observer::Value<bool>::create(false);
            p.close = observer::Value<bool>::create(false);
            p.colorBuffer = observer::Value<image::PixelType>::create(
                image::PixelType::RGBA_U8);

            p.glfwWindow = gl::GLFWWindow::create(
                name, p.windowSize->get(), context,
                static_cast<int>(gl::GLFWWindowOptions::DoubleBuffer) |
                    static_cast<int>(gl::GLFWWindowOptions::MakeCurrent),
                share ? share->getGLFWWindow() : nullptr);
            p.glfwWindow->setFrameBufferSizeCallback(
                [this](const math::Size2i& value)
                {
                    _p->frameBufferSize = value;
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                });
            p.glfwWindow->setContentScaleCallback(
                [this](const math::Vector2f& value)
                {
                    _p->displayScale = value.x;
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                });
            p.glfwWindow->setRefreshCallback([this] { _p->refresh = true; });
            p.glfwWindow->setCursorEnterCallback([this](bool value)
                                                 { _cursorEnter(value); });
            p.glfwWindow->setCursorPosCallback(
                [this](const math::Vector2f& value)
                {
                    math::Vector2i pos;
#if defined(__APPLE__)
                    //! \bug The mouse position needs to be scaled on macOS?
                    pos.x = value.x * _p->displayScale;
                    pos.y = value.y * _p->displayScale;
#else  // __APPLE__
                    pos.x = value.x;
                    pos.y = value.y;
#endif // __APPLE__
                    _cursorPos(pos);
                });
            p.glfwWindow->setButtonCallback(
                [this](int button, int action, int modifiers)
                {
                    _p->modifiers = modifiers;
                    _mouseButton(
                        button, GLFW_PRESS == action,
                        fromGLFWModifiers(modifiers));
                });
            p.glfwWindow->setScrollCallback(
                [this](const math::Vector2f& value)
                { _scroll(value, fromGLFWModifiers(_p->modifiers)); });
            p.glfwWindow->setKeyCallback(
                [this](int key, int scanCode, int action, int modifiers)
                {
                    TLRENDER_P();
                    p.modifiers = modifiers;
                    switch (action)
                    {
                    case GLFW_PRESS:
                    case GLFW_REPEAT:
                        _key(
                            fromGLFWKey(key), true,
                            fromGLFWModifiers(modifiers));
                        break;
                    case GLFW_RELEASE:
                        _key(
                            fromGLFWKey(key), false,
                            fromGLFWModifiers(modifiers));
                        break;
                    }
                });
            p.glfwWindow->setCharCallback(
                [this](unsigned int c)
                {
                    std::wstring_convert<
                        std::codecvt_utf8<tl_char_t>, tl_char_t>
                        utf32Convert;
                    _text(utf32Convert.to_bytes(c));
                });
            p.glfwWindow->setDropCallback(
                [this](int count, const char** fileNames)
                {
                    std::vector<std::string> tmp;
                    for (int i = 0; i < count; ++i)
                    {
                        tmp.push_back(fileNames[i]);
                    }
                    _drop(tmp);
                });

            p.frameBufferSize = p.glfwWindow->getFrameBufferSize();
            p.displayScale = p.glfwWindow->getContentScale().x;

            if (share)
            {
                p.textureCache = share->_p->render->getTextureCache();
            }
        }

        Window::Window() :
            _p(new Private)
        {
        }

        Window::~Window()
        {
            _makeCurrent();
        }

        std::shared_ptr<Window> Window::create(
            const std::string& name,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<Window>& share)
        {
            auto out = std::shared_ptr<Window>(new Window);
            out->_init(name, context, share);
            return out;
        }

        const math::Size2i& Window::getWindowSize() const
        {
            return _p->windowSize->get();
        }

        std::shared_ptr<observer::IValue<math::Size2i> >
        Window::observeWindowSize() const
        {
            return _p->windowSize;
        }

        void Window::setWindowSize(const math::Size2i& value)
        {
            _p->glfwWindow->setSize(value);
            setGeometry(
                math::Box2i(_geometry.x(), _geometry.y(), value.w, value.h));
        }

        std::shared_ptr<observer::IValue<bool> > Window::observeVisible() const
        {
            return _p->visible;
        }

        int Window::getScreen() const
        {
            return _p->glfwWindow->getScreen();
        }

        bool Window::isFullScreen() const
        {
            return _p->fullScreen->get();
        }

        std::shared_ptr<observer::IValue<bool> >
        Window::observeFullScreen() const
        {
            return _p->fullScreen;
        }

        void Window::setFullScreen(bool value, int screen)
        {
            TLRENDER_P();
            p.glfwWindow->setFullScreen(value, screen);
            p.fullScreen->setIfChanged(value);
        }

        bool Window::isFloatOnTop() const
        {
            return _p->floatOnTop->get();
        }

        std::shared_ptr<observer::IValue<bool> >
        Window::observeFloatOnTop() const
        {
            return _p->floatOnTop;
        }

        void Window::setFloatOnTop(bool value)
        {
            TLRENDER_P();
            p.glfwWindow->setFloatOnTop(value);
            p.floatOnTop->setIfChanged(value);
        }

        std::shared_ptr<observer::IValue<bool> > Window::observeClose() const
        {
            return _p->close;
        }

        image::PixelType Window::getColorBuffer() const
        {
            return _p->colorBuffer->get();
        }

        std::shared_ptr<observer::IValue<image::PixelType> >
        Window::observeColorBuffer() const
        {
            return _p->colorBuffer;
        }

        void Window::setColorBuffer(image::PixelType value)
        {
            if (_p->colorBuffer->setIfChanged(value))
            {
                _updates |= ui::Update::Draw;
            }
        }

        const std::shared_ptr<gl::GLFWWindow>& Window::getGLFWWindow() const
        {
            return _p->glfwWindow;
        }

        void Window::setGeometry(const math::Box2i& value)
        {
            IWindow::setGeometry(value);
            for (const auto& i : _children)
            {
                i->setGeometry(value);
            }
            _p->windowSize->setIfChanged(value.getSize());
        }

        void Window::setVisible(bool value)
        {
            IWindow::setVisible(value);
            TLRENDER_P();
            if (p.visible->setIfChanged(value))
            {
                if (value)
                {
                    p.glfwWindow->show();
                }
                else
                {
                    p.glfwWindow->hide();
                }
            }
        }

        void Window::tickEvent(
            bool parentsVisible, bool parentsEnabled,
            const ui::TickEvent& event)
        {
            IWindow::tickEvent(parentsVisible, parentsEnabled, event);
            TLRENDER_P();

            if (_hasSizeUpdate(shared_from_this()))
            {
                ui::SizeHintEvent sizeHintEvent(
                    event.style, event.iconLibrary, event.fontSystem,
                    p.displayScale);
                _sizeHintEventRecursive(shared_from_this(), sizeHintEvent);

                setGeometry(math::Box2i(p.frameBufferSize));

                _clipEventRecursive(
                    shared_from_this(), _geometry, !isVisible(false));
            }

            if (p.refresh || _hasDrawUpdate(shared_from_this()))
            {
                p.refresh = false;

                _makeCurrent();

                if (!p.render)
                {
                    p.render = timeline_gl::Render::create(
                        _context.lock(), p.textureCache);
                }

                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.colorBuffer->get();
                if (gl::doCreate(
                        p.offscreenBuffer, p.frameBufferSize,
                        offscreenBufferOptions))
                {
                    p.offscreenBuffer = gl::OffscreenBuffer::create(
                        p.frameBufferSize, offscreenBufferOptions);
                }
                if (p.offscreenBuffer)
                {
                    {
                        gl::OffscreenBufferBinding binding(p.offscreenBuffer);
                        timeline::RenderOptions renderOptions;
                        renderOptions.colorBuffer = p.colorBuffer->get();
                        p.render->begin(p.frameBufferSize, renderOptions);
                        ui::DrawEvent drawEvent(
                            event.style, event.iconLibrary, p.render,
                            event.fontSystem);
                        p.render->setClipRectEnabled(true);
                        _drawEventRecursive(
                            shared_from_this(), math::Box2i(p.frameBufferSize),
                            drawEvent);
                        p.render->setClipRectEnabled(false);
                        p.render->end();
                    }
                    glViewport(
                        0, 0, GLsizei(p.frameBufferSize.w),
                        GLsizei(p.frameBufferSize.h));
                    glClearColor(0.F, 0.F, 0.F, 0.F);
                    glClear(GL_COLOR_BUFFER_BIT);
#if defined(TLRENDER_API_GL_4_1)
                    glBindFramebuffer(
                        GL_READ_FRAMEBUFFER, p.offscreenBuffer->getID());
                    glBlitFramebuffer(
                        0, 0, p.frameBufferSize.w, p.frameBufferSize.h, 0, 0,
                        p.frameBufferSize.w, p.frameBufferSize.h,
                        GL_COLOR_BUFFER_BIT, GL_LINEAR);
#elif defined(TLRENDER_API_GLES_2)
                    if (!p.shader)
                    {
                        try
                        {
                            const std::string vertexSource =
                                "precision mediump float;\n"
                                "\n"
                                "attribute vec3 vPos;\n"
                                "attribute vec2 vTexture;\n"
                                "varying vec2 fTexture;\n"
                                "\n"
                                "struct Transform\n"
                                "{\n"
                                "    mat4 mvp;\n"
                                "};\n"
                                "\n"
                                "uniform Transform transform;\n"
                                "\n"
                                "void main()\n"
                                "{\n"
                                "    gl_Position = transform.mvp * vec4(vPos, "
                                "1.0);\n"
                                "    fTexture = vTexture;\n"
                                "}\n";
                            const std::string fragmentSource =
                                "precision mediump float;\n"
                                "\n"
                                "varying vec2 fTexture;\n"
                                "\n"
                                "uniform sampler2D textureSampler;\n"
                                "\n"
                                "void main()\n"
                                "{\n"
                                "    gl_FragColor = texture2D(textureSampler, "
                                "fTexture);\n"
                                "}\n";
                            p.shader = gl::Shader::create(
                                vertexSource, fragmentSource);
                        }
                        catch (const std::exception& e)
                        {
                            if (auto context = _context.lock())
                            {
                                context->log(
                                    "tl::ui_app::Window",
                                    string::Format("Cannot compile shader: {0}")
                                        .arg(e.what()),
                                    log::Type::Error);
                            }
                        }
                    }
                    if (p.shader)
                    {
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                        glDisable(GL_BLEND);
                        glDisable(GL_SCISSOR_TEST);

                        p.shader->bind();
                        p.shader->setUniform(
                            "transform.mvp",
                            math::ortho(
                                0.F, static_cast<float>(p.frameBufferSize.w),
                                0.F, static_cast<float>(p.frameBufferSize.h),
                                -1.F, 1.F));
                        p.shader->setUniform("textureSampler", 0);

                        glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                        glBindTexture(
                            GL_TEXTURE_2D, p.offscreenBuffer->getColorID());

                        auto mesh = geom::box(math::Box2i(
                            0, 0, p.frameBufferSize.w, p.frameBufferSize.h));
                        auto vboData = gl::convert(
                            mesh, gl::VBOType::Pos2_F32_UV_U16,
                            math::SizeTRange(0, mesh.triangles.size() - 1));
                        auto vbo = gl::VBO::create(
                            mesh.triangles.size() * 3,
                            gl::VBOType::Pos2_F32_UV_U16);
                        vbo->copy(vboData);
                        auto vao = gl::VAO::create(
                            gl::VBOType::Pos2_F32_UV_U16, vbo->getID());
                        vao->bind();
                        vao->draw(GL_TRIANGLES, 0, mesh.triangles.size() * 3);
                    }
#endif // TLRENDER_API_GL_4_1

                    p.glfwWindow->swap();
                }

                _doneCurrent();
            }

            if (p.glfwWindow->shouldClose())
            {
                hide();
                p.close->setAlways(true);
            }
        }

        void Window::_makeCurrent()
        {
            TLRENDER_P();
            if (p.glfwWindow)
            {
                p.glfwWindow->makeCurrent();
            }
        }

        void Window::_doneCurrent()
        {
            TLRENDER_P();
            if (p.glfwWindow)
            {
                p.glfwWindow->doneCurrent();
            }
        }

        bool
        Window::_hasSizeUpdate(const std::shared_ptr<IWidget>& widget) const
        {
            bool out = widget->getUpdates() & ui::Update::Size;
            if (out)
            {
                // std::cout << "Size update: " << widget->getObjectName() <<
                // std::endl;
            }
            else
            {
                for (const auto& child : widget->getChildren())
                {
                    out |= _hasSizeUpdate(child);
                }
            }
            return out;
        }

        void Window::_sizeHintEventRecursive(
            const std::shared_ptr<IWidget>& widget,
            const ui::SizeHintEvent& event)
        {
            for (const auto& child : widget->getChildren())
            {
                _sizeHintEventRecursive(child, event);
            }
            widget->sizeHintEvent(event);
        }

        bool
        Window::_hasDrawUpdate(const std::shared_ptr<IWidget>& widget) const
        {
            bool out = false;
            if (!widget->isClipped())
            {
                out = widget->getUpdates() & ui::Update::Draw;
                if (out)
                {
                    // std::cout << "Draw update: " << widget->getObjectName()
                    // << std::endl;
                }
                else
                {
                    for (const auto& child : widget->getChildren())
                    {
                        out |= _hasDrawUpdate(child);
                    }
                }
            }
            return out;
        }

        void Window::_drawEventRecursive(
            const std::shared_ptr<IWidget>& widget, const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            const math::Box2i& g = widget->getGeometry();
            if (!widget->isClipped() && g.w() > 0 && g.h() > 0)
            {
                event.render->setClipRect(drawRect);
                widget->drawEvent(drawRect, event);
                const math::Box2i childrenClipRect =
                    widget->getChildrenClipRect().intersect(drawRect);
                event.render->setClipRect(childrenClipRect);
                for (const auto& child : widget->getChildren())
                {
                    const math::Box2i& childGeometry = child->getGeometry();
                    if (childGeometry.intersects(childrenClipRect))
                    {
                        _drawEventRecursive(
                            child, childGeometry.intersect(childrenClipRect),
                            event);
                    }
                }
                event.render->setClipRect(drawRect);
                widget->drawOverlayEvent(drawRect, event);
            }
        }
    } // namespace ui_app
} // namespace tl

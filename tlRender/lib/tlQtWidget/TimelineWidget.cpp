// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineWidget.h>

#include <tlQtWidget/Util.h>

#include <tlTimelineUI/TimelineWidget.h>

#include <tlUI/IClipboard.h>
#include <tlUI/IWindow.h>
#include <tlUI/RowLayout.h>

#include <tlTimelineGL/Render.h>

#include <tlGL/Init.h>
#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>

#include <QClipboard>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QGuiApplication>
#include <QMimeData>
#include <QTimer>

namespace tl
{
    namespace qtwidget
    {
        namespace
        {
            const size_t timeout = 5;

            class TimelineWindow : public ui::IWindow
            {
                TLRENDER_NON_COPYABLE(TimelineWindow);

            public:
                void _init(const std::shared_ptr<system::Context>& context)
                {
                    IWindow::_init(
                        "tl::qtwidget::TimelineWindow", context, nullptr);
                }

                TimelineWindow() {}

            public:
                virtual ~TimelineWindow() {}

                static std::shared_ptr<TimelineWindow>
                create(const std::shared_ptr<system::Context>& context)
                {
                    auto out =
                        std::shared_ptr<TimelineWindow>(new TimelineWindow);
                    out->_init(context);
                    return out;
                }

                bool key(ui::Key key, bool press, int modifiers)
                {
                    return _key(key, press, modifiers);
                }

                void text(const std::string& text) { _text(text); }

                void cursorEnter(bool enter) { _cursorEnter(enter); }

                void cursorPos(const math::Vector2i& value)
                {
                    _cursorPos(value);
                }

                void mouseButton(int button, bool press, int modifiers)
                {
                    _mouseButton(button, press, modifiers);
                }

                void scroll(const math::Vector2f& value, int modifiers)
                {
                    _scroll(value, modifiers);
                }

                void setGeometry(const math::Box2i& value) override
                {
                    IWindow::setGeometry(value);
                    for (const auto& i : _children)
                    {
                        i->setGeometry(value);
                    }
                }
            };

            class Clipboard : public ui::IClipboard
            {
                TLRENDER_NON_COPYABLE(Clipboard);

            public:
                void _init(const std::shared_ptr<system::Context>& context)
                {
                    IClipboard::_init(context);
                }

                Clipboard() {}

            public:
                virtual ~Clipboard() {}

                static std::shared_ptr<Clipboard>
                create(const std::shared_ptr<system::Context>& context)
                {
                    auto out = std::shared_ptr<Clipboard>(new Clipboard);
                    out->_init(context);
                    return out;
                }

                std::string getText() const override
                {
                    QClipboard* clipboard = QGuiApplication::clipboard();
                    return clipboard->text().toUtf8().data();
                }

                void setText(const std::string& value) override
                {
                    QClipboard* clipboard = QGuiApplication::clipboard();
                    clipboard->setText(QString::fromUtf8(value.c_str()));
                }
            };
        } // namespace

        struct TimelineWidget::Private
        {
            std::weak_ptr<system::Context> context;

            std::shared_ptr<ui::Style> style;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<image::FontSystem> fontSystem;
            std::shared_ptr<Clipboard> clipboard;
            std::shared_ptr<timeline::IRender> render;
            timelineui::ItemOptions itemOptions;
            std::shared_ptr<timelineui::TimelineWidget> timelineWidget;
            std::shared_ptr<TimelineWindow> timelineWindow;
            std::shared_ptr<tl::gl::Shader> shader;
            std::shared_ptr<tl::gl::OffscreenBuffer> buffer;
            std::shared_ptr<gl::VBO> vbo;
            std::shared_ptr<gl::VAO> vao;
            std::chrono::steady_clock::time_point mouseWheelTimer;
            std::unique_ptr<QTimer> timer;

            std::shared_ptr<observer::ValueObserver<bool> > editableObserver;
            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<observer::ValueObserver<bool> > scrubObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> >
                timeScrubObserver;
        };

        TimelineWidget::TimelineWidget(
            const std::shared_ptr<ui::Style>& style,
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context, QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            // QSurfaceFormat surfaceFormat;
            // surfaceFormat.setMajorVersion(4);
            // surfaceFormat.setMinorVersion(1);
            // surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            // surfaceFormat.setStencilBufferSize(8);
            // setFormat(surfaceFormat);

            p.style = style;
            p.iconLibrary = ui::IconLibrary::create(context);
            p.fontSystem = context->getSystem<image::FontSystem>();
            p.clipboard = Clipboard::create(context);
            p.timelineWidget =
                timelineui::TimelineWidget::create(timeUnitsModel, context);
            // p.timelineWidget->setScrollBarsVisible(false);
            p.timelineWindow = TimelineWindow::create(context);
            p.timelineWindow->setClipboard(p.clipboard);
            p.timelineWidget->setParent(p.timelineWindow);

            _inputUpdate();
            _styleUpdate();

            p.editableObserver = observer::ValueObserver<bool>::create(
                p.timelineWidget->observeEditable(),
                [this](bool value) { Q_EMIT editableChanged(value); });

            p.frameViewObserver = observer::ValueObserver<bool>::create(
                p.timelineWidget->observeFrameView(),
                [this](bool value) { Q_EMIT frameViewChanged(value); });

            p.scrubObserver = observer::ValueObserver<bool>::create(
                p.timelineWidget->observeScrub(),
                [this](bool value) { Q_EMIT scrubChanged(value); });

            p.timeScrubObserver =
                observer::ValueObserver<otime::RationalTime>::create(
                    p.timelineWidget->observeTimeScrub(),
                    [this](const otime::RationalTime& value)
                    { Q_EMIT timeScrubbed(value); });

            p.timer.reset(new QTimer);
            p.timer->setTimerType(Qt::PreciseTimer);
            connect(
                p.timer.get(), &QTimer::timeout, this,
                &TimelineWidget::_timerUpdate);
            p.timer->start(timeout);
        }

        TimelineWidget::~TimelineWidget()
        {
            makeCurrent();
        }

        void TimelineWidget::setPlayer(
            const std::shared_ptr<timeline::Player>& player)
        {
            _p->timelineWidget->setPlayer(player);
        }

        bool TimelineWidget::isEditable() const
        {
            return _p->timelineWidget->isEditable();
        }

        bool TimelineWidget::hasFrameView() const
        {
            return _p->timelineWidget->hasFrameView();
        }

        bool TimelineWidget::areScrollBarsVisible() const
        {
            return _p->timelineWidget->areScrollBarsVisible();
        }

        bool TimelineWidget::hasScrollToCurrentFrame() const
        {
            return _p->timelineWidget->hasScrollToCurrentFrame();
        }

        ui::KeyModifier TimelineWidget::scrollKeyModifier() const
        {
            return _p->timelineWidget->getScrollKeyModifier();
        }

        float TimelineWidget::mouseWheelScale() const
        {
            return _p->timelineWidget->getMouseWheelScale();
        }

        bool TimelineWidget::hasStopOnScrub() const
        {
            return _p->timelineWidget->hasStopOnScrub();
        }

        const std::vector<int>& TimelineWidget::frameMarkers() const
        {
            return _p->timelineWidget->getFrameMarkers();
        }

        const timelineui::ItemOptions& TimelineWidget::itemOptions() const
        {
            return _p->timelineWidget->getItemOptions();
        }

        const timelineui::DisplayOptions& TimelineWidget::displayOptions() const
        {
            return _p->timelineWidget->getDisplayOptions();
        }

        QSize TimelineWidget::minimumSizeHint() const
        {
            math::Size2i sizeHint = _p->timelineWidget->getSizeHint();
            const float devicePixelRatio = window()->devicePixelRatio();
            sizeHint.w /= devicePixelRatio;
            sizeHint.h /= devicePixelRatio;
            if (!sizeHint.isValid())
            {
                sizeHint.w = 1;
                sizeHint.h = 1;
            }
            return QSize(sizeHint.w, sizeHint.h);
        }

        QSize TimelineWidget::sizeHint() const
        {
            return minimumSizeHint();
        }

        void TimelineWidget::setEditable(bool value)
        {
            _p->timelineWidget->setEditable(value);
        }

        void TimelineWidget::setFrameView(bool value)
        {
            _p->timelineWidget->setFrameView(value);
        }

        void TimelineWidget::setScrollBarsVisible(bool value)
        {
            _p->timelineWidget->setScrollBarsVisible(value);
        }

        void TimelineWidget::setScrollToCurrentFrame(bool value)
        {
            _p->timelineWidget->setScrollToCurrentFrame(value);
        }

        void TimelineWidget::setScrollKeyModifier(ui::KeyModifier value)
        {
            _p->timelineWidget->setScrollKeyModifier(value);
        }

        void TimelineWidget::setMouseWheelScale(float value)
        {
            _p->timelineWidget->setMouseWheelScale(value);
        }

        void TimelineWidget::setStopOnScrub(bool value)
        {
            _p->timelineWidget->setStopOnScrub(value);
        }

        void TimelineWidget::setFrameMarkers(const std::vector<int>& value)
        {
            _p->timelineWidget->setFrameMarkers(value);
        }

        void
        TimelineWidget::setItemOptions(const timelineui::ItemOptions& value)
        {
            TLRENDER_P();
            const bool inputChanged =
                p.timelineWidget->getItemOptions().inputEnabled !=
                value.inputEnabled;
            p.timelineWidget->setItemOptions(value);
            if (inputChanged)
            {
                _inputUpdate();
            }
        }

        void TimelineWidget::setDisplayOptions(
            const timelineui::DisplayOptions& value)
        {
            _p->timelineWidget->setDisplayOptions(value);
        }

        void TimelineWidget::initializeGL()
        {
            TLRENDER_P();
            initializeOpenGLFunctions();
            gl::initGLAD();
            if (auto context = p.context.lock())
            {
                try
                {
                    p.render = timeline_gl::Render::create(context);

                    const std::string vertexSource =
                        "#version 410\n"
                        "\n"
                        "in vec3 vPos;\n"
                        "in vec2 vTexture;\n"
                        "out vec2 fTexture;\n"
                        "\n"
                        "uniform struct Transform\n"
                        "{\n"
                        "    mat4 mvp;\n"
                        "} transform;\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                        "    fTexture = vTexture;\n"
                        "}\n";
                    const std::string fragmentSource =
                        "#version 410\n"
                        "\n"
                        "in vec2 fTexture;\n"
                        "out vec4 fColor;\n"
                        "\n"
                        "uniform sampler2D textureSampler;\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    fColor = texture(textureSampler, fTexture);\n"
                        "}\n";
                    p.shader = gl::Shader::create(vertexSource, fragmentSource);
                }
                catch (const std::exception& e)
                {
                    if (auto context = p.context.lock())
                    {
                        context->log(
                            "tl::qtwidget::TimelineWidget", e.what(),
                            log::Type::Error);
                    }
                }
            }
            _sizeHintEvent();
        }

        void TimelineWidget::resizeGL(int w, int h)
        {
            TLRENDER_P();
            _setGeometry();
            p.vao.reset();
            p.vbo.reset();
        }

        void TimelineWidget::paintGL()
        {
            TLRENDER_P();
            const math::Size2i renderSize(_toUI(width()), _toUI(height()));
            if (_getDrawUpdate(p.timelineWindow))
            {
                try
                {
                    if (renderSize.isValid())
                    {
                        gl::OffscreenBufferOptions offscreenBufferOptions;
                        offscreenBufferOptions.colorType =
                            image::PixelType::RGBA_U8;
                        if (gl::doCreate(
                                p.buffer, renderSize, offscreenBufferOptions))
                        {
                            p.buffer = gl::OffscreenBuffer::create(
                                renderSize, offscreenBufferOptions);
                        }
                    }
                    else
                    {
                        p.buffer.reset();
                    }

                    if (p.render && p.buffer)
                    {
                        gl::OffscreenBufferBinding binding(p.buffer);
                        timeline::RenderOptions renderOptions;
                        renderOptions.clearColor =
                            p.style->getColorRole(ui::ColorRole::Window);
                        p.render->begin(renderSize, renderOptions);
                        ui::DrawEvent drawEvent(
                            p.style, p.iconLibrary, p.render, p.fontSystem);
                        p.render->setClipRectEnabled(true);
                        _drawEvent(
                            p.timelineWindow, math::Box2i(renderSize),
                            drawEvent);
                        p.render->setClipRectEnabled(false);
                        p.render->end();
                    }
                }
                catch (const std::exception& e)
                {
                    if (auto context = p.context.lock())
                    {
                        context->log(
                            "tl::qtwidget::TimelineWidget", e.what(),
                            log::Type::Error);
                    }
                }
            }

            glViewport(0, 0, renderSize.w, renderSize.h);
            glClearColor(0.F, 0.F, 0.F, 0.F);
            glClear(GL_COLOR_BUFFER_BIT);

            if (p.buffer)
            {
                p.shader->bind();
                const auto pm = math::ortho(
                    0.F, static_cast<float>(renderSize.w), 0.F,
                    static_cast<float>(renderSize.h), -1.F, 1.F);
                p.shader->setUniform("transform.mvp", pm);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, p.buffer->getColorID());

                const auto mesh =
                    geom::box(math::Box2i(0, 0, renderSize.w, renderSize.h));
                if (!p.vbo)
                {
                    p.vbo = gl::VBO::create(
                        mesh.triangles.size() * 3,
                        gl::VBOType::Pos2_F32_UV_U16);
                }
                if (p.vbo)
                {
                    p.vbo->copy(convert(mesh, gl::VBOType::Pos2_F32_UV_U16));
                }

                if (!p.vao && p.vbo)
                {
                    p.vao = gl::VAO::create(
                        gl::VBOType::Pos2_F32_UV_U16, p.vbo->getID());
                }
                if (p.vao && p.vbo)
                {
                    p.vao->bind();
                    p.vao->draw(GL_TRIANGLES, 0, p.vbo->getSize());
                }
            }
        }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        void TimelineWidget::enterEvent(QEvent* event)
#else
        void TimelineWidget::enterEvent(QEnterEvent* event)
#endif // QT_VERSION
        {
            TLRENDER_P();
            if (p.timelineWidget->getItemOptions().inputEnabled)
            {
                event->accept();
                p.timelineWindow->cursorEnter(true);
            }
        }

        void TimelineWidget::leaveEvent(QEvent* event)
        {
            TLRENDER_P();
            if (p.timelineWidget->getItemOptions().inputEnabled)
            {
                event->accept();
                p.timelineWindow->cursorEnter(false);
            }
        }

        namespace
        {
            int fromQtModifiers(int value)
            {
                int out = 0;
                if (value & Qt::ShiftModifier)
                {
                    out |= static_cast<int>(ui::KeyModifier::Shift);
                }
                if (value & Qt::ControlModifier)
                {
                    out |= static_cast<int>(ui::KeyModifier::Control);
                }
                if (value & Qt::AltModifier)
                {
                    out |= static_cast<int>(ui::KeyModifier::Alt);
                }
                return out;
            }
        } // namespace

        void TimelineWidget::mousePressEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            if (p.timelineWidget->getItemOptions().inputEnabled)
            {
                event->accept();
                int button = -1;
                if (event->button() == Qt::LeftButton)
                {
                    button = 0;
                }
                p.timelineWindow->mouseButton(
                    button, true, fromQtModifiers(event->modifiers()));
            }
        }

        void TimelineWidget::mouseReleaseEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            if (p.timelineWidget->getItemOptions().inputEnabled)
            {
                event->accept();
                int button = 0;
                if (event->button() == Qt::LeftButton)
                {
                    button = 1;
                }
                p.timelineWindow->mouseButton(
                    button, false, fromQtModifiers(event->modifiers()));
            }
        }

        void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            if (p.timelineWidget->getItemOptions().inputEnabled)
            {
                event->accept();
                p.timelineWindow->cursorPos(
                    math::Vector2i(_toUI(event->x()), _toUI(event->y())));
            }
        }

        void TimelineWidget::wheelEvent(QWheelEvent* event)
        {
            TLRENDER_P();
            if (p.timelineWidget->getItemOptions().inputEnabled)
            {
                const auto now = std::chrono::steady_clock::now();
                const auto diff =
                    std::chrono::duration<float>(now - p.mouseWheelTimer);
                const float delta = event->angleDelta().y() / 8.F / 15.F;
                p.mouseWheelTimer = now;
                p.timelineWindow->scroll(
                    math::Vector2f(
                        event->angleDelta().x() / 8.F / 15.F,
                        event->angleDelta().y() / 8.F / 15.F),
                    fromQtModifiers(event->modifiers()));
            }
        }

        namespace
        {
            ui::Key fromQtKey(int key)
            {
                ui::Key out = ui::Key::Unknown;
                switch (key)
                {
                case Qt::Key_Space:
                    out = ui::Key::Space;
                    break;
                case Qt::Key_Apostrophe:
                    out = ui::Key::Apostrophe;
                    break;
                case Qt::Key_Comma:
                    out = ui::Key::Comma;
                    break;
                case Qt::Key_Minus:
                    out = ui::Key::Minus;
                    break;
                case Qt::Key_Period:
                    out = ui::Key::Period;
                    break;
                case Qt::Key_Slash:
                    out = ui::Key::Slash;
                    break;
                case Qt::Key_0:
                    out = ui::Key::_0;
                    break;
                case Qt::Key_1:
                    out = ui::Key::_1;
                    break;
                case Qt::Key_2:
                    out = ui::Key::_2;
                    break;
                case Qt::Key_3:
                    out = ui::Key::_3;
                    break;
                case Qt::Key_4:
                    out = ui::Key::_4;
                    break;
                case Qt::Key_5:
                    out = ui::Key::_5;
                    break;
                case Qt::Key_6:
                    out = ui::Key::_6;
                    break;
                case Qt::Key_7:
                    out = ui::Key::_7;
                    break;
                case Qt::Key_8:
                    out = ui::Key::_8;
                    break;
                case Qt::Key_9:
                    out = ui::Key::_9;
                    break;
                case Qt::Key_Semicolon:
                    out = ui::Key::Semicolon;
                    break;
                case Qt::Key_Equal:
                    out = ui::Key::Equal;
                    break;
                case Qt::Key_A:
                    out = ui::Key::A;
                    break;
                case Qt::Key_B:
                    out = ui::Key::B;
                    break;
                case Qt::Key_C:
                    out = ui::Key::C;
                    break;
                case Qt::Key_D:
                    out = ui::Key::D;
                    break;
                case Qt::Key_E:
                    out = ui::Key::E;
                    break;
                case Qt::Key_F:
                    out = ui::Key::F;
                    break;
                case Qt::Key_G:
                    out = ui::Key::G;
                    break;
                case Qt::Key_H:
                    out = ui::Key::H;
                    break;
                case Qt::Key_I:
                    out = ui::Key::I;
                    break;
                case Qt::Key_J:
                    out = ui::Key::J;
                    break;
                case Qt::Key_K:
                    out = ui::Key::K;
                    break;
                case Qt::Key_L:
                    out = ui::Key::L;
                    break;
                case Qt::Key_M:
                    out = ui::Key::M;
                    break;
                case Qt::Key_N:
                    out = ui::Key::N;
                    break;
                case Qt::Key_O:
                    out = ui::Key::O;
                    break;
                case Qt::Key_P:
                    out = ui::Key::P;
                    break;
                case Qt::Key_Q:
                    out = ui::Key::Q;
                    break;
                case Qt::Key_R:
                    out = ui::Key::R;
                    break;
                case Qt::Key_S:
                    out = ui::Key::S;
                    break;
                case Qt::Key_T:
                    out = ui::Key::T;
                    break;
                case Qt::Key_U:
                    out = ui::Key::U;
                    break;
                case Qt::Key_V:
                    out = ui::Key::V;
                    break;
                case Qt::Key_W:
                    out = ui::Key::W;
                    break;
                case Qt::Key_X:
                    out = ui::Key::X;
                    break;
                case Qt::Key_Y:
                    out = ui::Key::Y;
                    break;
                case Qt::Key_Z:
                    out = ui::Key::Z;
                    break;
                case Qt::Key_BracketLeft:
                    out = ui::Key::LeftBracket;
                    break;
                case Qt::Key_Backslash:
                    out = ui::Key::Backslash;
                    break;
                case Qt::Key_BracketRight:
                    out = ui::Key::RightBracket;
                    break;
                case Qt::Key_Agrave:
                    out = ui::Key::GraveAccent;
                    break;
                case Qt::Key_Escape:
                    out = ui::Key::Escape;
                    break;
                case Qt::Key_Enter:
                    out = ui::Key::Enter;
                    break;
                case Qt::Key_Tab:
                    out = ui::Key::Tab;
                    break;
                case Qt::Key_Backspace:
                    out = ui::Key::Backspace;
                    break;
                case Qt::Key_Insert:
                    out = ui::Key::Insert;
                    break;
                case Qt::Key_Delete:
                    out = ui::Key::Delete;
                    break;
                case Qt::Key_Right:
                    out = ui::Key::Right;
                    break;
                case Qt::Key_Left:
                    out = ui::Key::Left;
                    break;
                case Qt::Key_Down:
                    out = ui::Key::Down;
                    break;
                case Qt::Key_Up:
                    out = ui::Key::Up;
                    break;
                case Qt::Key_PageUp:
                    out = ui::Key::PageUp;
                    break;
                case Qt::Key_PageDown:
                    out = ui::Key::PageDown;
                    break;
                case Qt::Key_Home:
                    out = ui::Key::Home;
                    break;
                case Qt::Key_End:
                    out = ui::Key::End;
                    break;
                case Qt::Key_CapsLock:
                    out = ui::Key::CapsLock;
                    break;
                case Qt::Key_ScrollLock:
                    out = ui::Key::ScrollLock;
                    break;
                case Qt::Key_NumLock:
                    out = ui::Key::NumLock;
                    break;
                case Qt::Key_Print:
                    out = ui::Key::PrintScreen;
                    break;
                case Qt::Key_Pause:
                    out = ui::Key::Pause;
                    break;
                case Qt::Key_F1:
                    out = ui::Key::F1;
                    break;
                case Qt::Key_F2:
                    out = ui::Key::F2;
                    break;
                case Qt::Key_F3:
                    out = ui::Key::F3;
                    break;
                case Qt::Key_F4:
                    out = ui::Key::F4;
                    break;
                case Qt::Key_F5:
                    out = ui::Key::F5;
                    break;
                case Qt::Key_F6:
                    out = ui::Key::F6;
                    break;
                case Qt::Key_F7:
                    out = ui::Key::F7;
                    break;
                case Qt::Key_F8:
                    out = ui::Key::F8;
                    break;
                case Qt::Key_F9:
                    out = ui::Key::F9;
                    break;
                case Qt::Key_F10:
                    out = ui::Key::F10;
                    break;
                case Qt::Key_F11:
                    out = ui::Key::F11;
                    break;
                case Qt::Key_F12:
                    out = ui::Key::F12;
                    break;
                case Qt::Key_Shift:
                    out = ui::Key::LeftShift;
                    break;
                case Qt::Key_Control:
                    out = ui::Key::LeftControl;
                    break;
                case Qt::Key_Alt:
                    out = ui::Key::LeftAlt;
                    break;
                case Qt::Key_Super_L:
                    out = ui::Key::LeftSuper;
                    break;
                case Qt::Key_Super_R:
                    out = ui::Key::RightSuper;
                    break;
                }
                return out;
            }
        } // namespace

        void TimelineWidget::keyPressEvent(QKeyEvent* event)
        {
            TLRENDER_P();
            if (p.timelineWidget->getItemOptions().inputEnabled &&
                p.timelineWindow->key(
                    fromQtKey(event->key()), true,
                    fromQtModifiers(event->modifiers())))
            {
                event->accept();
            }
            else
            {
                QOpenGLWidget::keyPressEvent(event);
            }
        }

        void TimelineWidget::keyReleaseEvent(QKeyEvent* event)
        {
            TLRENDER_P();
            if (p.timelineWidget->getItemOptions().inputEnabled &&
                p.timelineWindow->key(
                    fromQtKey(event->key()), false,
                    fromQtModifiers(event->modifiers())))
            {
                event->accept();
            }
            else
            {
                QOpenGLWidget::keyReleaseEvent(event);
            }
        }

        bool TimelineWidget::event(QEvent* event)
        {
            TLRENDER_P();
            bool out = QOpenGLWidget::event(event);
            if (event->type() == QEvent::StyleChange)
            {
                _styleUpdate();
            }
            return out;
        }

        void TimelineWidget::_tickEvent()
        {
            TLRENDER_P();
            ui::TickEvent tickEvent(p.style, p.iconLibrary, p.fontSystem);
            _tickEvent(_p->timelineWindow, true, true, tickEvent);
        }

        void TimelineWidget::_tickEvent(
            const std::shared_ptr<ui::IWidget>& widget, bool visible,
            bool enabled, const ui::TickEvent& event)
        {
            TLRENDER_P();
            const bool parentsVisible = visible && widget->isVisible(false);
            const bool parentsEnabled = enabled && widget->isEnabled(false);
            for (const auto& child : widget->getChildren())
            {
                _tickEvent(child, parentsVisible, parentsEnabled, event);
            }
            widget->tickEvent(visible, enabled, event);
        }

        bool TimelineWidget::_getSizeUpdate(
            const std::shared_ptr<ui::IWidget>& widget) const
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
                    out |= _getSizeUpdate(child);
                }
            }
            return out;
        }

        void TimelineWidget::_sizeHintEvent()
        {
            TLRENDER_P();
            const float devicePixelRatio = window()->devicePixelRatio();
            ui::SizeHintEvent sizeHintEvent(
                p.style, p.iconLibrary, p.fontSystem, devicePixelRatio);
            _sizeHintEvent(p.timelineWindow, sizeHintEvent);
        }

        void TimelineWidget::_sizeHintEvent(
            const std::shared_ptr<ui::IWidget>& widget,
            const ui::SizeHintEvent& event)
        {
            for (const auto& child : widget->getChildren())
            {
                _sizeHintEvent(child, event);
            }
            widget->sizeHintEvent(event);
        }

        void TimelineWidget::_setGeometry()
        {
            TLRENDER_P();
            const math::Box2i geometry(0, 0, _toUI(width()), _toUI(height()));
            p.timelineWindow->setGeometry(geometry);
        }

        void TimelineWidget::_clipEvent()
        {
            TLRENDER_P();
            const math::Box2i geometry(0, 0, _toUI(width()), _toUI(height()));
            _clipEvent(p.timelineWindow, geometry, false);
        }

        void TimelineWidget::_clipEvent(
            const std::shared_ptr<ui::IWidget>& widget,
            const math::Box2i& clipRect, bool clipped)
        {
            const math::Box2i& g = widget->getGeometry();
            clipped |= !g.intersects(clipRect);
            clipped |= !widget->isVisible(false);
            const math::Box2i clipRect2 = g.intersect(clipRect);
            widget->clipEvent(clipRect2, clipped);
            const math::Box2i childrenClipRect =
                widget->getChildrenClipRect().intersect(clipRect2);
            for (const auto& child : widget->getChildren())
            {
                const math::Box2i& childGeometry = child->getGeometry();
                _clipEvent(
                    child, childGeometry.intersect(childrenClipRect), clipped);
            }
        }

        bool TimelineWidget::_getDrawUpdate(
            const std::shared_ptr<ui::IWidget>& widget) const
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
                        out |= _getDrawUpdate(child);
                    }
                }
            }
            return out;
        }

        void TimelineWidget::_drawEvent(
            const std::shared_ptr<ui::IWidget>& widget,
            const math::Box2i& drawRect, const ui::DrawEvent& event)
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
                        _drawEvent(
                            child, childGeometry.intersect(childrenClipRect),
                            event);
                    }
                }
                event.render->setClipRect(drawRect);
                widget->drawOverlayEvent(drawRect, event);
            }
        }

        int TimelineWidget::_toUI(int value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return value * devicePixelRatio;
        }

        math::Vector2i TimelineWidget::_toUI(const math::Vector2i& value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return value * devicePixelRatio;
        }

        int TimelineWidget::_fromUI(int value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio) : 0.F;
        }

        math::Vector2i
        TimelineWidget::_fromUI(const math::Vector2i& value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio)
                                          : math::Vector2i();
        }

        void TimelineWidget::_inputUpdate()
        {
            TLRENDER_P();
            const bool inputEnabled =
                p.timelineWidget->getItemOptions().inputEnabled;
            setMouseTracking(inputEnabled);
            setFocusPolicy(inputEnabled ? Qt::StrongFocus : Qt::NoFocus);
            if (!inputEnabled)
            {
                p.timelineWindow->cursorEnter(false);
            }
        }

        void TimelineWidget::_timerUpdate()
        {
            if (_p)
            {
                _tickEvent();
                if (_getSizeUpdate(_p->timelineWindow))
                {
                    _sizeHintEvent();
                    _setGeometry();
                    _clipEvent();
                    updateGeometry();
                }
                if (_getDrawUpdate(_p->timelineWindow))
                {
                    update();
                }
            }
        }

        void TimelineWidget::_styleUpdate()
        {
            /*TLRENDER_P();
            const auto palette = this->palette();
            p.style->setColorRole(
                ui::ColorRole::Window,
                fromQt(palette.color(QPalette::ColorRole::Window)));
            p.style->setColorRole(
                ui::ColorRole::Base,
                fromQt(palette.color(QPalette::ColorRole::Base)));
            p.style->setColorRole(
                ui::ColorRole::Button,
                fromQt(palette.color(QPalette::ColorRole::Button)));
            p.style->setColorRole(
                ui::ColorRole::Text,
                fromQt(palette.color(QPalette::ColorRole::WindowText)));*/
        }
    } // namespace qtwidget
} // namespace tl

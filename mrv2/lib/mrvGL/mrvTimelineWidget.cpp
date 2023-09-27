// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl.H>
#include <FL/names.h>

#include <tlTimelineUI/TimelineWidget.h>

#include <tlUI/EventLoop.h>
#include <tlUI/IClipboard.h>

#include <tlTimeline/GLRender.h>

#include <tlGL/Init.h>
#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>

#include "mrvCore/mrvHotkey.h"

#include "mrvEdit/mrvEditCallbacks.h"
#include "mrvEdit/mrvEditUtil.h"

#include "mrvFl/mrvIO.h"

#include "mrvGL/mrvThumbnailCreator.h"
#include "mrvGL/mrvTimelineWidget.h"
#include "mrvGL/mrvGLErrors.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrViewer.h"

namespace mrv
{
    namespace
    {
        const double kTimeout = 0.0; // 05;
        const char* kModule = "timelineui";
    } // namespace

    namespace
    {
        int getIndex(const otio::SerializableObject::Retainer<otio::Composable>&
                         composable)
        {
            int out = -1;
            if (composable && composable->parent())
            {
                const auto& children = composable->parent()->children();
                for (int i = 0; i < children.size(); ++i)
                {
                    if (composable == children[i].value)
                    {
                        out = i;
                        break;
                    }
                }
            }
            return out;
        }
    } // namespace

    namespace
    {
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
            ~Clipboard() override {}

            static std::shared_ptr<Clipboard>
            create(const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<Clipboard>(new Clipboard);
                out->_init(context);
                return out;
            }

            std::string getText() const override
            {
                std::string text;
                if (Fl::event_text())
                    text = Fl::event_text();
                return text;
            }

            void setText(const std::string& value) override
            {
                Fl::copy(value.c_str(), value.size());
            }
        };
    } // namespace

    struct TimelineWidget::Private
    {
        std::weak_ptr<system::Context> context;

        ViewerUI* ui = nullptr;

        TimelinePlayer* player = nullptr;

        ThumbnailCreator* thumbnailCreator = nullptr;
        Fl_Double_Window* thumbnailWindow = nullptr; // thumbnail window
        int64_t thumbnailRequestId = 0;
        Fl_Box* box = nullptr;

        timeline::ColorConfigOptions colorConfigOptions;
        timeline::LUTOptions lutOptions;

        mrv::TimeUnits units = mrv::TimeUnits::Timecode;

        std::shared_ptr<ui::Style> style;
        std::shared_ptr<ui::IconLibrary> iconLibrary;
        std::shared_ptr<image::FontSystem> fontSystem;
        std::shared_ptr<Clipboard> clipboard;
        std::shared_ptr<timeline::IRender> render;
        std::shared_ptr<ui::EventLoop> eventLoop;
        timelineui::ItemOptions itemOptions;
        std::shared_ptr<timelineui::TimelineWidget> timelineWidget;
        std::shared_ptr<tl::gl::Shader> shader;
        std::shared_ptr<tl::gl::OffscreenBuffer> buffer;
        std::shared_ptr<gl::VBO> vbo;
        std::shared_ptr<gl::VAO> vao;
        std::chrono::steady_clock::time_point mouseWheelTimer;

        bool dragging = false;

        otime::TimeRange timeRange = time::invalidTimeRange;
    };

    TimelineWidget::TimelineWidget(int X, int Y, int W, int H, const char* L) :
        Fl_Gl_Window(X, Y, W, H, L),
        _p(new Private)
    {
        mode(FL_RGB | FL_ALPHA | FL_STENCIL | FL_OPENGL3);
    }

    void TimelineWidget::setContext(
        const std::shared_ptr<system::Context>& context,
        const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
        ViewerUI* ui)
    {
        TLRENDER_P();

        p.context = context;

        p.ui = ui;

        p.style = ui::Style::create(context);
        p.iconLibrary = ui::IconLibrary::create(context);
        p.fontSystem = image::FontSystem::create(context);
        p.clipboard = Clipboard::create(context);
        p.eventLoop =
            ui::EventLoop::create(p.style, p.iconLibrary, p.clipboard, context);

        p.timelineWidget =
            timelineui::TimelineWidget::create(timeUnitsModel, context);
        p.timelineWidget->setEditable(false);
        p.timelineWidget->setFrameView(true);
        p.timelineWidget->setScrollBarsVisible(false);
        p.timelineWidget->setStopOnScrub(false);
        p.timelineWidget->setInsertCallback(std::bind(
            &mrv::TimelineWidget::insertCallback, this, std::placeholders::_1));

        p.eventLoop->addWidget(p.timelineWidget);
        const float devicePixelRatio = pixels_per_unit();
        p.eventLoop->setDisplayScale(devicePixelRatio);
        p.eventLoop->setDisplaySize(math::Size2i(_toUI(w()), _toUI(h())));

        p.thumbnailCreator = new ThumbnailCreator(context);

        setStopOnScrub(false);

        _styleUpdate();

        Fl::add_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
    }

    ThumbnailCreator* TimelineWidget::thumbnailCreator()
    {
        return _p->thumbnailCreator;
    }

    void TimelineWidget::setStyle(const std::shared_ptr<ui::Style>& style)
    {
        _p->style = style;
        _styleUpdate();
    }

    void TimelineWidget::hideThumbnail_cb(TimelineWidget* t)
    {
        t->hideThumbnail();
    }

    void TimelineWidget::hideThumbnail()
    {
        TLRENDER_P();
        if (!p.thumbnailWindow)
            return;

        p.thumbnailWindow->hide();
    }

    TimelineWidget::~TimelineWidget()
    {
        TLRENDER_P();
        if (p.box)
        {
            delete p.box->image();
            p.box->image(nullptr);
        }
    }

    bool TimelineWidget::isEditable() const
    {
        return _p->timelineWidget->isEditable();
    }

    void TimelineWidget::setEditable(bool value)
    {
        _p->timelineWidget->setEditable(value);
    }

    void TimelineWidget::_seek()
    {
        TLRENDER_P();
        const int maxY = 48;
        const int Y = _toUI(Fl::event_y());
        const int X = _toUI(Fl::event_x());
        if ((Y < maxY && !p.timelineWidget->isDragging()) ||
            !p.timelineWidget->isEditable())
        {
            auto time = _posToTime(X);
            p.player->seek(time);
        }
        else
        {
            p.dragging = p.timelineWidget->isDragging();
        }
    }

    int TimelineWidget::_requestThumbnail(bool fetch)
    {
        TLRENDER_P();
        if (!p.player)
            return 0;

        const auto player = p.player->player();

        if (!p.ui->uiPrefs->uiPrefsTimelineThumbnails->value())
        {
            if (!Fl::has_timeout((Fl_Timeout_Handler)hideThumbnail_cb, this))
            {
                Fl::add_timeout(
                    0.005, (Fl_Timeout_Handler)hideThumbnail_cb, this);
            }
            return 0;
        }
        int W = 128;
        int H = 90;
        int X = Fl::event_x_root() - p.ui->uiMain->x() - W / 2;
        // int Y = Fl::event_y_root() - Fl::event_y() - H - 20;
        int Y = y() - H - 10;
        if (X < 0)
            X = 0;
        else if (X + W / 2 > x() + w())
            X -= W / 2;

        char buffer[64];
        if (!p.thumbnailWindow)
        {
            // Open a thumbnail window just above the timeline
            Fl_Group::current(p.ui->uiMain);
            p.thumbnailWindow = new Fl_Double_Window(X, Y, W, H);
            p.thumbnailWindow->clear_border();
            p.thumbnailWindow->set_non_modal();
            p.thumbnailWindow->callback((Fl_Callback*)0);
            p.thumbnailWindow->begin();

            p.box = new Fl_Box(2, 2, W - 2, H - 2);
            p.box->box(FL_FLAT_BOX);
            p.box->labelcolor(fl_contrast(p.box->labelcolor(), p.box->color()));
            p.thumbnailWindow->end();
            p.thumbnailWindow->show();
        }
#ifdef _WIN32
        // Without this, the window would not show on Windows
        if (fetch)
        {
            p.thumbnailWindow->resize(X, Y, W, H);
            p.thumbnailWindow->show();
        }
#else
        p.thumbnailWindow->resize(X, Y, W, H);
#endif

        file::Path path;
        auto model = p.ui->app->filesModel();
        auto Aitem = model->observeA()->get();
        if (Aitem)
            path = Aitem->path;
        else
            path = player->getPath();
        image::Size size(p.box->w(), p.box->h() - 24);
        const auto& time = _posToTime(_toUI(Fl::event_x()));

        if (p.thumbnailCreator)
        {
            if (p.thumbnailRequestId)
            {
                p.thumbnailCreator->cancelRequests(p.thumbnailRequestId);
            }

            if (fetch)
            {
                uint16_t layerId = p.ui->uiColorChannel->value();
                p.thumbnailCreator->initThread();
                p.thumbnailRequestId = p.thumbnailCreator->request(
                    path.get(), time, size, single_thumbnail_cb, (void*)this,
                    layerId, p.colorConfigOptions, p.lutOptions);
            }
        }
        timeToText(buffer, time, _p->units);
        p.box->copy_label(buffer);
        return 1;
    }

    //! Get timelineUI's timelineWidget item options
    timelineui::ItemOptions TimelineWidget::getItemOptions() const
    {
        return _p->timelineWidget->getItemOptions();
    }

    void TimelineWidget::setTimelinePlayer(TimelinePlayer* player)
    {
        TLRENDER_P();
        if (player == p.player)
            return;
        p.player = player;
        if (player)
        {
            auto innerPlayer = player->player();
            p.timeRange = innerPlayer->getTimeRange();
            p.timelineWidget->setPlayer(innerPlayer);
        }
        else
        {
            if (p.thumbnailRequestId)
            {
                p.thumbnailCreator->cancelRequests(p.thumbnailRequestId);
                Fl_Image* image = p.box->image();
                delete image;
                p.box->image(nullptr);
            }
            p.timeRange = time::invalidTimeRange;
            p.timelineWidget->setPlayer(nullptr);
        }
    }

    void TimelineWidget::setLUTOptions(const timeline::LUTOptions& lutOptions)
    {
        TLRENDER_P();
        if (lutOptions == p.lutOptions)
            return;
        p.lutOptions = lutOptions;
    }

    void TimelineWidget::setColorConfigOptions(
        const timeline::ColorConfigOptions& colorConfigOptions)
    {
        TLRENDER_P();
        if (colorConfigOptions == p.colorConfigOptions)
            return;
        p.colorConfigOptions = colorConfigOptions;
    }

    void TimelineWidget::setFrameView(bool value)
    {
        _p->timelineWidget->setFrameView(value);
    }

    void TimelineWidget::setScrollBarsVisible(bool value)
    {
        _p->timelineWidget->setScrollBarsVisible(value);
    }

    void TimelineWidget::setScrollKeyModifier(ui::KeyModifier value)
    {
        _p->timelineWidget->setScrollKeyModifier(value);
    }

    void TimelineWidget::setStopOnScrub(bool value)
    {
        _p->timelineWidget->setStopOnScrub(value);
    }

    void TimelineWidget::setThumbnails(bool value)
    {
        TLRENDER_P();
        p.itemOptions.thumbnails = value;
        _p->timelineWidget->setItemOptions(p.itemOptions);
    }

    void TimelineWidget::setMouseWheelScale(float value)
    {
        _p->timelineWidget->setMouseWheelScale(value);
    }

    void TimelineWidget::setItemOptions(const timelineui::ItemOptions& value)
    {
        _p->timelineWidget->setItemOptions(value);
    }

    void TimelineWidget::_initializeGLResources()
    {
        TLRENDER_P();

        if (auto context = p.context.lock())
        {
            try
            {
                p.render = timeline::GLRender::create(context);
                CHECK_GL;
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
                CHECK_GL;
            }
            catch (const std::exception& e)
            {
                context->log(
                    "mrv::mrvTimelineWidget", e.what(), log::Type::Error);
            }

            p.vao.reset();
            p.vbo.reset();
            p.buffer.reset();
        }
    }

    void TimelineWidget::_initializeGL()
    {
        gl::initGLAD();

        CHECK_GL;
        refresh();
        CHECK_GL;

        _initializeGLResources();
        CHECK_GL;
    }

    void TimelineWidget::resize(int X, int Y, int W, int H)
    {
        TLRENDER_P();

        Fl_Gl_Window::resize(X, Y, W, H);

        if (p.eventLoop)
        {
            const float devicePixelRatio = pixels_per_unit();
            p.eventLoop->setDisplayScale(devicePixelRatio);
            p.eventLoop->setDisplaySize(math::Size2i(_toUI(W), _toUI(H)));
            p.eventLoop->tick();
        }

        p.vbo.reset();
        p.vao.reset();
    }

    void TimelineWidget::draw()
    {
        TLRENDER_P();
        const math::Size2i renderSize(pixel_w(), pixel_h());
        if (!valid())
        {
            _initializeGL();
            CHECK_GL;

            if (p.eventLoop)
            {
                const float devicePixelRatio = pixels_per_unit();
                p.eventLoop->setDisplayScale(devicePixelRatio);
                p.eventLoop->setDisplaySize(renderSize);
                p.eventLoop->tick(); // needed so it refreshes while dragging
            }

            valid(1);
        }
        CHECK_GL;

        bool annotationMarks = false;
        if (p.player)
        {
            annotationMarks = p.player->hasAnnotations();
        }

        if (p.eventLoop->hasDrawUpdate() || annotationMarks || !p.buffer)
        {
            try
            {
                if (renderSize.isValid())
                {
                    gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType =
                        image::PixelType::RGBA_F32;
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
                    p.render->begin(
                        renderSize, timeline::ColorConfigOptions(),
                        timeline::LUTOptions(), renderOptions);
                    p.eventLoop->draw(p.render);
                    if (annotationMarks)
                        _drawAnnotationMarks();
                    p.render->end();
                }
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }
        }

        glViewport(0, 0, renderSize.w, renderSize.h);
        glClearColor(0.F, 0.F, 0.F, 0.F);
        glClear(GL_COLOR_BUFFER_BIT);
        CHECK_GL;

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
                    mesh.triangles.size() * 3, gl::VBOType::Pos2_F32_UV_U16);
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

    int TimelineWidget::enterEvent()
    {
        TLRENDER_P();

        bool takeFocus = true;
        Fl_Widget* focusWidget = Fl::focus();
        TimelineClass* c = p.ui->uiTimeWindow;
        if (focusWidget == c->uiFrame || focusWidget == c->uiStartFrame ||
            focusWidget == c->uiEndFrame)
            takeFocus = false;
        // if (Fl::focus() == nullptr)
        if (takeFocus)
            take_focus();
        p.eventLoop->cursorEnter(true);
        return 1;
    }

    int TimelineWidget::leaveEvent()
    {
        TLRENDER_P();
        p.eventLoop->cursorEnter(false);
        focus(p.ui->uiView);
        return 1;
    }

    namespace
    {
        int fromFLTKModifiers()
        {
            int out = 0;
            if (Fl::event_key(FL_Shift_L) || Fl::event_key(FL_Shift_R))
            {
                out |= static_cast<int>(ui::KeyModifier::Shift);
            }
            if (Fl::event_key(FL_Control_L) || Fl::event_key(FL_Control_R))
            {
                out |= static_cast<int>(ui::KeyModifier::Control);
            }
            if (Fl::event_key(FL_Alt_L) || Fl::event_key(FL_Alt_R))
            {
                out |= static_cast<int>(ui::KeyModifier::Alt);
            }
            return out;
        }
    } // namespace

    int TimelineWidget::mousePressEvent(int button, bool on, int modifiers)
    {
        TLRENDER_P();
        bool send = App::ui->uiPrefs->SendTimeline->value();
        if (send)
        {
            Message message;
            message["command"] = "Timeline Mouse Press";
            message["button"] = button;
            message["on"] = on;
            message["modifiers"] = modifiers;
            tcp->pushMessage(message);
        }
        if (p.dragging)
        {
            makePathsAbsolute(p.player, p.ui);
        }
        p.eventLoop->mouseButton(button, on, modifiers);
        return 1;
    }

    int TimelineWidget::mousePressEvent()
    {
        TLRENDER_P();
        take_focus();
        int button = 0;
        int modifiers = fromFLTKModifiers();
        if (Fl::event_button1())
        {
            button = 0;
            _seek();
        }
        else if (Fl::event_button2())
        {
            button = 0;
            modifiers = static_cast<int>(ui::KeyModifier::Control);
        }
        else
        {
            return 0;
        }

        mousePressEvent(button, true, modifiers);
        return 1;
    }

    int TimelineWidget::mouseDragEvent(const int X, const int Y)
    {
        TLRENDER_P();
        if (Fl::event_button1())
        {
            _seek();
        }
        else if (Fl::event_button2())
        {
            // left empty on purpose
        }
        else
        {
            return 0;
        }
        mouseMoveEvent(X, Y);
        return 1;
    }

    int TimelineWidget::mouseReleaseEvent(
        const int X, const int Y, int button, bool on, int modifiers)
    {
        TLRENDER_P();
        if (button == 0)
        {
            _seek();
        }
        mouseMoveEvent(X, Y);
        mousePressEvent(button, on, modifiers);
        if (p.dragging)
        {
            toOtioFile(p.player, p.ui);
            p.ui->uiView->redrawWindows();
            redrawPanelThumbnails();
            p.dragging = false;
        }
        bool send = App::ui->uiPrefs->SendTimeline->value();
        if (send)
        {
            Message message;
            message["command"] = "Timeline Mouse Release";
            message["X"] = static_cast<float>(_toUI(X)) / pixel_w();
            message["Y"] = static_cast<float>(_toUI(Y)) / pixel_h();
            message["button"] = button;
            message["on"] = on;
            message["modifiers"] = modifiers;
            tcp->pushMessage(message);
        }
        return 1;
    }

    int TimelineWidget::mouseReleaseEvent()
    {
        int button = 0;
        mouseReleaseEvent(
            Fl::event_x(), Fl::event_y(), button, false, fromFLTKModifiers());
        return 1;
    }

    int TimelineWidget::mouseMoveEvent()
    {
        TLRENDER_P();
        mouseMoveEvent(Fl::event_x(), Fl::event_y());
        return 1;
    }

    void TimelineWidget::mouseMoveEvent(const int X, const int Y)
    {
        TLRENDER_P();
        bool send = App::ui->uiPrefs->SendTimeline->value();
        if (send)
        {
            Message message;
            message["command"] = "Timeline Mouse Move";
            message["X"] = static_cast<float>(_toUI(X)) / pixel_w();
            message["Y"] = static_cast<float>(_toUI(Y)) / pixel_h();
            tcp->pushMessage(message);
        }
        p.eventLoop->cursorPos(math::Vector2i(_toUI(X), _toUI(Y)));
    }

    void
    TimelineWidget::scrollEvent(const float X, const float Y, int modifiers)
    {
        TLRENDER_P();
        math::Vector2f pos(X, Y);
        p.eventLoop->scroll(pos, modifiers);

        Message message;
        message["command"] = "Timeline Widget Scroll";
        message["X"] = X;
        message["Y"] = Y;
        message["modifiers"] = modifiers;
        bool send = App::ui->uiPrefs->SendTimeline->value();
        if (send)
            tcp->pushMessage(message);
    }

    int TimelineWidget::wheelEvent()
    {
        TLRENDER_P();
        const auto now = std::chrono::steady_clock::now();
        const auto diff = std::chrono::duration<float>(now - p.mouseWheelTimer);
        const float delta = Fl::event_dy() / 8.F / 15.F;
        p.mouseWheelTimer = now;
        scrollEvent(Fl::event_dx() / 8.F / 15.F, -delta, fromFLTKModifiers());
        return 1;
    }

    namespace
    {
        ui::Key fromFLTKKey(unsigned key)
        {
            ui::Key out = ui::Key::Unknown;

#if defined(FLTK_USE_WAYLAND)
            if (key >= 'A' && key <= 'Z')
            {
                key = tolower(key);
            }
#endif
            switch (key)
            {
            case ' ':
                out = ui::Key::Space;
                break;
            case '\'':
                out = ui::Key::Apostrophe;
                break;
            case ',':
                out = ui::Key::Comma;
                break;
            case '-':
                out = ui::Key::Minus;
                break;
            case '.':
                out = ui::Key::Period;
                break;
            case '/':
                out = ui::Key::Slash;
                break;
            case '0':
                out = ui::Key::_0;
                break;
            case '1':
                out = ui::Key::_1;
                break;
            case '2':
                out = ui::Key::_2;
                break;
            case '3':
                out = ui::Key::_3;
                break;
            case '4':
                out = ui::Key::_4;
                break;
            case '5':
                out = ui::Key::_5;
                break;
            case '6':
                out = ui::Key::_6;
                break;
            case '7':
                out = ui::Key::_7;
                break;
            case '8':
                out = ui::Key::_8;
                break;
            case '9':
                out = ui::Key::_9;
                break;
            case ';':
                out = ui::Key::Semicolon;
                break;
            case '=':
                out = ui::Key::Equal;
                break;
            case 'a':
                out = ui::Key::A;
                break;
            case 'b':
                out = ui::Key::B;
                break;
            case 'c':
                out = ui::Key::C;
                break;
            case 'd':
                out = ui::Key::D;
                break;
            case 'e':
                out = ui::Key::E;
                break;
            case 'f':
                out = ui::Key::F;
                break;
            case 'g':
                out = ui::Key::G;
                break;
            case 'h':
                out = ui::Key::H;
                break;
            case 'i':
                out = ui::Key::I;
                break;
            case 'j':
                out = ui::Key::J;
                break;
            case 'k':
                out = ui::Key::K;
                break;
            case 'l':
                out = ui::Key::L;
                break;
            case 'm':
                out = ui::Key::M;
                break;
            case 'n':
                out = ui::Key::N;
                break;
            case 'o':
                out = ui::Key::O;
                break;
            case 'p':
                out = ui::Key::P;
                break;
            case 'q':
                out = ui::Key::Q;
                break;
            case 'r':
                out = ui::Key::R;
                break;
            case 's':
                out = ui::Key::S;
                break;
            case 't':
                out = ui::Key::T;
                break;
            case 'u':
                out = ui::Key::U;
                break;
            case 'v':
                out = ui::Key::V;
                break;
            case 'w':
                out = ui::Key::W;
                break;
            case 'x':
                out = ui::Key::X;
                break;
            case 'y':
                out = ui::Key::Y;
                break;
            case 'z':
                out = ui::Key::Z;
                break;
            case '[':
                out = ui::Key::LeftBracket;
                break;
            case '\\':
                out = ui::Key::Backslash;
                break;
            case ']':
                out = ui::Key::RightBracket;
                break;
            case 0xfe51: // @todo: is there a Fl_ shortcut for this?
                out = ui::Key::GraveAccent;
                break;
            case FL_Escape:
                out = ui::Key::Escape;
                break;
            case FL_Enter:
                out = ui::Key::Enter;
                break;
            case FL_Tab:
                out = ui::Key::Tab;
                break;
            case FL_BackSpace:
                out = ui::Key::Backspace;
                break;
            case FL_Insert:
                out = ui::Key::Insert;
                break;
            case FL_Delete:
                out = ui::Key::Delete;
                break;
            case FL_Right:
                out = ui::Key::Right;
                break;
            case FL_Left:
                out = ui::Key::Left;
                break;
            case FL_Down:
                out = ui::Key::Down;
                break;
            case FL_Up:
                out = ui::Key::Up;
                break;
            case FL_Page_Up:
                out = ui::Key::PageUp;
                break;
            case FL_Page_Down:
                out = ui::Key::PageDown;
                break;
            case FL_Home:
                out = ui::Key::Home;
                break;
            case FL_End:
                out = ui::Key::End;
                break;
            case FL_Caps_Lock:
                out = ui::Key::CapsLock;
                break;
            case FL_Scroll_Lock:
                out = ui::Key::ScrollLock;
                break;
            case FL_Num_Lock:
                out = ui::Key::NumLock;
                break;
            case FL_Print:
                out = ui::Key::PrintScreen;
                break;
            case FL_Pause:
                out = ui::Key::Pause;
                break;
            case FL_F + 1:
                out = ui::Key::F1;
                break;
            case FL_F + 2:
                out = ui::Key::F2;
                break;
            case FL_F + 3:
                out = ui::Key::F3;
                break;
            case FL_F + 4:
                out = ui::Key::F4;
                break;
            case FL_F + 5:
                out = ui::Key::F5;
                break;
            case FL_F + 6:
                out = ui::Key::F6;
                break;
            case FL_F + 7:
                out = ui::Key::F7;
                break;
            case FL_F + 8:
                out = ui::Key::F8;
                break;
            case FL_F + 9:
                out = ui::Key::F9;
                break;
            case FL_F + 10:
                out = ui::Key::F10;
                break;
            case FL_F + 11:
                out = ui::Key::F11;
                break;
            case FL_F + 12:
                out = ui::Key::F12;
                break;
            case FL_Shift_L:
                out = ui::Key::LeftShift;
                break;
            case FL_Control_L:
                out = ui::Key::LeftControl;
                break;
            case FL_Alt_L:
                out = ui::Key::LeftAlt;
                break;
            case FL_Meta_L:
                out = ui::Key::LeftSuper;
                break;
            case FL_Meta_R:
                out = ui::Key::RightSuper;
                break;
            }
            return out;
        }
    } // namespace

    void TimelineWidget::frameView()
    {
        TLRENDER_P();
        unsigned key = _changeKey(kFitScreen.hotkey());
        if (p.player)
        {
            auto innerPlayer = p.player->player();
            p.timeRange = innerPlayer->getTimeRange();
        }
        p.eventLoop->key(fromFLTKKey(key), true, 0);
        bool send = App::ui->uiPrefs->SendTimeline->value();
        if (send)
        {
            Message message;
            message["command"] = "Timeline Fit";
            tcp->pushMessage(message);
        }
    }

    int TimelineWidget::keyPressEvent(unsigned key, const int modifiers)
    {
        TLRENDER_P();
        // First, check if it is one of the menu shortcuts
        int ret = p.ui->uiMenuBar->handle(FL_SHORTCUT);
        if (ret)
            return ret;
        if (kToggleEditMode.match(key))
        {
            p.ui->uiEdit->do_callback();
            return 1;
        }
        bool send = App::ui->uiPrefs->SendTimeline->value();
        if (send)
        {
            Message message;
            message["command"] = "Timeline Key Press";
            message["value"] = key;
            message["modifiers"] = modifiers;
            tcp->pushMessage(message);
        }

        key = _changeKey(key);
        p.eventLoop->key(fromFLTKKey(key), true, modifiers);
        return 1;
    }

    int TimelineWidget::keyPressEvent()
    {
        TLRENDER_P();
        unsigned key = Fl::event_key();
        keyPressEvent(key, fromFLTKModifiers());
        return 1;
    }

    int TimelineWidget::keyReleaseEvent(unsigned key, const int modifiers)
    {
        TLRENDER_P();
        bool send = App::ui->uiPrefs->SendTimeline->value();
        if (send)
        {
            Message message;
            message["command"] = "Timeline Key Release";
            message["value"] = key;
            message["modifiers"] = modifiers;
            tcp->pushMessage(message);
        }
        key = _changeKey(key);
        p.eventLoop->key(fromFLTKKey(key), false, modifiers);
        return 1;
    }

    int TimelineWidget::keyReleaseEvent()
    {
        TLRENDER_P();
        keyReleaseEvent(Fl::event_key(), fromFLTKModifiers());
        return 1;
    }

    void TimelineWidget::timerEvent_cb(void* d)
    {
        TimelineWidget* o = static_cast<TimelineWidget*>(d);
        o->timerEvent();
    }

    void TimelineWidget::timerEvent()
    {
        TLRENDER_P();
        p.eventLoop->tick();
        if (p.eventLoop->hasDrawUpdate())
        {
            redraw();
        }
        Fl::repeat_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
    }

    int TimelineWidget::handle(int event)
    {
        TLRENDER_P();
        switch (event)
        {
        case FL_FOCUS:
        case FL_UNFOCUS:
            return 1;
        case FL_ENTER:
            cursor(FL_CURSOR_DEFAULT);
            if (p.thumbnailWindow &&
                p.ui->uiPrefs->uiPrefsTimelineThumbnails->value())
            {
                p.thumbnailWindow->show();
                _requestThumbnail();
            }
            return enterEvent();
        case FL_LEAVE:
            if (p.ui->uiPrefs->uiPrefsTimelineThumbnails->value())
            {
                if (p.thumbnailCreator && p.thumbnailRequestId)
                    p.thumbnailCreator->cancelRequests(p.thumbnailRequestId);
                if (!Fl::has_timeout(
                        (Fl_Timeout_Handler)hideThumbnail_cb, this))
                {
                    Fl::add_timeout(
                        0.005, (Fl_Timeout_Handler)hideThumbnail_cb, this);
                }
            }
            return leaveEvent();
        case FL_PUSH:
            if (p.ui->uiPrefs->uiPrefsTimelineThumbnails->value())
                _requestThumbnail(true);
            return mousePressEvent();
        case FL_DRAG:
            return mouseDragEvent(Fl::event_x(), Fl::event_y());
        case FL_RELEASE:
            redrawPanelThumbnails();
            return mouseReleaseEvent();
        case FL_MOVE:
            _requestThumbnail();
            return mouseMoveEvent();
        case FL_MOUSEWHEEL:
            return wheelEvent();
        case FL_KEYDOWN:
        {
            // @todo: ask darby for a return code from key press
            // int ret = p.ui->uiView->handle(event);
            return keyPressEvent();
        }
        case FL_KEYUP:
            return keyReleaseEvent();
        case FL_HIDE:
        {
            if (p.ui->uiPrefs->uiPrefsTimelineThumbnails->value())
            {
                if (p.thumbnailCreator && p.thumbnailRequestId)
                    p.thumbnailCreator->cancelRequests(p.thumbnailRequestId);
                if (!Fl::has_timeout(
                        (Fl_Timeout_Handler)hideThumbnail_cb, this))
                {
                    Fl::add_timeout(
                        0.005, (Fl_Timeout_Handler)hideThumbnail_cb, this);
                }
            }
            refresh();
            valid(0);
            return Fl_Gl_Window::handle(event);
        }
        }
        int out = Fl_Gl_Window::handle(event);
        // if (event->type() == QEvent::StyleChange)
        // {
        //     _styleUpdate();
        // }
        return out;
    }

    int TimelineWidget::_toUI(int value) const
    {
        TimelineWidget* self = const_cast<TimelineWidget*>(this);
        const float devicePixelRatio = self->pixels_per_unit();
        return value * devicePixelRatio;
    }

    math::Vector2i TimelineWidget::_toUI(const math::Vector2i& value) const
    {
        TimelineWidget* self = const_cast<TimelineWidget*>(this);
        const float devicePixelRatio = self->pixels_per_unit();
        return value * devicePixelRatio;
    }

    int TimelineWidget::_fromUI(int value) const
    {
        TimelineWidget* self = const_cast<TimelineWidget*>(this);
        const float devicePixelRatio = self->pixels_per_unit();
        return devicePixelRatio > 0.F ? (value / devicePixelRatio) : 0.F;
    }

    math::Vector2i TimelineWidget::_fromUI(const math::Vector2i& value) const
    {
        TimelineWidget* self = const_cast<TimelineWidget*>(this);
        const float devicePixelRatio = self->pixels_per_unit();
        return devicePixelRatio > 0.F ? (value / devicePixelRatio)
                                      : math::Vector2i();
    }

    //! Routine to turn mrv2's hotkeys into Darby's shortcuts
    unsigned TimelineWidget::_changeKey(unsigned key)
    {
        if (key == kFitScreen.hotkey())
            key = '0'; // Darby uses 0 to frame view
        else if (key == kFitAll.hotkey())
            key = '0'; // Darby uses 0 to frame view
        return key;
    }

    //! Draw annotation marks on timeline
    void TimelineWidget::_drawAnnotationMarks() const noexcept
    {
        TLRENDER_P();

        const auto& duration = p.timeRange.duration();
        const auto& color = image::Color4f(1, 1, 0, 0.25);
        TimelineWidget* self = const_cast<TimelineWidget*>(this);
        const float devicePixelRatio = self->pixels_per_unit();
        const auto& times = p.player->getAnnotationTimes();
        for (const auto time : times)
        {
            double X = _timeToPos(time::floor(time));
            math::Box2i bbox(X, 0, 2, 20 * devicePixelRatio);
            p.render->drawRect(bbox, color);
        }
    }

    int
    TimelineWidget::_timeToPos(const otime::RationalTime& value) const noexcept
    {
        TLRENDER_P();
        if (!p.player || !p.timelineWidget)
            return 0;

        const math::Box2i& geometry =
            p.timelineWidget->getTimelineItemGeometry();
        const double scale = p.timelineWidget->getScale();
        const otime::RationalTime t = value - p.timeRange.start_time();
        return geometry.min.x + t.rescaled_to(1.0).value() * scale;
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

    otime::RationalTime TimelineWidget::_posToTime(float value) const noexcept
    {
        TLRENDER_P();

        otime::RationalTime out = time::invalidTime;
        if (p.player && p.timelineWidget)
        {
            const math::Box2i& geometry =
                p.timelineWidget->getTimelineItemGeometry();
            const double normalized =
                (value - geometry.min.x) / static_cast<double>(geometry.w());
            out = time::round(
                p.timeRange.start_time() +
                otime::RationalTime(
                    p.timeRange.duration().value() * normalized,
                    p.timeRange.duration().rate()));
            out = math::clamp(
                out, p.timeRange.start_time(),
                p.timeRange.end_time_inclusive());
        }
        return out;
    }

    void TimelineWidget::refresh()
    {
        TLRENDER_P();
        p.render.reset();
        p.buffer.reset();
        p.shader.reset();
        p.vbo.reset();
        p.vao.reset();
    }

    void TimelineWidget::setUnits(TimeUnits value)
    {
        TLRENDER_P();
        p.units = value;
        auto timeUnitsModel = _p->ui->app->timeUnitsModel();
        timeUnitsModel->setTimeUnits(
            static_cast<tl::timeline::TimeUnits>(value));
        TimelineClass* c = _p->ui->uiTimeWindow;
        c->uiStartFrame->setUnits(value);
        c->uiEndFrame->setUnits(value);
        c->uiFrame->setUnits(value);
        redraw();
    }

    void TimelineWidget::insertCallback(
        const std::vector<tl::timeline::InsertData>& inserts)
    {
        TLRENDER_P();
        edit_store_undo(p.player, p.ui);
        edit_clear_redo(p.ui);
        edit_insert_clip_annotations(inserts, p.ui);
    }

    void TimelineWidget::single_thumbnail(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
            thumbnails)
    {
        TLRENDER_P();

        if (id == p.thumbnailRequestId)
        {
            for (const auto& i : thumbnails)
            {
                Fl_Image* image = p.box->image();
                delete image;
                p.box->image(i.second);
            }
            p.box->redraw();
        }
        else
        {
            for (const auto& i : thumbnails)
            {
                delete i.second;
            }
        }
    }

    void TimelineWidget::single_thumbnail_cb(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
            thumbnails,
        void* data)
    {
        TimelineWidget* self = static_cast< TimelineWidget* >(data);
        self->single_thumbnail(id, thumbnails);
    }

} // namespace mrv

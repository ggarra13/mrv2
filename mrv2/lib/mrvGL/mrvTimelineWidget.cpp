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
#include <tlUI/RowLayout.h>

#include <tlTimeline/GLRender.h>

#include <tlGL/Init.h>

#include "mrvFl/mrvIO.h"

#include "mrvGL/mrvThumbnailCreator.h"
#include "mrvGL/mrvTimelineWidget.h"
#include "mrvGL/mrvGLErrors.h"

#include "mrViewer.h"

namespace mrv
{
    namespace
    {
        const double kTimeout = 0.01;
        const char* kModule = "timelineui";
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

        const ViewerUI* ui = nullptr;

        std::shared_ptr<timeline::Player> player;

        ThumbnailCreator* thumbnailCreator = nullptr;
        Fl_Double_Window* thumbnailWindow = nullptr; // thumbnail window
        int64_t thumbnailRequestId = 0;
        Fl_Box* box = nullptr;

        timeline::ColorConfigOptions colorConfigOptions;
        timeline::LUTOptions lutOptions;

        mrv::TimeUnits units = mrv::TimeUnits::Timecode;

        std::shared_ptr<ui::Style> style;
        std::shared_ptr<ui::IconLibrary> iconLibrary;
        std::shared_ptr<imaging::FontSystem> fontSystem;
        std::shared_ptr<Clipboard> clipboard;
        std::shared_ptr<timeline::IRender> render;
        std::shared_ptr<ui::EventLoop> eventLoop;
        timelineui::ItemOptions itemOptions;
        std::shared_ptr<timelineui::TimelineWidget> timelineWidget;
        std::chrono::steady_clock::time_point mouseWheelTimer;

        otime::TimeRange timeRange = time::invalidTimeRange;
    };

    TimelineWidget::TimelineWidget(int X, int Y, int W, int H, const char* L) :
        Fl_Gl_Window(X, Y, W, H, L),
        _p(new Private)
    {
        int fl_double = FL_DOUBLE;
#ifdef __APPLE__
        fl_double = 0; // @bug: Apple flickers when window is double.
#endif
        mode(FL_RGB | FL_ALPHA | fl_double | FL_STENCIL | FL_OPENGL3);
    }

    void TimelineWidget::setContext(
        const std::shared_ptr<system::Context>& context,
        const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
        const ViewerUI* ui)
    {
        TLRENDER_P();

        p.context = context;

        p.ui = ui;

        p.style = ui::Style::create(context);
        p.iconLibrary = ui::IconLibrary::create(context);
        p.fontSystem = imaging::FontSystem::create(context);
        p.clipboard = Clipboard::create(context);
        p.eventLoop = ui::EventLoop::create(
            p.style, p.iconLibrary, p.fontSystem, p.clipboard, context);
        p.timelineWidget =
            timelineui::TimelineWidget::create(timeUnitsModel, context);
        // p.timelineWidget->setScrollBarsVisible(false);
        p.eventLoop->addWidget(p.timelineWidget);

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

    int TimelineWidget::_requestThumbnail(bool fetch)
    {
        TLRENDER_P();
        const auto& player = p.player;
        if (!player)
            return 0;

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

        const auto path = player->getPath();
        imaging::Size size(p.box->w(), p.box->h() - 24);
        const auto& time = _posToTime(Fl::event_x() - x());

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

    void
    TimelineWidget::setPlayer(const std::shared_ptr<timeline::Player>& player)
    {
        TLRENDER_P();
        if (player == p.player)
            return;
        p.player = player;
        if (player)
            p.timeRange = player->getTimeRange();
        else
            p.timeRange = time::invalidTimeRange;
        p.timelineWidget->setPlayer(p.player);
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

    // @todo: do we need to do anything here?
    void TimelineWidget::frameViewChanged(bool value) {}

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

    void TimelineWidget::initializeGL()
    {
        TLRENDER_P();
        gl::initGLAD();
        if (auto context = p.context.lock())
        {
            p.render = timeline::GLRender::create(context);
        }
    }

    void TimelineWidget::resize(int X, int Y, int W, int H)
    {
        TLRENDER_P();

        assert(X >= 0);
        assert(Y >= 0);
        assert(W > 0);
        assert(H > 0);

        // std::cerr << "resize " << X << " " << Y << " " << W << "x" << H
        //           << std::endl;

        Fl_Gl_Window::resize(X, Y, W, H);

        if (p.eventLoop)
        {
            const float devicePixelRatio = pixels_per_unit();
            p.eventLoop->setDisplayScale(devicePixelRatio);
            p.eventLoop->setDisplaySize(imaging::Size(_toUI(W), _toUI(H)));
        }
    }

    void TimelineWidget::draw()
    {
        TLRENDER_P();

        if (!valid())
        {
            initializeGL();
            CHECK_GL;

            const float devicePixelRatio = pixels_per_unit();
            p.eventLoop->setDisplayScale(devicePixelRatio);
            p.eventLoop->setDisplaySize(imaging::Size(_toUI(w()), _toUI(h())));
            CHECK_GL;

            valid(1);
        }

        if (p.render)
        {
            try
            {
                make_current();
                timeline::RenderOptions renderOptions;
                renderOptions.clearColor =
                    p.style->getColorRole(ui::ColorRole::Window);
                p.render->begin(
                    imaging::Size(_toUI(w()), _toUI(h())),
                    timeline::ColorConfigOptions(), timeline::LUTOptions(),
                    renderOptions);
                CHECK_GL;
                p.eventLoop->draw(p.render);
                CHECK_GL;
                _drawAnnotationMarks();
                CHECK_GL;
                p.render->end();
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }
        }
    }

    int TimelineWidget::enterEvent()
    {
        TLRENDER_P();
        p.eventLoop->cursorEnter(true);
        return 1;
    }

    int TimelineWidget::leaveEvent()
    {
        TLRENDER_P();
        p.eventLoop->cursorEnter(false);
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

    int TimelineWidget::mousePressEvent()
    {
        TLRENDER_P();
        take_focus();
        int button = 0;
        int modifiers = fromFLTKModifiers();
        if (Fl::event_button1())
        {
            button = 1;
        }
        else if (Fl::event_button2())
        {
            button = 1;
            modifiers = static_cast<int>(ui::KeyModifier::Control);
        }
        p.eventLoop->mouseButton(button, true, modifiers);
        return 1;
    }

    int TimelineWidget::mouseDragEvent()
    {
        TLRENDER_P();
        p.eventLoop->cursorPos(
            math::Vector2i(_toUI(Fl::event_x()), _toUI(Fl::event_y())));
        return 1;
    }

    int TimelineWidget::mouseReleaseEvent()
    {
        TLRENDER_P();
        int button = 0;
        if (Fl::event_button1())
        {
            button = 1;
        }
        p.eventLoop->cursorPos(
            math::Vector2i(_toUI(Fl::event_x()), _toUI(Fl::event_y())));
        p.eventLoop->mouseButton(button, false, fromFLTKModifiers());
        return 1;
    }

    int TimelineWidget::mouseMoveEvent()
    {
        TLRENDER_P();
        p.eventLoop->cursorPos(
            math::Vector2i(_toUI(Fl::event_x()), _toUI(Fl::event_y())));
        return 1;
    }

    int TimelineWidget::wheelEvent()
    {
        TLRENDER_P();
        const auto now = std::chrono::steady_clock::now();
        const auto diff = std::chrono::duration<float>(now - p.mouseWheelTimer);
        const float delta = Fl::event_dy() / 8.F / 15.F;
        p.mouseWheelTimer = now;
        p.eventLoop->scroll(
            Fl::event_dx() / 8.F / 15.F, -Fl::event_dy() / 8.F / 15.F,
            fromFLTKModifiers());
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

    int TimelineWidget::keyPressEvent()
    {
        TLRENDER_P();
        unsigned key = Fl::event_key();

        // First, check if it is one of the menu shortcuts
        // int ret = p.ui->uiMenuBar->handle(FL_SHORTCUT);
        // if (ret)
        //     return ret;

        key = _changeKey(key);
        p.eventLoop->key(fromFLTKKey(key), true, fromFLTKModifiers());
        return 1;
    }

    int TimelineWidget::keyReleaseEvent()
    {
        TLRENDER_P();
        unsigned key = _changeKey(Fl::event_key());
        p.eventLoop->key(fromFLTKKey(key), false, fromFLTKModifiers());
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
        // if (event != FL_NO_EVENT && event != FL_MOVE)
        //     std::cerr << fl_eventnames[event] << " active=" << active()
        //               << std::endl;
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
        case FL_HIDE:
            if (p.thumbnailCreator && p.thumbnailRequestId)
                p.thumbnailCreator->cancelRequests(p.thumbnailRequestId);
            if (!Fl::has_timeout((Fl_Timeout_Handler)hideThumbnail_cb, this))
            {
                Fl::add_timeout(
                    0.005, (Fl_Timeout_Handler)hideThumbnail_cb, this);
            }
            return leaveEvent();
        case FL_PUSH:
            if (p.ui->uiPrefs->uiPrefsTimelineThumbnails->value())
                _requestThumbnail(true);
            return mousePressEvent();
        case FL_DRAG:
            return mouseDragEvent();
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
        if (key == 'f')
            key = '0'; // Darby uses 0 to frame view
        else if (key == 'a')
            key = '0'; // Darby uses 0 to frame view
        return key;
    }

    //! Draw annotation marks on timeline
    void TimelineWidget::_drawAnnotationMarks() const noexcept
    {
        TLRENDER_P();

        auto player = p.ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const int H = 20;
        const int Y = 0;
        const auto& frames = player->getAnnotationFrames();
        if (frames.empty())
            return;

        p.timeRange = p.timelineWidget->timeRange();

        const auto& duration =
            p.timeRange.end_time_inclusive() - p.timeRange.start_time();
        const auto& color = imaging::Color4f(0, 1, 1, 0.25);
        for (const auto frame : frames)
        {
            otime::RationalTime time(frame, duration.rate());
            double X = _timeToPos(time);
            math::BBox2i bbox(X - 0.5, 0, 2, 20);
            p.render->drawRect(bbox, color);
        }
    }

    int
    TimelineWidget::_timeToPos(const otime::RationalTime& value) const noexcept
    {
        TLRENDER_P();
        int out = 0;
        if (p.player && p.timelineWidget)
        {
            p.timeRange = p.timelineWidget->timeRange();
            if (!time::compareExact(value, time::invalidTime) &&
                p.timeRange.duration().value() > 0.0)
            {
                const float normalized =
                    (value.value() - p.timeRange.start_time().value()) /
                    p.timeRange.duration().value();
                out = x() + normalized * w();
            }
        }
        return out;
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

    otime::RationalTime TimelineWidget::_posToTime(int value) const noexcept
    {
        TLRENDER_P();

        otime::RationalTime out = time::invalidTime;
        if (p.player && p.timelineWidget)
        {
            p.timeRange = p.timelineWidget->timeRange();
            const double normalized = (value - x()) / static_cast<double>(w());
            const int width = w();
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

    void TimelineWidget::main(ViewerUI* ui)
    {
        _p->ui = ui;
    }
} // namespace mrv

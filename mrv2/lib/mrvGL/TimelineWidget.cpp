// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <mrvGL/TimelineWidget.h>

#include <tlTimelineUI/TimelineWidget.h>

#include <tlUI/EventLoop.h>
#include <tlUI/IClipboard.h>
#include <tlUI/RowLayout.h>

#include <tlTimeline/GLRender.h>

#include <tlGL/Init.h>

namespace mrv
{
    namespace
    {
        const double kTimeout = 0.01;
    }

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

        TimeObject* timeObject = nullptr;
        std::shared_ptr<timeline::Player> player;

        std::shared_ptr<ui::Style> style;
        std::shared_ptr<ui::IconLibrary> iconLibrary;
        std::shared_ptr<imaging::FontSystem> fontSystem;
        std::shared_ptr<Clipboard> clipboard;
        std::shared_ptr<timeline::IRender> render;
        std::shared_ptr<ui::EventLoop> eventLoop;
        timelineui::ItemOptions itemOptions;
        std::shared_ptr<timelineui::TimelineWidget> timelineWidget;
        std::chrono::steady_clock::time_point mouseWheelTimer;
    };

    TimelineWidget::TimelineWidget(int X, int Y, int W, int H, const char* L) :
        Fl_Gl_Window(X, Y, W, H, L),
        _p(new Private)
    {
    }

    void TimelineWidget::setContext(
        const std::shared_ptr<system::Context>& context,
        const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel)
    {
        TLRENDER_P();

        p.context = context;

        p.style = ui::Style::create(context);
        p.iconLibrary = ui::IconLibrary::create(context);
        p.fontSystem = imaging::FontSystem::create(context);
        p.clipboard = Clipboard::create(context);
        p.eventLoop = ui::EventLoop::create(
            p.style, p.iconLibrary, p.fontSystem, p.clipboard, context);
        p.timelineWidget =
            timelineui::TimelineWidget::create(timeUnitsModel, context);
        p.timelineWidget->setFrameViewCallback([this](bool value)
                                               { frameViewChanged(value); });
        // p.timelineWidget->setScrollBarsVisible(false);
        p.eventLoop->addWidget(p.timelineWidget);

        _styleUpdate();

        Fl::add_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
    }

    void TimelineWidget::setStyle(const std::shared_ptr<ui::Style>& style)
    {
        _p->style = style;
        _styleUpdate();
    }

    TimelineWidget::~TimelineWidget() {}

#if 0
    // @todo:
    void TimelineWidget::setTimeObject(qt::TimeObject * timeObject)
    {
        TLRENDER_P();
        if (timeObject == p.timeObject)
            return;
        if (p.timeObject)
        {
            disconnect(
                p.timeObject,
                SIGNAL(timeUnitsChanged(tl::timeline::TimeUnits)),
                this,
                SLOT(_setTimeUnits(tl::timeline::TimeUnits)));
        }
        p.timeObject = timeObject;
        if (p.timeObject)
        {
            p.itemOptions.timeUnits = p.timeObject->timeUnits();
            p.timelineWidget->setItemOptions(p.itemOptions);
            connect(
                p.timeObject,
                SIGNAL(timeUnitsChanged(tl::timeline::TimeUnits)),
                SLOT(_setTimeUnits(tl::timeline::TimeUnits)));
        }
    }
#endif

    void
    TimelineWidget::setPlayer(const std::shared_ptr<timeline::Player>& player)
    {
        TLRENDER_P();
        if (player == p.player)
            return;
        p.player = player;
        p.timelineWidget->setPlayer(p.player);
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
        const float devicePixelRatio = pixels_per_unit();
        p.eventLoop->setDisplayScale(devicePixelRatio);
        p.eventLoop->setDisplaySize(imaging::Size(_toUI(W), _toUI(H)));
    }

    void TimelineWidget::draw()
    {
        TLRENDER_P();
        if (!valid())
        {
            initializeGL();
            valid(1);
        }
        if (p.render)
        {
            timeline::RenderOptions renderOptions;
            renderOptions.clearColor =
                p.style->getColorRole(ui::ColorRole::Window);
            p.render->begin(
                imaging::Size(_toUI(w()), _toUI(h())),
                timeline::ColorConfigOptions(), timeline::LUTOptions(),
                renderOptions);
            p.eventLoop->draw(p.render);
            p.render->end();
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
        if (Fl::event_button1())
        {
            button = 1;
        }
        p.eventLoop->mouseButton(button, true, fromFLTKModifiers());
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
            Fl::event_dx() / 8.F / 15.F, Fl::event_dy() / 8.F / 15.F);
        return 1;
    }

    namespace
    {
        ui::Key fromFLTKKey(unsigned key)
        {
            ui::Key out = ui::Key::Unknown;
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
                // case '´': out = ui::Key::GraveAccent; break; // @todo:
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
        p.eventLoop->key(
            fromFLTKKey(Fl::event_key()), true, fromFLTKModifiers());
        return 1;
    }

    int TimelineWidget::keyReleaseEvent()
    {
        TLRENDER_P();
        p.eventLoop->key(
            fromFLTKKey(Fl::event_key()), false, fromFLTKModifiers());
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
            return enterEvent();
        case FL_LEAVE:
            return leaveEvent();
        case FL_PUSH:
            return mousePressEvent();
        case FL_RELEASE:
            return mouseReleaseEvent();
        case FL_MOVE:
            return mouseMoveEvent();
        case FL_MOUSEWHEEL:
            return wheelEvent();
        case FL_KEYDOWN:
            return keyPressEvent();
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
} // namespace mrv

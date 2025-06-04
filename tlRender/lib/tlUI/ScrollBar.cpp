// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/ScrollBar.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct ScrollBar::Private
        {
            Orientation orientation = Orientation::Horizontal;
            int scrollSize = 0;
            int scrollPos = 0;
            std::function<void(int)> scrollPosCallback;

            struct SizeData
            {
                bool sizeInit = true;
                int border = 0;
                int handle = 0;
            };
            SizeData size;

            struct MouseData
            {
                int pressedScrollPos = 0;
            };
            MouseData mouse;
        };

        void ScrollBar::_init(
            Orientation orientation,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ScrollBar", context, parent);
            TLRENDER_P();
            setStretch(
                Orientation::Horizontal == orientation ? Stretch::Expanding
                                                       : Stretch::Fixed,
                Orientation::Vertical == orientation ? Stretch::Expanding
                                                     : Stretch::Fixed);
            _setMouseHover(true);
            _setMousePress(true);
            p.orientation = orientation;
        }

        ScrollBar::ScrollBar() :
            _p(new Private)
        {
        }

        ScrollBar::~ScrollBar() {}

        std::shared_ptr<ScrollBar> ScrollBar::create(
            Orientation orientation,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ScrollBar>(new ScrollBar);
            out->_init(orientation, context, parent);
            return out;
        }

        void ScrollBar::setScrollSize(int value)
        {
            TLRENDER_P();
            if (value == p.scrollSize)
                return;
            p.scrollSize = value;
            _updates |= Update::Draw;
        }

        int ScrollBar::getScrollPos() const
        {
            return _p->scrollPos;
        }

        void ScrollBar::setScrollPos(int value)
        {
            TLRENDER_P();
            if (value == p.scrollPos)
                return;
            p.scrollPos = value;
            _updates |= Update::Draw;
        }

        void
        ScrollBar::setScrollPosCallback(const std::function<void(int)>& value)
        {
            _p->scrollPosCallback = value;
        }

        void ScrollBar::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged =
                event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.border =
                    event.style->getSizeRole(SizeRole::Border, _displayScale);
                p.size.handle =
                    event.style->getSizeRole(SizeRole::Handle, _displayScale);
            }
            p.size.sizeInit = false;

            _sizeHint.w = p.size.handle;
            _sizeHint.h = p.size.handle;
            switch (p.orientation)
            {
            case Orientation::Horizontal:
                _sizeHint.w += p.size.handle + p.size.border * 2;
                _sizeHint.h += p.size.border;
                break;
            case Orientation::Vertical:
                _sizeHint.w += p.size.border;
                _sizeHint.h += p.size.handle + p.size.border * 2;
                break;
            default:
                break;
            }
        }

        void ScrollBar::drawEvent(
            const math::Box2i& drawRect, const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i g = _getBorderGeometry();

            event.render->drawMesh(
                border(g, p.size.border), math::Vector2i(),
                event.style->getColorRole(ColorRole::Border));

            const int scrollPosMax = _getScrollPosMax();
            if (scrollPosMax > 0)
            {
                const math::Box2i g2 = _getHandleGeometry();
                event.render->drawRect(
                    g2, event.style->getColorRole(ColorRole::Button));

                if (_mouse.press)
                {
                    event.render->drawRect(
                        g2, event.style->getColorRole(ColorRole::Pressed));
                }
                else if (_mouse.inside)
                {
                    event.render->drawRect(
                        g2, event.style->getColorRole(ColorRole::Hover));
                }
            }
        }

        void ScrollBar::mouseEnterEvent()
        {
            IWidget::mouseEnterEvent();
            _updates |= Update::Draw;
        }

        void ScrollBar::mouseLeaveEvent()
        {
            IWidget::mouseLeaveEvent();
            _updates |= Update::Draw;
        }

        void ScrollBar::mouseMoveEvent(MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            TLRENDER_P();
            if (_mouse.press)
            {
                int scrollPos = 0;
                const float s = _getScrollScale();
                switch (p.orientation)
                {
                case Orientation::Horizontal:
                    scrollPos = p.mouse.pressedScrollPos +
                                (event.pos.x - _mouse.pressPos.x) * s;
                    break;
                case Orientation::Vertical:
                    scrollPos = p.mouse.pressedScrollPos +
                                (event.pos.y - _mouse.pressPos.y) * s;
                    break;
                default:
                    break;
                }
                const int scrollPosMax = _getScrollPosMax();
                const int scrollPosClamped =
                    math::clamp(scrollPos, 0, scrollPosMax);
                if (scrollPosClamped != p.scrollPos)
                {
                    p.scrollPos = scrollPosClamped;
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                    if (p.scrollPosCallback)
                    {
                        p.scrollPosCallback(p.scrollPos);
                    }
                }
            }
        }

        void ScrollBar::mousePressEvent(MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            const math::Box2i g = _getHandleGeometry();
            if (!g.contains(event.pos))
            {
                int scrollPos = 0;
                const float s = _getScrollScale();
                switch (p.orientation)
                {
                case Orientation::Horizontal:
                    scrollPos = (event.pos.x - g.w() / 2 - _geometry.min.x) * s;
                    break;
                case Orientation::Vertical:
                    scrollPos = (event.pos.y - g.h() / 2 - _geometry.min.y) * s;
                    break;
                default:
                    break;
                }
                const int scrollPosMax = _getScrollPosMax();
                const int scrollPosClamped =
                    math::clamp(scrollPos, 0, scrollPosMax);
                if (scrollPosClamped != p.scrollPos)
                {
                    p.scrollPos = scrollPosClamped;
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                    if (p.scrollPosCallback)
                    {
                        p.scrollPosCallback(p.scrollPos);
                    }
                }
            }
            p.mouse.pressedScrollPos = p.scrollPos;
            _updates |= Update::Draw;
        }

        void ScrollBar::mouseReleaseEvent(MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            _updates |= Update::Draw;
        }

        math::Box2i ScrollBar::_getBorderGeometry() const
        {
            TLRENDER_P();
            math::Box2i out;
            const math::Box2i& g = _geometry;
            switch (p.orientation)
            {
            case Orientation::Horizontal:
                out = g.margin(0, p.size.border, 0, 0);
                break;
            case Orientation::Vertical:
                out = g.margin(p.size.border, 0, 0, 0);
                break;
            default:
                break;
            }
            return out;
        }

        math::Box2i ScrollBar::_getHandleGeometry() const
        {
            TLRENDER_P();
            math::Box2i out;
            const math::Box2i g = _getBorderGeometry().margin(-p.size.border);
            switch (p.orientation)
            {
            case Orientation::Horizontal:
            {
                const int w = p.scrollSize - g.w();
                const int w2 = std::max(
                    static_cast<int>(
                        g.w() / static_cast<float>(p.scrollSize) * g.w()),
                    p.size.handle * 2);
                const int x =
                    p.scrollPos / static_cast<float>(w) * (g.w() - w2);
                out = math::Box2i(g.x() + x, g.y(), w2, g.h());
                break;
            }
            case Orientation::Vertical:
            {
                const int h = p.scrollSize - g.h();
                const int h2 = std::max(
                    static_cast<int>(
                        g.h() / static_cast<float>(p.scrollSize) * g.h()),
                    p.size.handle * 2);
                const int y =
                    p.scrollPos / static_cast<float>(h) * (g.h() - h2);
                out = math::Box2i(g.x(), g.y() + y, g.w(), h2);
                break;
            }
            default:
                break;
            }
            return out;
        }

        int ScrollBar::_getScrollPosMax() const
        {
            TLRENDER_P();
            int out = 0;
            const math::Box2i g = _getBorderGeometry().margin(-p.size.border);
            switch (p.orientation)
            {
            case Orientation::Horizontal:
                out = std::max(0, p.scrollSize - g.w() + 2);
                break;
            case Orientation::Vertical:
                out = std::max(0, p.scrollSize - g.h() + 2);
                break;
            default:
                break;
            }
            return out;
        }

        float ScrollBar::_getScrollScale() const
        {
            TLRENDER_P();
            float out = 0.F;
            const math::Box2i g = _getBorderGeometry().margin(-p.size.border);
            switch (p.orientation)
            {
            case Orientation::Horizontal:
                out = g.w() > 0 ? (p.scrollSize / static_cast<float>(g.w()))
                                : 0.F;
                break;
            case Orientation::Vertical:
                out = g.h() > 0 ? (p.scrollSize / static_cast<float>(g.h()))
                                : 0.F;
                break;
            default:
                break;
            }
            return out;
        }
    } // namespace ui
} // namespace tl

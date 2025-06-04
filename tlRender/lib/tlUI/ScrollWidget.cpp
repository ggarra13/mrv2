// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/ScrollWidget.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/GridLayout.h>
#include <tlUI/ScrollArea.h>
#include <tlUI/ScrollBar.h>

namespace tl
{
    namespace ui
    {
        struct ScrollWidget::Private
        {
            ScrollType scrollType = ScrollType::Both;
            bool scrollBarsVisible = true;
            bool scrollEventsEnabled = true;
            std::shared_ptr<IWidget> widget;
            std::shared_ptr<ScrollArea> scrollArea;
            std::shared_ptr<ScrollBar> horizontalScrollBar;
            std::shared_ptr<ScrollBar> verticalScrollBar;
            std::shared_ptr<GridLayout> layout;
            std::function<void(const math::Vector2i&)> scrollPosCallback;
        };

        void ScrollWidget::_init(
            const std::shared_ptr<system::Context>& context,
            ScrollType scrollType, const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ScrollWidget", context, parent);
            TLRENDER_P();

            p.scrollType = scrollType;

            p.scrollArea = ScrollArea::create(context, scrollType);
            p.scrollArea->setStretch(Stretch::Expanding);

            switch (scrollType)
            {
            case ScrollType::Horizontal:
                p.horizontalScrollBar =
                    ScrollBar::create(Orientation::Horizontal, context);
                break;
            case ScrollType::Vertical:
            case ScrollType::Menu:
                p.verticalScrollBar =
                    ScrollBar::create(Orientation::Vertical, context);
                break;
            case ScrollType::Both:
                p.horizontalScrollBar =
                    ScrollBar::create(Orientation::Horizontal, context);
                p.verticalScrollBar =
                    ScrollBar::create(Orientation::Vertical, context);
                break;
            }

            p.layout = GridLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::kNone);
            p.layout->setStretch(Stretch::Expanding);
            p.scrollArea->setParent(p.layout);
            p.layout->setGridPos(p.scrollArea, 0, 0);
            if (p.horizontalScrollBar)
            {
                p.horizontalScrollBar->setParent(p.layout);
                p.layout->setGridPos(p.horizontalScrollBar, 1, 0);
            }
            if (p.verticalScrollBar)
            {
                p.verticalScrollBar->setParent(p.layout);
                p.layout->setGridPos(p.verticalScrollBar, 0, 1);
            }

            if (p.horizontalScrollBar)
            {
                p.horizontalScrollBar->setScrollPosCallback(
                    [this](int value)
                    {
                        math::Vector2i scrollPos;
                        scrollPos.x = value;
                        if (_p->verticalScrollBar)
                        {
                            scrollPos.y = _p->verticalScrollBar->getScrollPos();
                        }
                        _p->scrollArea->setScrollPos(scrollPos);
                    });
            }

            if (p.verticalScrollBar)
            {
                p.verticalScrollBar->setScrollPosCallback(
                    [this](int value)
                    {
                        math::Vector2i scrollPos;
                        if (_p->horizontalScrollBar)
                        {
                            scrollPos.x =
                                _p->horizontalScrollBar->getScrollPos();
                        }
                        scrollPos.y = value;
                        _p->scrollArea->setScrollPos(scrollPos);
                    });
            }

            p.scrollArea->setScrollSizeCallback(
                [this](const math::Vector2i& value)
                {
                    if (_p->horizontalScrollBar)
                    {
                        _p->horizontalScrollBar->setScrollSize(value.x);
                    }
                    if (_p->verticalScrollBar)
                    {
                        _p->verticalScrollBar->setScrollSize(value.y);
                    }
                });

            p.scrollArea->setScrollPosCallback(
                [this](const math::Vector2i& value)
                {
                    if (_p->horizontalScrollBar)
                    {
                        _p->horizontalScrollBar->setScrollPos(value.x);
                    }
                    if (_p->verticalScrollBar)
                    {
                        _p->verticalScrollBar->setScrollPos(value.y);
                    }
                    if (_p->scrollPosCallback)
                    {
                        _p->scrollPosCallback(value);
                    }
                });
        }

        ScrollWidget::ScrollWidget() :
            _p(new Private)
        {
        }

        ScrollWidget::~ScrollWidget() {}

        std::shared_ptr<ScrollWidget> ScrollWidget::create(
            const std::shared_ptr<system::Context>& context,
            ScrollType scrollType, const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ScrollWidget>(new ScrollWidget);
            out->_init(context, scrollType, parent);
            return out;
        }

        void ScrollWidget::setWidget(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            if (p.widget)
            {
                p.widget->setParent(nullptr);
            }
            p.widget = value;
            if (p.widget)
            {
                p.widget->setParent(_p->scrollArea);
            }
        }

        math::Box2i ScrollWidget::getViewport() const
        {
            return _p->scrollArea->getChildrenClipRect();
        }

        const math::Vector2i& ScrollWidget::getScrollSize() const
        {
            return _p->scrollArea->getScrollSize();
        }

        const math::Vector2i& ScrollWidget::getScrollPos() const
        {
            return _p->scrollArea->getScrollPos();
        }

        void ScrollWidget::setScrollPos(const math::Vector2i& value, bool clamp)
        {
            _p->scrollArea->setScrollPos(value, clamp);
        }

        void ScrollWidget::setScrollPosCallback(
            const std::function<void(const math::Vector2i&)>& value)
        {
            _p->scrollPosCallback = value;
        }

        bool ScrollWidget::areScrollBarsVisible() const
        {
            return _p->horizontalScrollBar->isVisible() &&
                   _p->verticalScrollBar->isVisible();
        }

        void ScrollWidget::setScrollBarsVisible(bool value)
        {
            TLRENDER_P();
            if (value == p.scrollBarsVisible)
                return;
            p.scrollBarsVisible = value;
            p.horizontalScrollBar->setVisible(value);
            p.verticalScrollBar->setVisible(value);
        }

        void ScrollWidget::setScrollEventsEnabled(bool value)
        {
            _p->scrollEventsEnabled = value;
        }

        void ScrollWidget::setBorder(bool value)
        {
            _p->scrollArea->setBorder(value);
        }

        void ScrollWidget::setMarginRole(SizeRole value)
        {
            _p->layout->setMarginRole(value);
        }

        void ScrollWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ScrollWidget::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void ScrollWidget::scrollEvent(ScrollEvent& event)
        {
            IWidget::scrollEvent(event);
            TLRENDER_P();
            if (p.scrollEventsEnabled)
            {
                event.accept = true;
                math::Vector2i scrollPos = getScrollPos();
                scrollPos.y -= event.value.y * getLineStep();
                setScrollPos(scrollPos);
            }
        }

        void ScrollWidget::keyPressEvent(KeyEvent& event)
        {
            IWidget::keyPressEvent(event);
            TLRENDER_P();
            if (0 == event.modifiers)
            {
                switch (event.key)
                {
                case Key::PageUp:
                {
                    event.accept = true;
                    math::Vector2i scrollPos = getScrollPos();
                    scrollPos.y -= getPageStep();
                    setScrollPos(scrollPos);
                    break;
                }
                case Key::PageDown:
                {
                    event.accept = true;
                    math::Vector2i scrollPos = getScrollPos();
                    scrollPos.y += getPageStep();
                    setScrollPos(scrollPos);
                    break;
                }
                default:
                    break;
                }
            }
        }

        void ScrollWidget::keyReleaseEvent(KeyEvent& event)
        {
            IWidget::keyReleaseEvent(event);
            event.accept = true;
        }

        int ScrollWidget::getLineStep() const
        {
            TLRENDER_P();
            const math::Size2i scrollAreaSize =
                p.scrollArea->getGeometry().getSize();
            return scrollAreaSize.h / 10.F;
        }

        int ScrollWidget::getPageStep() const
        {
            TLRENDER_P();
            const math::Size2i scrollAreaSize =
                p.scrollArea->getGeometry().getSize();
            return scrollAreaSize.h;
        }
    } // namespace ui
} // namespace tl

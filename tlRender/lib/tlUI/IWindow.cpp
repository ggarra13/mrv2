// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/IWindow.h>

namespace tl
{
    namespace ui
    {
        struct IWindow::Private
        {
            math::Vector2i cursorPos;
            math::Vector2i cursorPosPrev;
            std::weak_ptr<IWidget> hover;
            std::weak_ptr<IWidget> mousePress;
            MouseClickEvent mouseClickEvent;
            std::weak_ptr<IWidget> keyFocus;
            std::weak_ptr<IWidget> keyPress;
            KeyEvent keyEvent;

            std::shared_ptr<DragAndDropData> dndData;
            std::shared_ptr<image::Image> dndCursor;
            math::Vector2i dndCursorHotspot;
            std::weak_ptr<IWidget> dndHover;

        };

        IWindow::IWindow() :
            _p(new Private)
        {
        }

        IWindow::~IWindow() {}

        void IWindow::setKeyFocus(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            if (value == p.keyFocus.lock())
                return;
            if (auto widget = p.keyFocus.lock())
            {
                widget->keyFocusEvent(false);
                _updates |= Update::Draw;
            }
            p.keyFocus = value;
            if (auto widget = p.keyFocus.lock())
            {
                widget->keyFocusEvent(true);
                _updates |= Update::Draw;
            }
        }

        void IWindow::setVisible(bool value)
        {
            const bool changed = value != _visible;
            IWidget::setVisible(value);
            TLRENDER_P();
            if (changed && !_visible)
            {
                if (auto hover = p.hover.lock())
                {
                    p.hover.reset();
                    hover->mouseLeaveEvent();
                }
                if (auto pressed = p.mousePress.lock())
                {
                    p.mousePress.reset();
                    p.mouseClickEvent.pos = p.cursorPos;
                    p.mouseClickEvent.accept = false;
                    pressed->mouseReleaseEvent(p.mouseClickEvent);
                }
                if (auto focus = p.keyFocus.lock())
                {
                    p.keyFocus.reset();
                    focus->keyFocusEvent(false);
                }
                if (auto keyPress = p.keyPress.lock())
                {
                    p.keyPress.reset();
                    p.keyEvent.pos = p.cursorPos;
                    p.keyEvent.accept = false;
                    keyPress->keyReleaseEvent(p.keyEvent);
                }
                if (auto dragAndDrop = p.dndHover.lock())
                {
                    p.dndHover.reset();
                    DragAndDropEvent event(
                        p.cursorPos, p.cursorPosPrev, p.dndData);
                    dragAndDrop->dragLeaveEvent(event);
                }
                p.dndData.reset();
                p.dndCursor.reset();
                _clipEventRecursive(shared_from_this(), _geometry, true);
            }
        }

        void IWindow::tickEvent(
            bool parentsVisible, bool parentsEnabled, const TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
        }

        void IWindow::drawOverlayEvent(
            const math::Box2i& clipRect, const DrawEvent& event)
        {
            IWidget::drawOverlayEvent(clipRect, event);
            TLRENDER_P();
            if (p.dndCursor)
            {
                event.render->drawImage(
                    p.dndCursor,
                    math::Box2i(
                        p.cursorPos.x - p.dndCursorHotspot.x,
                        p.cursorPos.y - p.dndCursorHotspot.y,
                        p.dndCursor->getWidth(), p.dndCursor->getHeight()),
                    image::Color4f(1.F, 1.F, 1.F));
            }
        }

        bool IWindow::_key(Key key, bool press, int modifiers)
        {
            TLRENDER_P();
            p.keyEvent.key = key;
            p.keyEvent.modifiers = modifiers;
            p.keyEvent.pos = p.cursorPos;
            p.keyEvent.accept = false;
            if (press)
            {
                // Send event to the focused widget or parent.
                if (auto widget = p.keyFocus.lock())
                {
                    while (widget)
                    {
                        widget->keyPressEvent(p.keyEvent);
                        if (p.keyEvent.accept)
                        {
                            p.keyPress = widget;
                            break;
                        }
                        widget = widget->getParent().lock();
                    }
                }

                // Send event to the hovered widget.
                if (!p.keyEvent.accept)
                {
                    auto widgets = _getUnderCursor(p.cursorPos);
                    for (auto i = widgets.begin(); i != widgets.end(); ++i)
                    {
                        (*i)->keyPressEvent(p.keyEvent);
                        if (p.keyEvent.accept)
                        {
                            p.keyPress = *i;
                            break;
                        }
                    }
                }

                // Handle tab key navigation.
                if (!p.keyEvent.accept && Key::Tab == key)
                {
                    auto keyFocus = p.keyFocus.lock();
                    if (modifiers == static_cast<int>(KeyModifier::Shift))
                    {
                        keyFocus = _keyFocusPrev(keyFocus);
                    }
                    else
                    {
                        keyFocus = _keyFocusNext(keyFocus);
                    }
                    setKeyFocus(keyFocus);
                }
            }
            else if (auto widget = p.keyPress.lock())
            {
                widget->keyReleaseEvent(p.keyEvent);
            }
            return p.keyEvent.accept;
        }

        void IWindow::_text(const std::string& value)
        {
            TLRENDER_P();
            TextEvent event(value);

            // Send event to the focused widget.
            if (auto widget = p.keyFocus.lock())
            {
                while (widget)
                {
                    widget->textEvent(event);
                    if (event.accept)
                    {
                        break;
                    }
                    widget = widget->getParent().lock();
                }
            }

            // Send event to the hovered widget.
            if (!event.accept)
            {
                auto widgets = _getUnderCursor(p.cursorPos);
                for (auto i = widgets.begin(); i != widgets.end(); ++i)
                {
                    (*i)->textEvent(event);
                    if (event.accept)
                    {
                        break;
                    }
                }
            }
        }

        void IWindow::_cursorEnter(bool enter)
        {
            TLRENDER_P();
            if (!enter)
            {
                _setHover(nullptr);
            }
        }

        void IWindow::_cursorPos(const math::Vector2i& pos)
        {
            TLRENDER_P();

            p.cursorPosPrev = p.cursorPos;
            p.cursorPos = pos;

            MouseMoveEvent event(p.cursorPos, p.cursorPosPrev);
            auto widget = p.mousePress.lock();
            if (widget)
            {
                if (p.dndData)
                {
                    // Find the drag and drop hover widget.
                    DragAndDropEvent event(
                        p.cursorPos, p.cursorPosPrev, p.dndData);
                    auto hover = p.dndHover.lock();
                    auto widgets = _getUnderCursor(p.cursorPos);
                    std::shared_ptr<IWidget> widget;
                    while (!widgets.empty())
                    {
                        if (hover == widgets.front())
                        {
                            break;
                        }
                        widgets.front()->dragEnterEvent(event);
                        if (event.accept)
                        {
                            widget = widgets.front();
                            break;
                        }
                        widgets.pop_front();
                    }
                    if (widget)
                    {
                        if (hover)
                        {
                            hover->dragLeaveEvent(event);
                        }
                        p.dndHover = widget;
                    }
                    else if (widgets.empty() && hover)
                    {
                        p.dndHover.reset();
                        hover->dragLeaveEvent(event);
                    }
                    hover = p.dndHover.lock();
                    if (hover)
                    {
                        DragAndDropEvent event(
                            p.cursorPos, p.cursorPosPrev, p.dndData);
                        hover->dragMoveEvent(event);
                    }
                }
                else
                {
                    widget->mouseMoveEvent(event);

                    p.dndData = event.dndData;
                    p.dndCursor = event.dndCursor;
                    p.dndCursorHotspot = event.dndCursorHotspot;
                    if (p.dndData)
                    {
                        // Start a drag and drop.
                        widget->mouseReleaseEvent(p.mouseClickEvent);
                        widget->mouseLeaveEvent();
                    }
                }
            }
            else
            {
                _hoverUpdate(event);
            }

            if (widget && p.dndCursor)
            {
                _updates |= Update::Draw;
            }
        }

        void IWindow::_mouseButton(int button, bool press, int modifiers)
        {
            TLRENDER_P();
            p.mouseClickEvent.button = button;
            p.mouseClickEvent.modifiers = modifiers;
            p.mouseClickEvent.pos = p.cursorPos;
            p.mouseClickEvent.accept = false;
            if (press)
            {
                auto widgets = _getUnderCursor(p.cursorPos);
                auto i = widgets.begin();
                for (; i != widgets.end(); ++i)
                {
                    (*i)->mousePressEvent(p.mouseClickEvent);
                    if (p.mouseClickEvent.accept)
                    {
                        p.mousePress = *i;
                        break;
                    }
                }
            }
            else
            {
                if (auto widget = p.mousePress.lock())
                {
                    p.mousePress.reset();
                    if (auto hover = p.dndHover.lock())
                    {
                        // Finish a drag and drop.
                        p.dndHover.reset();
                        DragAndDropEvent event(
                            p.cursorPos, p.cursorPosPrev, p.dndData);
                        hover->dragLeaveEvent(event);
                        hover->dropEvent(event);
                    }
                    else
                    {
                        widget->mouseReleaseEvent(p.mouseClickEvent);
                    }
                    p.dndData.reset();
                    p.dndCursor.reset();
                    _updates |= Update::Draw;
                }

                MouseMoveEvent event(p.cursorPos, p.cursorPosPrev);
                _hoverUpdate(event);
            }
        }

        void IWindow::_scroll(const math::Vector2f& value, int modifiers)
        {
            TLRENDER_P();
            ScrollEvent event(value, modifiers, p.cursorPos);
            auto widgets = _getUnderCursor(p.cursorPos);
            for (auto i = widgets.begin(); i != widgets.end(); ++i)
            {
                (*i)->scrollEvent(event);
                if (event.accept)
                {
                    break;
                }
            }
        }

        void IWindow::_clipEventRecursive(
            const std::shared_ptr<IWidget>& widget, const math::Box2i& clipRect,
            bool clipped)
        {
            const math::Box2i& g = widget->getGeometry();
            clipped |= !math::intersects(g, clipRect);
            clipped |= !widget->isVisible(false);
            const math::Box2i intersectedClipRect =
                math::intersect(g, clipRect);
            widget->clipEvent(intersectedClipRect, clipped);
            const math::Box2i childrenClipRect =
                math::intersect(widget->getChildrenClipRect(),
                                intersectedClipRect);
            for (const auto& child : widget->getChildren())
            {
                const math::Box2i& childGeometry = child->getGeometry();
                _clipEventRecursive(
                    child, math::intersect(childGeometry, childrenClipRect),
                    clipped);
            }
        }

        void IWindow::_drop(const std::vector<std::string>&) {}

        std::list<std::shared_ptr<IWidget> >
        IWindow::_getUnderCursor(const math::Vector2i& pos)
        {
            TLRENDER_P();
            std::list<std::shared_ptr<IWidget> > out;
            _getUnderCursor(shared_from_this(), pos, out);
            return out;
        }

        void IWindow::_getUnderCursor(
            const std::shared_ptr<IWidget>& widget, const math::Vector2i& pos,
            std::list<std::shared_ptr<IWidget> >& out)
        {
            if (!widget->isClipped() && widget->isEnabled() &&
                math::contains(widget->getGeometry(), pos))
            {
                for (auto i = widget->getChildren().rbegin();
                     i != widget->getChildren().rend(); ++i)
                {
                    _getUnderCursor(*i, pos, out);
                }
                out.push_back(widget);
            }
        }

        void IWindow::_setHover(const std::shared_ptr<IWidget>& hover)
        {
            TLRENDER_P();
            if (auto widget = p.hover.lock())
            {
                if (hover != widget)
                {
                    // std::cout << "leave: " << widget->getObjectName() <<
                    // std::endl;
                    widget->mouseLeaveEvent();
                    if (hover)
                    {
                        // std::cout << "enter: " << hover->getObjectName() <<
                        // std::endl;
                        hover->mouseEnterEvent();
                    }
                }
            }
            else if (hover)
            {
                // std::cout << "enter: " << hover->getObjectName() <<
                // std::endl;
                hover->mouseEnterEvent();
            }

            p.hover = hover;

            if (auto widget = p.hover.lock())
            {
                MouseMoveEvent event(p.cursorPos, p.cursorPosPrev);
                widget->mouseMoveEvent(event);
            }
        }

        void IWindow::_hoverUpdate(MouseMoveEvent& event)
        {
            auto widgets = _getUnderCursor(event.pos);
            while (!widgets.empty())
            {
                if (widgets.front()->hasMouseHover())
                {
                    break;
                }
                widgets.pop_front();
            }
            _setHover(!widgets.empty() ? widgets.front() : nullptr);
        }

        std::shared_ptr<IWidget>
        IWindow::_keyFocusNext(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            std::shared_ptr<IWidget> out;
            if (!_children.empty())
            {
                std::list<std::shared_ptr<IWidget> > widgets;
                _getKeyFocus(_children.back(), widgets);
                if (!widgets.empty())
                {
                    auto i = std::find(widgets.begin(), widgets.end(), value);
                    if (i != widgets.end())
                    {
                        ++i;
                        if (i != widgets.end())
                        {
                            out = *i;
                        }
                        else
                        {
                            out = widgets.front();
                        }
                    }
                    if (!out)
                    {
                        out = widgets.front();
                    }
                }
            }
            return out;
        }

        std::shared_ptr<IWidget>
        IWindow::_keyFocusPrev(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            std::shared_ptr<IWidget> out;
            if (!_children.empty())
            {
                std::list<std::shared_ptr<IWidget> > widgets;
                _getKeyFocus(_children.back(), widgets);
                if (!widgets.empty())
                {
                    auto i = std::find(widgets.rbegin(), widgets.rend(), value);
                    if (i != widgets.rend())
                    {
                        ++i;
                        if (i != widgets.rend())
                        {
                            out = *i;
                        }
                        else
                        {
                            out = widgets.back();
                        }
                    }
                    if (!out)
                    {
                        out = widgets.back();
                    }
                }
            }
            return out;
        }

        void IWindow::_getKeyFocus(
            const std::shared_ptr<IWidget>& widget,
            std::list<std::shared_ptr<IWidget> >& out)
        {
            if (widget->acceptsKeyFocus())
            {
                out.push_back(widget);
            }
            for (const auto& child : widget->getChildren())
            {
                if (!child->isClipped() && child->isEnabled())
                {
                    _getKeyFocus(child, out);
                }
            }
        }
    } // namespace ui
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/IWidget.h>

#include <tlUI/IWindow.h>

namespace tl
{
    namespace ui
    {
        void IWidget::_init(
            const std::string& objectName,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            _context = context;
            _objectName = objectName;
            _parent = parent;
            if (parent)
            {
                parent->_children.push_back(
                    std::static_pointer_cast<IWidget>(shared_from_this()));

                ChildEvent event;
                event.child = shared_from_this();
                parent->childAddedEvent(event);
            }
        }

        IWidget::IWidget() {}

        IWidget::~IWidget() {}

        void IWidget::setObjectName(const std::string& value)
        {
            _objectName = value;
        }

        void IWidget::setBackgroundRole(ColorRole value)
        {
            if (value == _backgroundRole)
                return;
            _backgroundRole = value;
            _updates |= Update::Draw;
        }

        void IWidget::setParent(const std::shared_ptr<IWidget>& value)
        {
            if (auto parent = _parent.lock())
            {
                auto i = std::find(
                    parent->_children.begin(), parent->_children.end(),
                    shared_from_this());
                if (i != parent->_children.end())
                {
                    ChildEvent event;
                    event.child = *i;
                    parent->_children.erase(i);
                    parent->childRemovedEvent(event);
                    parent->_updates |= Update::Size;
                    parent->_updates |= Update::Draw;
                }
            }
            _parent = value;
            if (value)
            {
                value->_children.push_back(
                    std::static_pointer_cast<IWidget>(shared_from_this()));
                ChildEvent event;
                event.child = shared_from_this();
                value->childAddedEvent(event);
                value->_updates |= Update::Size;
                value->_updates |= Update::Draw;
            }
        }

        void IWidget::moveToFront(const std::shared_ptr<IWidget>& value)
        {
            auto i = std::find(_children.begin(), _children.end(), value);
            if (i != _children.end())
            {
                auto child = *i;
                _children.erase(i);
                _children.push_back(child);
                value->_updates |= Update::Size;
                value->_updates |= Update::Draw;
            }
        }

        void IWidget::moveToBack(const std::shared_ptr<IWidget>& value)
        {
            auto i = std::find(_children.begin(), _children.end(), value);
            if (i != _children.end())
            {
                auto child = *i;
                _children.erase(i);
                _children.push_front(child);
                value->_updates |= Update::Size;
                value->_updates |= Update::Draw;
            }
        }

        std::shared_ptr<IWindow> IWidget::getWindow()
        {
            std::shared_ptr<IWidget> out = shared_from_this();
            auto parent = out->_parent.lock();
            while (parent)
            {
                out = parent;
                parent = parent->_parent.lock();
            }
            return std::dynamic_pointer_cast<IWindow>(out);
        }

        void IWidget::setHStretch(Stretch value)
        {
            if (value == _hStretch)
                return;
            _hStretch = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IWidget::setVStretch(Stretch value)
        {
            if (value == _vStretch)
                return;
            _vStretch = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IWidget::setStretch(Stretch horizontal, Stretch vertical)
        {
            if (horizontal == _hStretch && vertical == _vStretch)
                return;
            _hStretch = horizontal;
            _vStretch = vertical;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IWidget::setStretch(Stretch value)
        {
            setStretch(value, value);
        }

        void IWidget::setHAlign(HAlign value)
        {
            if (value == _hAlign)
                return;
            _hAlign = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IWidget::setVAlign(VAlign value)
        {
            if (value == _vAlign)
                return;
            _vAlign = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IWidget::setAlign(HAlign hAlign, VAlign vAlign)
        {
            if (hAlign == _hAlign && vAlign == _vAlign)
                return;
            _hAlign = hAlign;
            _vAlign = vAlign;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IWidget::setGeometry(const math::Box2i& value)
        {
            if (value == _geometry)
                return;
            _geometry = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IWidget::setVisible(bool value)
        {
            if (value == _visible)
                return;
            _visible = value;
            if (!_visible)
            {
                _releaseMouse();
                releaseKeyFocus();
            }
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IWidget::show()
        {
            setVisible(true);
        }

        void IWidget::hide()
        {
            setVisible(false);
        }

        math::Box2i IWidget::getChildrenClipRect() const
        {
            return _geometry;
        }

        void IWidget::setEnabled(bool value)
        {
            if (value == _enabled)
                return;
            _enabled = value;
            if (!_enabled)
            {
                _releaseMouse();
                releaseKeyFocus();
            }
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IWidget::setAcceptsKeyFocus(bool value)
        {
            _acceptsKeyFocus = value;
        }

        void IWidget::takeKeyFocus()
        {
            if (auto window = getWindow())
            {
                window->setKeyFocus(shared_from_this());
            }
        }

        void IWidget::releaseKeyFocus()
        {
            if (_keyFocus)
            {
                if (auto window = getWindow())
                {
                    window->setKeyFocus(nullptr);
                }
            }
        }

        void IWidget::setToolTip(const std::string& value)
        {
            _toolTip = value;
        }

        void IWidget::childAddedEvent(const ChildEvent&) {}

        void IWidget::childRemovedEvent(const ChildEvent&) {}

        void IWidget::tickEvent(
            bool parentsVisible, bool parentsEnabled, const TickEvent&)
        {
            _parentsVisible = parentsVisible;
            _parentsEnabled = parentsEnabled;
        }

        void IWidget::sizeHintEvent(const SizeHintEvent& event)
        {
            _displayScale = event.displayScale;
            _updates &= ~static_cast<int>(Update::Size);
        }

        void IWidget::clipEvent(const math::Box2i&, bool clipped)
        {
            if (clipped && clipped != _clipped)
            {
                _releaseMouse();
                releaseKeyFocus();
            }
            _clipped = clipped;
        }

        void IWidget::drawEvent(const math::Box2i&, const DrawEvent& event)
        {
            _updates &= ~static_cast<int>(Update::Draw);
            if (_backgroundRole != ColorRole::kNone)
            {
                event.render->drawRect(
                    _geometry, event.style->getColorRole(_backgroundRole));
            }
        }

        void IWidget::drawOverlayEvent(const math::Box2i&, const DrawEvent&)
        {
            _updates &= ~static_cast<int>(Update::Draw);
        }

        void IWidget::mouseEnterEvent()
        {
            _mouse.inside = true;
        }

        void IWidget::mouseLeaveEvent()
        {
            _mouse.inside = false;
        }

        void IWidget::mouseMoveEvent(MouseMoveEvent& event)
        {
            if (_mouseHoverEnabled)
            {
                event.accept = true;
            }
            _mouse.pos = event.pos;
        }

        void IWidget::mousePressEvent(MouseClickEvent& event)
        {
            const bool button = _mousePressButton != -1
                                    ? event.button == _mousePressButton
                                    : true;
            const bool modifiers = _mousePressModifiers != -1
                                       ? event.modifiers == _mousePressModifiers
                                       : true;
            if (_mousePressEnabled && button && modifiers)
            {
                event.accept = true;
                _mouse.press = true;
                _mouse.pressPos = event.pos;
            }
        }

        void IWidget::mouseReleaseEvent(MouseClickEvent& event)
        {
            if (_mouse.press)
            {
                event.accept = true;
                _mouse.press = false;
            }
        }

        void IWidget::scrollEvent(ScrollEvent&) {}

        void IWidget::keyFocusEvent(bool value)
        {
            _keyFocus = value;
        }

        void IWidget::keyPressEvent(KeyEvent&) {}

        void IWidget::keyReleaseEvent(KeyEvent&) {}

        void IWidget::textEvent(TextEvent&) {}

        void IWidget::dragEnterEvent(DragAndDropEvent&) {}

        void IWidget::dragLeaveEvent(DragAndDropEvent&) {}

        void IWidget::dragMoveEvent(DragAndDropEvent&) {}

        void IWidget::dropEvent(DragAndDropEvent&) {}

        void IWidget::_setMouseHover(bool value)
        {
            _mouseHoverEnabled = value;
        }

        void IWidget::_setMousePress(bool value, int button, int modifiers)
        {
            _mousePressEnabled = value;
            _mousePressButton = button;
            _mousePressModifiers = modifiers;
        }

        void IWidget::_releaseMouse()
        {
            if (_mouse.inside || _mouse.press)
            {
                _mouse.inside = false;
                _mouse.press = false;
                _updates |= Update::Draw;
            }
        }
    } // namespace ui
} // namespace tl

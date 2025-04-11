// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/IButton.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct IButton::Private
        {
            bool checkable = false;
            float iconScale = 1.F;
            bool iconInit = false;
            std::future<std::shared_ptr<image::Image> > iconFuture;
            bool checkedIconInit = false;
            std::future<std::shared_ptr<image::Image> > checkedIconFuture;
            bool repeatClick = false;
            bool repeatClickInit = false;
            std::chrono::steady_clock::time_point repeatClickTimer;
        };

        void IButton::_init(
            const std::string& objectName,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(objectName, context, parent);
            _setMouseHover(true);
            _setMousePress(true);
        }

        IButton::IButton() :
            _p(new Private)
        {
        }

        IButton::~IButton() {}

        bool IButton::isCheckable() const
        {
            return _p->checkable;
        }

        void IButton::setCheckable(bool value)
        {
            TLRENDER_P();
            if (value == p.checkable)
                return;
            p.checkable = value;
            if (!p.checkable && _checked)
            {
                _checked = false;
                _updates |= Update::Draw;
            }
        }

        bool IButton::isChecked() const
        {
            return _checked;
        }

        void IButton::setChecked(bool value)
        {
            TLRENDER_P();
            if (value == _checked)
                return;
            _checked = value;
            _updates |= Update::Draw;
        }

        void IButton::setText(const std::string& value)
        {
            if (value == _text)
                return;
            _text = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IButton::setFontRole(FontRole value)
        {
            if (value == _fontRole)
                return;
            _fontRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IButton::setIcon(const std::string& icon)
        {
            TLRENDER_P();
            _icon = icon;
            p.iconInit = true;
            _iconImage.reset();
        }

        void IButton::setCheckedIcon(const std::string& icon)
        {
            TLRENDER_P();
            _checkedIcon = icon;
            p.checkedIconInit = true;
            _checkedIconImage.reset();
        }

        void IButton::setButtonRole(ColorRole value)
        {
            if (value == _buttonRole)
                return;
            _buttonRole = value;
            _updates |= Update::Draw;
        }

        void IButton::setCheckedRole(ColorRole value)
        {
            if (value == _checkedRole)
                return;
            _checkedRole = value;
            _updates |= Update::Draw;
        }

        void IButton::setRepeatClick(bool value)
        {
            TLRENDER_P();
            p.repeatClick = value;
        }

        void IButton::setHoveredCallback(const std::function<void(bool)>& value)
        {
            _hoveredCallback = value;
        }

        void IButton::setPressedCallback(const std::function<void(void)>& value)
        {
            _pressedCallback = value;
        }

        void IButton::setClickedCallback(const std::function<void(void)>& value)
        {
            _clickedCallback = value;
        }

        void IButton::setCheckedCallback(const std::function<void(bool)>& value)
        {
            _checkedCallback = value;
        }

        void IButton::tickEvent(
            bool parentsVisible, bool parentsEnabled, const TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            TLRENDER_P();
            if (p.iconFuture.valid() &&
                p.iconFuture.wait_for(std::chrono::seconds(0)) ==
                    std::future_status::ready)
            {
                _iconImage = p.iconFuture.get();
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
            if (p.checkedIconFuture.valid() &&
                p.checkedIconFuture.wait_for(std::chrono::seconds(0)) ==
                    std::future_status::ready)
            {
                _checkedIconImage = p.checkedIconFuture.get();
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
            if (_mouse.press && p.repeatClick)
            {
                const float duration = p.repeatClickInit ? .4F : .02F;
                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff =
                    now - p.repeatClickTimer;
                if (diff.count() > duration)
                {
                    _click();
                    p.repeatClickInit = false;
                    p.repeatClickTimer = now;
                }
            }
        }

        void IButton::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();
            if (_displayScale != p.iconScale)
            {
                p.iconScale = _displayScale;
                p.iconInit = true;
                p.iconFuture = std::future<std::shared_ptr<image::Image> >();
                _iconImage.reset();
                p.checkedIconInit = true;
                p.checkedIconFuture =
                    std::future<std::shared_ptr<image::Image> >();
                _checkedIconImage.reset();
            }
            if (!_icon.empty() && p.iconInit)
            {
                p.iconInit = false;
                p.iconFuture = event.iconLibrary->request(_icon, _displayScale);
            }
            if (!_checkedIcon.empty() && p.checkedIconInit)
            {
                p.checkedIconInit = false;
                p.checkedIconFuture =
                    event.iconLibrary->request(_checkedIcon, _displayScale);
            }
        }

        void IButton::mouseEnterEvent()
        {
            IWidget::mouseEnterEvent();
            _updates |= Update::Draw;
            if (_hoveredCallback)
            {
                _hoveredCallback(_mouse.inside);
            }
        }

        void IButton::mouseLeaveEvent()
        {
            IWidget::mouseLeaveEvent();
            _updates |= Update::Draw;
            if (_hoveredCallback)
            {
                _hoveredCallback(_mouse.inside);
            }
        }

        void IButton::mousePressEvent(MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            if (acceptsKeyFocus())
            {
                takeKeyFocus();
            }
            _updates |= Update::Draw;
            if (_pressedCallback)
            {
                _pressedCallback();
            }
            if (p.repeatClick)
            {
                p.repeatClickInit = true;
                p.repeatClickTimer = std::chrono::steady_clock::now();
            }
        }

        void IButton::mouseReleaseEvent(MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            _updates |= Update::Draw;
            if (_geometry.contains(_mouse.pos))
            {
                _click();
            }
        }

        void IButton::_click()
        {
            TLRENDER_P();
            if (_clickedCallback)
            {
                _clickedCallback();
            }
            if (p.checkable)
            {
                _checked = !_checked;
                _updates |= Update::Draw;
                if (_checkedCallback)
                {
                    _checkedCallback(_checked);
                }
            }
        }

        void IButton::_releaseMouse()
        {
            const bool inside = _mouse.inside;
            IWidget::_releaseMouse();
            if (inside)
            {
                if (_hoveredCallback)
                {
                    _hoveredCallback(false);
                }
            }
        }
    } // namespace ui
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace ui
    {
        inline const std::string& IWidget::getObjectName() const
        {
            return _objectName;
        }

        inline int IWidget::getUpdates() const
        {
            return _updates;
        }

        inline const std::weak_ptr<IWidget>& IWidget::getParent() const
        {
            return _parent;
        }

        inline const std::list<std::shared_ptr<IWidget> >&
        IWidget::getChildren() const
        {
            return _children;
        }

        template <typename T>
        inline std::shared_ptr<T> IWidget::getParentT() const
        {
            std::shared_ptr<T> out;
            auto parent = _parent.lock();
            while (parent)
            {
                if (auto t = std::dynamic_pointer_cast<T>(parent))
                {
                    out = t;
                    break;
                }
                parent = parent->_parent.lock();
            }
            return out;
        }

        inline const math::Size2i& IWidget::getSizeHint() const
        {
            return _sizeHint;
        }

        inline Stretch IWidget::getHStretch() const
        {
            return _hStretch;
        }

        inline Stretch IWidget::getVStretch() const
        {
            return _vStretch;
        }

        inline HAlign IWidget::getHAlign() const
        {
            return _hAlign;
        }

        inline VAlign IWidget::getVAlign() const
        {
            return _vAlign;
        }

        inline const math::Box2i& IWidget::getGeometry() const
        {
            return _geometry;
        }

        inline bool IWidget::isVisible(bool andParentsVisible) const
        {
            bool out = _visible;
            if (andParentsVisible)
            {
                out &= _parentsVisible;
            }
            return out;
        }

        inline bool IWidget::isClipped() const
        {
            return _clipped;
        }

        inline bool IWidget::isEnabled(bool andParentsEnabled) const
        {
            bool out = _enabled;
            if (andParentsEnabled)
            {
                out &= _parentsEnabled;
            }
            return out;
        }

        inline bool IWidget::hasMouseHover()
        {
            return _mouseHoverEnabled;
        }

        inline bool IWidget::acceptsKeyFocus() const
        {
            return _acceptsKeyFocus;
        }

        inline bool IWidget::hasKeyFocus() const
        {
            return _keyFocus;
        }

        inline const std::string& IWidget::getToolTip() const
        {
            return _toolTip;
        }
    } // namespace ui
} // namespace tl

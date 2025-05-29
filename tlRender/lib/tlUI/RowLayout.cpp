// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/RowLayout.h>

#include <set>

namespace tl
{
    namespace ui
    {
        struct RowLayout::Private
        {
            Orientation orientation = Orientation::Horizontal;
            SizeRole marginRole = SizeRole::kNone;
            SizeRole spacingRole = SizeRole::Spacing;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
                int spacing = 0;
            };
            SizeData size;
        };

        void RowLayout::_init(
            Orientation orientation, const std::string& objectName,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(objectName, context, parent);
            TLRENDER_P();
            p.orientation = orientation;
        }

        RowLayout::RowLayout() :
            _p(new Private)
        {
        }

        RowLayout::~RowLayout() {}

        std::shared_ptr<RowLayout> RowLayout::create(
            Orientation orientation,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<RowLayout>(new RowLayout);
            out->_init(orientation, "tl::ui::RowLayout", context, parent);
            return out;
        }

        void RowLayout::setMarginRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void RowLayout::setSpacingRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.spacingRole)
                return;
            p.spacingRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void RowLayout::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const math::Box2i g = _geometry.margin(-p.size.margin);
            std::vector<math::Size2i> sizeHints;
            size_t expanding = 0;
            std::shared_ptr<IWidget> lastVisibleChild;
            for (const auto& child : _children)
            {
                if (child->isVisible(false))
                {
                    sizeHints.push_back(child->getSizeHint());
                    switch (p.orientation)
                    {
                    case Orientation::Horizontal:
                        if (Stretch::Expanding == child->getHStretch())
                        {
                            ++expanding;
                        }
                        break;
                    case Orientation::Vertical:
                        if (Stretch::Expanding == child->getVStretch())
                        {
                            ++expanding;
                        }
                        break;
                    }
                    lastVisibleChild = child;
                }
            }
            const std::pair<int, int> extra(
                _geometry.w() - _sizeHint.w, _geometry.h() - _sizeHint.h);
            math::Vector2i pos = g.min;
            size_t count = 0;
            for (const auto& child : _children)
            {
                if (child->isVisible(false))
                {
                    math::Size2i size = sizeHints[count];
                    switch (p.orientation)
                    {
                    case Orientation::Horizontal:
                        size.h = g.h();
                        if (expanding > 0 &&
                            Stretch::Expanding == child->getHStretch())
                        {
                            size.w += extra.first / expanding;
                            if (child == lastVisibleChild)
                            {
                                size.w += extra.first -
                                          (extra.first / expanding * expanding);
                            }
                        }
                        break;
                    case Orientation::Vertical:
                        size.w = g.w();
                        if (expanding > 0 &&
                            Stretch::Expanding == child->getVStretch())
                        {
                            size.h += extra.second / expanding;
                            if (child == lastVisibleChild)
                            {
                                size.h +=
                                    extra.second -
                                    (extra.second / expanding * expanding);
                            }
                        }
                        break;
                    }
                    child->setGeometry(math::Box2i(pos, size));
                    switch (p.orientation)
                    {
                    case Orientation::Horizontal:
                        pos.x += size.w;
                        if (sizeHints[count].w > 0)
                        {
                            for (size_t i = count + 1; i < sizeHints.size();
                                 ++i)
                            {
                                if (sizeHints[i].w > 0)
                                {
                                    pos.x += p.size.spacing;
                                    break;
                                }
                            }
                        }
                        break;
                    case Orientation::Vertical:
                        pos.y += size.h;
                        if (sizeHints[count].h > 0)
                        {
                            for (size_t i = count + 1; i < sizeHints.size();
                                 ++i)
                            {
                                if (sizeHints[i].h > 0)
                                {
                                    pos.y += p.size.spacing;
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    ++count;
                }
            }
        }

        math::Box2i RowLayout::getChildrenClipRect() const
        {
            return _geometry.margin(-_p->size.margin);
        }

        void RowLayout::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged =
                event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin =
                    event.style->getSizeRole(p.marginRole, _displayScale);
                p.size.spacing =
                    event.style->getSizeRole(p.spacingRole, _displayScale);
            }
            p.size.sizeInit = false;

            _sizeHint = math::Size2i();
            std::vector<math::Size2i> sizeHints;
            size_t visible = 0;
            for (const auto& child : _children)
            {
                if (child->isVisible(false))
                {
                    const math::Size2i& sizeHint = child->getSizeHint();
                    sizeHints.push_back(sizeHint);
                    switch (p.orientation)
                    {
                    case Orientation::Horizontal:
                        _sizeHint.w += sizeHint.w;
                        _sizeHint.h = std::max(_sizeHint.h, sizeHint.h);
                        if (sizeHint.w > 0)
                        {
                            ++visible;
                        }
                        break;
                    case Orientation::Vertical:
                        _sizeHint.w = std::max(_sizeHint.w, sizeHint.w);
                        _sizeHint.h += sizeHint.h;
                        if (sizeHint.h > 0)
                        {
                            ++visible;
                        }
                        break;
                    }
                }
            }
            if (visible > 0)
            {
                switch (p.orientation)
                {
                case Orientation::Horizontal:
                    _sizeHint.w += p.size.spacing * (visible - 1);
                    break;
                case Orientation::Vertical:
                    _sizeHint.h += p.size.spacing * (visible - 1);
                    break;
                default:
                    break;
                }
            }
            _sizeHint.w += p.size.margin * 2;
            _sizeHint.h += p.size.margin * 2;
        }

        void RowLayout::childAddedEvent(const ChildEvent&)
        {
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void RowLayout::childRemovedEvent(const ChildEvent&)
        {
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void HorizontalLayout::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            RowLayout::_init(
                Orientation::Horizontal, "tl::ui::HorizontalLayout", context,
                parent);
        }

        HorizontalLayout::HorizontalLayout() {}

        HorizontalLayout::~HorizontalLayout() {}

        std::shared_ptr<HorizontalLayout> HorizontalLayout::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<HorizontalLayout>(new HorizontalLayout);
            out->_init(context, parent);
            return out;
        }

        void VerticalLayout::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            RowLayout::_init(
                Orientation::Vertical, "tl::ui::VerticalLayout", context,
                parent);
        }

        VerticalLayout::VerticalLayout() {}

        VerticalLayout::~VerticalLayout() {}

        std::shared_ptr<VerticalLayout> VerticalLayout::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<VerticalLayout>(new VerticalLayout);
            out->_init(context, parent);
            return out;
        }
    } // namespace ui
} // namespace tl

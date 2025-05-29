// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace ui
    {
        //! Scroll type.
        enum class ScrollType { Horizontal, Vertical, Both, Menu };

        //! Scroll area.
        class ScrollArea : public IWidget
        {
            TLRENDER_NON_COPYABLE(ScrollArea);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&, ScrollType,
                const std::shared_ptr<IWidget>& parent);

            ScrollArea();

        public:
            virtual ~ScrollArea();

            //! Create a new widget.
            static std::shared_ptr<ScrollArea> create(
                const std::shared_ptr<system::Context>&,
                ScrollType = ScrollType::Both,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the scroll size.
            const math::Vector2i& getScrollSize() const;

            //! Set the scroll size callback.
            void setScrollSizeCallback(
                const std::function<void(const math::Vector2i&)>&);

            //! Get the scroll position.
            const math::Vector2i& getScrollPos() const;

            //! Set the scroll position.
            void setScrollPos(const math::Vector2i&, bool clamp = true);

            //! Set the scroll position callback.
            void setScrollPosCallback(
                const std::function<void(const math::Vector2i&)>&);

            //! Set whether the scroll area has a border.
            void setBorder(bool);

            math::Box2i getChildrenClipRect() const override;
            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const math::Box2i&, const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace ui
} // namespace tl

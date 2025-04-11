// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/ScrollArea.h>

namespace tl
{
    namespace ui
    {
        //! Scroll widget.
        class ScrollWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(ScrollWidget);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&, ScrollType,
                const std::shared_ptr<IWidget>& parent);

            ScrollWidget();

        public:
            virtual ~ScrollWidget();

            //! Create a new widget.
            static std::shared_ptr<ScrollWidget> create(
                const std::shared_ptr<system::Context>&,
                ScrollType = ScrollType::Both,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the widget.
            void setWidget(const std::shared_ptr<IWidget>&);

            //! Get the viewport geometry.
            math::Box2i getViewport() const;

            //! Get the scroll size.
            const math::Vector2i& getScrollSize() const;

            //! Get the scroll position.
            const math::Vector2i& getScrollPos() const;

            //! Set the scroll position.
            void setScrollPos(const math::Vector2i&, bool clamp = true);

            //! Set the scroll position callback.
            void setScrollPosCallback(
                const std::function<void(const math::Vector2i&)>&);

            //! Get whether the scroll bars are visible.
            bool areScrollBarsVisible() const;

            //! Set whether the scroll bars are visible.
            void setScrollBarsVisible(bool);

            //! Set whether scroll events are enabled.
            void setScrollEventsEnabled(bool);

            //! Set whether the scroll area has a border.
            void setBorder(bool);

            //! Set the margin role.
            void setMarginRole(SizeRole);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void scrollEvent(ScrollEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            int getLineStep() const;
            int getPageStep() const;

            TLRENDER_PRIVATE();
        };
    } // namespace ui
} // namespace tl

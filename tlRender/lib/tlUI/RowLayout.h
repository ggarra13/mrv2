// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Row layout.
        class RowLayout : public IWidget
        {
            TLRENDER_NON_COPYABLE(RowLayout);

        protected:
            void _init(
                Orientation, const std::string& objectName,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            RowLayout();

        public:
            virtual ~RowLayout();

            //! Create a new layout.
            static std::shared_ptr<RowLayout> create(
                Orientation, const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the margin role.
            void setMarginRole(SizeRole);

            //! Set the spacing role.
            void setSpacingRole(SizeRole);

            void setGeometry(const math::Box2i&) override;
            math::Box2i getChildrenClipRect() const override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void childAddedEvent(const ChildEvent&) override;
            void childRemovedEvent(const ChildEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Horizontal layout.
        class HorizontalLayout : public RowLayout
        {
            TLRENDER_NON_COPYABLE(HorizontalLayout);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            HorizontalLayout();

        public:
            virtual ~HorizontalLayout();

            //! Create a new layout.
            static std::shared_ptr<HorizontalLayout> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };

        //! Vertical layout.
        class VerticalLayout : public RowLayout
        {
            TLRENDER_NON_COPYABLE(VerticalLayout);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            VerticalLayout();

        public:
            virtual ~VerticalLayout();

            //! Create a new layout.
            static std::shared_ptr<VerticalLayout> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };
    } // namespace ui
} // namespace tl

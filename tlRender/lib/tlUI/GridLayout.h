// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Grid layout.
        class GridLayout : public IWidget
        {
            TLRENDER_NON_COPYABLE(GridLayout);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            GridLayout();

        public:
            virtual ~GridLayout();

            //! Create a new layout.
            static std::shared_ptr<GridLayout> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set a child position within the grid.
            void
            setGridPos(const std::shared_ptr<IWidget>&, int row, int column);

            //! Set the margin role.
            void setMarginRole(SizeRole);

            //! Set the spacing role.
            void setSpacingRole(SizeRole);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void childRemovedEvent(const ChildEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace ui
} // namespace tl

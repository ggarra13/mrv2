// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "IBasicItem.h"

#include <opentimelineio/gap.h>

namespace tl
{
    namespace TIMELINEUI
    {
        //! Gap item.
        class GapItem : public IBasicItem
        {
        protected:
            void _init(
                ui::ColorRole,
                const otio::SerializableObject::Retainer<otio::Gap>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            GapItem();

        public:
            virtual ~GapItem();

            //! Create a new item.
            static std::shared_ptr<GapItem> create(
                ui::ColorRole,
                const otio::SerializableObject::Retainer<otio::Gap>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };
    } // namespace TIMELINEUI
} // namespace tl

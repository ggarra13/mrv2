// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineUIVk/GapItem.h>

namespace tl
{
    namespace timelineui_vk
    {
        void GapItem::_init(
            ui::ColorRole colorRole,
            const otio::SerializableObject::Retainer<otio::Gap>& gap,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IBasicItem::_init(
                !gap->name().empty() ? gap->name() : "Gap", colorRole,
                "tl::timelineui_vk::GapItem", gap.value, scale, options,
                displayOptions, itemData, context, parent);
        }

        GapItem::GapItem() {}

        GapItem::~GapItem() {}

        std::shared_ptr<GapItem> GapItem::create(
            ui::ColorRole colorRole,
            const otio::SerializableObject::Retainer<otio::Gap>& gap,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<GapItem>(new GapItem);
            out->_init(
                colorRole, gap, scale, options, displayOptions, itemData,
                context, parent);
            return out;
        }
    } // namespace timelineui_vk
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUIVk/IBasicItem.h>

#include <opentimelineio/transition.h>

namespace tl
{
    namespace timelineui_vk
    {
        //! Transition item.
        class TransitionItem : public IItem
        {
        protected:
            void _init(
                const otio::SerializableObject::Retainer<otio::Transition>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TransitionItem();

        public:
            virtual ~TransitionItem();

            //! Create a new item.
            static std::shared_ptr<TransitionItem> create(
                const otio::SerializableObject::Retainer<otio::Transition>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
            
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(const math::Box2i&, bool) override;
            void drawEvent(const math::Box2i&, const ui::DrawEvent&) override;

        private:
            void _timeUnitsUpdate() override;
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace timelineui_vk
} // namespace tl

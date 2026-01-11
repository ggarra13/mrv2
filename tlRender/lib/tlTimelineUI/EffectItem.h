// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "IBasicItem.h"

#include <opentimelineio/effect.h>

namespace tl
{
    namespace TIMELINEUI
    {
        //! Effect item.
        class EffectItem : public IBasicItem
        {
        protected:
            void _init(
                const otio::SerializableObject::Retainer<otio::Effect>&,
                const otio::SerializableObject::Retainer<otio::Item>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            EffectItem();

        public:
            virtual ~EffectItem();

            //! Create a new item.
            static std::shared_ptr<EffectItem> create(
                const otio::SerializableObject::Retainer<otio::Effect>&,
                const otio::SerializableObject::Retainer<otio::Item>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setDurationLabel(const std::string&);
            
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(const math::Box2i&, bool) override;
            void drawEvent(const math::Box2i&, const ui::DrawEvent&) override;
            
        private:
            void _timeUnitsUpdate() override;
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace TIMELINEUI_vk
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "IItem.h"

#include <opentimelineio/gap.h>

namespace tl
{
    namespace TIMELINEUI
    {
        //! Base class for clips, gaps, and other items.
        class IBasicItem : public IItem
        {
        protected:
            void _init(
                const std::string& label, ui::ColorRole,
                const std::string& objectName,
                const otio::SerializableObject::Retainer<otio::Item>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IBasicItem();

        public:
            virtual ~IBasicItem() = 0;

            std::string getLabel() const;

            void setDisplayOptions(const DisplayOptions&) override;

            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(const math::Box2i&, bool) override;
            void drawEvent(const math::Box2i&, const ui::DrawEvent&) override;

            const otio::Item* getOtioItem() const;

        protected:
            int _getMargin() const;
            int _getLineHeight() const;
            math::Box2i _getInsideGeometry() const;

            void _timeUnitsUpdate() override;

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace TIMELINEUI
} // namespace tl

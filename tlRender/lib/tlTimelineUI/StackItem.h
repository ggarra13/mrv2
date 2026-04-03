// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "IItem.h"

#include <opentimelineio/stack.h>

namespace tl
{
    namespace TIMELINEUI
    {
        class ThumbnailGenerator;
    }
    
    namespace TIMELINEUI
    {
        //! Stack item.
        class StackItem : public IItem
        {
        protected:
            void _init(
                const otio::SerializableObject::Retainer<otio::Stack>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ThumbnailGenerator> thumbnailGenerator,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            StackItem();

        public:
            virtual ~StackItem();

            //! Create a new item.
            static std::shared_ptr<StackItem> create(
                const otio::SerializableObject::Retainer<otio::Stack>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ThumbnailGenerator> thumbnailGenerator,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setScale(double) override;
            void setDurationLabel(const std::string&);
            
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(const math::Box2i&, bool) override;
            void drawEvent(const math::Box2i&, const ui::DrawEvent&) override;

            const otio::Stack* getOtioItem() const;
            
        private:
            void _timeUnitsUpdate() override;
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace TIMELINEUI
} // namespace tl

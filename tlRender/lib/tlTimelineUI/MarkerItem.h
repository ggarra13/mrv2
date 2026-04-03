// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <tlTimelineUI/IItem.h>
// #include <toucanView/ItemLabel.h>

//#include <tlUI/RowLayout.h>
#include <tlUI/Label.h>

#include <opentimelineio/marker.h>

namespace tl
{
    namespace TIMELINEUI
    {
        //! Get a marker color.
        image::Color4f getMarkerColor(const std::string&);

        //! Timeline marker item.
        class MarkerItem : public IItem
        {
        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const OTIO_NS::Marker*,
                const OTIO_NS::TimeRange&,
                const std::shared_ptr<IWidget>& parent);

        public:
            virtual ~MarkerItem();

            //! Create a new item.
            static std::shared_ptr<MarkerItem> create(
                const std::shared_ptr<system::Context>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const otio::Marker*,
                const otio::TimeRange&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void drawEvent(const math::Box2i&, const ui::DrawEvent&) override;

        protected:
            void _timeUnitsUpdate() override;

        private:
            void _textUpdate();

            const otio::Timeline* _timeline = nullptr;
            const otio::Marker* _marker = nullptr;
            std::string _text;
            image::Color4f _color;

            std::shared_ptr<ui::Label> _label;

            struct SizeData
            {
                bool init = true;
                float displayScale = 0.F;
                int margin = 0;
                int border = 0;
            };
            SizeData _size;
        };

    }
}

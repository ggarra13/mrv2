// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "StackItem.h"
#include "ThumbnailSystem.h"

#include <tlUI/DrawUtil.h>

#ifdef VULKAN_BACKEND
#    include <tlTimelineVk/Render.h>
#endif

#include <tlTimeline/RenderUtil.h>
#include <tlTimeline/Util.h>

#include <tlIO/Cache.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/marker.h>

namespace tl
{
    namespace TIMELINEUI
    {
        struct StackItem::Private
        {
            std::string label;
            ui::FontRole labelFontRole = ui::FontRole::Label;
            std::string durationLabel;
            ui::FontRole durationFontRole = ui::FontRole::Label;

            struct SizeData
            {
                bool sizeInit = true;
                int dragLength = 0;
                
                int margin = 0;
                int spacing = 0;
                int border = 2;
                image::FontInfo labelFontInfo = image::FontInfo("", 0);
                image::FontInfo durationFontInfo = image::FontInfo("", 0);
                image::FontMetrics fontMetrics;
                int lineHeight = 0;
                bool textUpdate = true;
                math::Size2i labelSize;
                math::Size2i durationSize;
                math::Box2i clipRect;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<image::Glyph> > labelGlyphs;
                std::vector<std::shared_ptr<image::Glyph> > durationGlyphs;
            };
            DrawData draw;
            
            const otio::Stack* otioStack = nullptr;
        };

        void StackItem::_init(
            const otio::SerializableObject::Retainer<otio::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<ThumbnailGenerator> thumbnailGenerator,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            
            TLRENDER_P();
            
            otio::TimeRange timeRange = stack->trimmed_range();
            otio::TimeRange trimmedRange = timeRange;
            
            p.label = stack->name();
            IItem::_init(
                "tl::TIMELINEUI::StackItem", timeRange, trimmedRange,
                scale, options, displayOptions, itemData, context, parent);
            
            const auto& markers = stack->markers();
            if (!markers.empty())
            {
                //_markerLayout = TimeLayout::create(context, timeRange, _layout);
                for (const auto& marker : markers)
                {
                    otio::TimeRange markerRange = marker->marked_range();
                    markerRange = otio::TimeRange(/*timeline.start_time() + */
                                                  markerRange.start_time(),
                                                  markerRange.duration());
                    // auto markerItem = MarkerItem::create(
                    //     context,
                    //     data,
                    //     marker,
                    //     markerRange,
                    //     _markerLayout);
                    // _markerItems.push_back(markerItem);
                }
            }
        }

        StackItem::StackItem() :
            _p(new Private)
        {
        }

        StackItem::~StackItem()
        {
            TLRENDER_P();
        }

        std::shared_ptr<StackItem> StackItem::create(
            const otio::SerializableObject::Retainer<otio::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<TIMELINEUI::ThumbnailGenerator> thumbnailGenerator,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<StackItem>(new StackItem);
            out->_init(
                stack, scale, options, displayOptions, itemData,
                thumbnailGenerator, context, parent);
            return out;
        }

        void StackItem::setScale(double value)
        {
            const bool changed = value != _scale;
            IItem::setScale(value);
            TLRENDER_P();
            if (changed)
            {
                _updates |= ui::Update::Draw;
            }
        }

        void StackItem::sizeHintEvent(const ui::SizeHintEvent& event)
        { 
            const bool displayScaleChanged =
                event.displayScale != _displayScale;
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                _displayScale = event.displayScale;
                p.size.dragLength = event.style->getSizeRole(
                    ui::SizeRole::DragLength, _displayScale);
                p.size.sizeInit = false;
            }
            
            if (_displayOptions.thumbnails)
            {
                _sizeHint.h += _displayOptions.thumbnailHeight;
            }
        }

        void StackItem::clipEvent(const math::Box2i& clipRect, bool clipped)
        {
            IItem::clipEvent(clipRect, clipped);
            TLRENDER_P();
            if (clipRect == p.size.clipRect)
                return;
            p.size.clipRect = clipRect;
            if (clipped)
            {
                _updates |= ui::Update::Draw;
            }
        }

        void StackItem::drawEvent(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
        }
        
    } // namespace TIMELINEUI
} // namespace tl

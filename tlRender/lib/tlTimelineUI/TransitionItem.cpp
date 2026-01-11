// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "TransitionItem.h"

#include <tlUI/DrawUtil.h>

#include <tlTimeline/RenderUtil.h>
#include <tlTimeline/Util.h>

namespace tl
{
    namespace TIMELINEUI
    {
        struct TransitionItem::Private
        {
            std::string label;
            ui::FontRole labelFontRole = ui::FontRole::Label;
            std::string durationLabel;
            ui::FontRole durationFontRole = ui::FontRole::Label;

            struct SizeData
            {
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
        };

        void TransitionItem::_init(
            const otio::SerializableObject::Retainer<otio::Transition>&
                transition,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            otime::TimeRange timeRange = time::invalidTimeRange;
            otime::TimeRange trimmedRange = time::invalidTimeRange;
            const auto timeRangeOpt = transition->trimmed_range_in_parent();
            if (timeRangeOpt.has_value())
            {
                timeRange = timeRangeOpt.value();
                trimmedRange = otime::TimeRange(
                    otime::RationalTime(0.0, timeRange.duration().rate()),
                    timeRange.duration());
            }

            TLRENDER_P();

            p.label = transition->name();
            if (p.label.empty())
            {
                p.label = "Transition";
            }

            IItem::_init(
                "tl::TIMELINEUI::TransitionItem", timeRange, trimmedRange,
                scale, options, displayOptions, itemData, context, parent);
        }

        TransitionItem::TransitionItem() :
            _p(new Private)
        {
        }

        TransitionItem::~TransitionItem() {}
        
        std::shared_ptr<TransitionItem> TransitionItem::create(
            const otio::SerializableObject::Retainer<otio::Transition>&
                transition,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TransitionItem>(new TransitionItem);
            out->_init(
                transition, scale, options, displayOptions, itemData, context,
                parent);
            return out;
        }

        void TransitionItem::setDurationLabel(const std::string& value)
        {
            _p->durationLabel = value;
            _p->draw.durationGlyphs.clear();
        }
        
        void TransitionItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(
                ui::SizeRole::MarginSmall, event.displayScale);
            p.size.spacing = event.style->getSizeRole(
                ui::SizeRole::SpacingSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(
                ui::SizeRole::Border, event.displayScale);

            auto fontInfo =
                event.style->getFontRole(p.labelFontRole, event.displayScale);
            if (fontInfo != p.size.labelFontInfo || p.size.textUpdate)
            {
                p.size.labelFontInfo = fontInfo;
                p.size.fontMetrics = event.fontSystem->getMetrics(fontInfo);
                p.size.lineHeight = p.size.fontMetrics.lineHeight;
                p.size.labelSize = event.fontSystem->getSize(p.label, fontInfo);
            }
            fontInfo = event.style->getFontRole(
                p.durationFontRole, event.displayScale);
            if (fontInfo != p.size.durationFontInfo || p.size.textUpdate)
            {
                p.size.durationFontInfo = fontInfo;
                p.size.durationSize =
                    event.fontSystem->getSize(p.durationLabel, fontInfo);
            }
            p.size.textUpdate = false;

            _sizeHint = math::Size2i(
                _timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.margin + p.size.lineHeight + p.size.margin);
        }

        void
        TransitionItem::clipEvent(const math::Box2i& clipRect, bool clipped)
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

        void TransitionItem::drawEvent(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i& g = _geometry;
            ui::ColorRole colorRole = getSelectRole();
            if (colorRole != ui::ColorRole::kNone)
            {
                event.render->drawMesh(
                    ui::border(g, p.size.border * 2), math::Vector2i(),
                    event.style->getColorRole(colorRole));
            }

            const math::Box2i g2 = g.margin(-p.size.border);
            event.render->drawMesh(
                ui::rect(g2, p.size.margin), math::Vector2i(),
                event.style->getColorRole(ui::ColorRole::Transition));

            const math::Box2i labelGeometry(
                g.min.x + p.size.margin, g.min.y + p.size.margin,
                p.size.labelSize.w, p.size.lineHeight);
            const math::Box2i durationGeometry(
                g.max.x - p.size.margin - p.size.durationSize.w,
                g.min.y + p.size.margin, p.size.durationSize.w,
                p.size.lineHeight);
            const bool labelVisible = drawRect.intersects(labelGeometry);
            const bool durationVisible =
                drawRect.intersects(durationGeometry) &&
                !durationGeometry.intersects(labelGeometry);
            
            std::vector<timeline::TextInfo> textInfos;

            if (labelVisible)
            {
                if (!p.label.empty() && p.draw.labelGlyphs.empty())
                {
                    p.draw.labelGlyphs = event.fontSystem->getGlyphs(
                        p.label, p.size.labelFontInfo);
                }
                event.render->appendText(
                    textInfos,
                    p.draw.labelGlyphs,
                    math::Vector2i(
                        labelGeometry.min.x,
                        labelGeometry.min.y + p.size.fontMetrics.ascender));
            }

            if (durationVisible)
            {
                if (!p.durationLabel.empty() && p.draw.durationGlyphs.empty())
                {
                    p.draw.durationGlyphs = event.fontSystem->getGlyphs(
                        p.durationLabel, p.size.durationFontInfo);
                }
                event.render->appendText(
                    textInfos,
                    p.draw.durationGlyphs,
                    math::Vector2i(
                        durationGeometry.min.x,
                        durationGeometry.min.y + p.size.fontMetrics.ascender));
            }
            
            for (const auto& textInfo : textInfos)
            {
                event.render->drawText(textInfo, math::Vector2i(),
                                       event.style->getColorRole(ui::ColorRole::Text));
            }
        }

        void TransitionItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
        }

        void TransitionItem::_textUpdate()
        {
            TLRENDER_P();
            p.durationLabel = IItem::_getDurationLabel(_timeRange.duration());
            p.size.textUpdate = true;
            p.draw.durationGlyphs.clear();
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }
    } // namespace TIMELINEUI
} // namespace tl

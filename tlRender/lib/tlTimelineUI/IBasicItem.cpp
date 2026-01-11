// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "IBasicItem.h"

#include <tlUI/DrawUtil.h>

#include <tlTimeline/RenderUtil.h>

namespace tl
{
    namespace TIMELINEUI
    {
        struct IBasicItem::Private
        {
            std::string label;
            std::string durationLabel;
            ui::ColorRole colorRole = ui::ColorRole::VideoClip;
            std::vector<Marker> markers;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
                int border = 0;

                bool textInit = true;
                image::FontInfo fontInfo = image::FontInfo("", 0);
                image::FontMetrics fontMetrics;
                math::Size2i labelSize;
                math::Size2i durationSize;
                std::vector<math::Size2i> markerSizes;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<image::Glyph> > labelGlyphs;
                std::vector<std::shared_ptr<image::Glyph> > durationGlyphs;
                std::vector<std::vector<std::shared_ptr<image::Glyph> > >
                    markerGlyphs;
            };
            DrawData draw;

            const otio::Item* otioItem = nullptr;
        };

        void IBasicItem::_init(
            const std::string& label, ui::ColorRole colorRole,
            const std::string& objectName,
            const otio::SerializableObject::Retainer<otio::Item>& item,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            otime::TimeRange timeRange = time::invalidTimeRange;
            const auto timeRangeOpt = item->trimmed_range_in_parent();
            if (timeRangeOpt.has_value())
            {
                timeRange = timeRangeOpt.value();
            }
            const otime::TimeRange trimmedRange = item->trimmed_range();
            IItem::_init(
                objectName, timeRange, trimmedRange, scale, options,
                displayOptions, itemData, context, parent);
            TLRENDER_P();

            p.label = label;
            p.colorRole = colorRole;
            p.markers = getMarkers(item.value);
            p.otioItem = item.value;

            _textUpdate();
        }

        IBasicItem::IBasicItem() :
            _p(new Private)
        {
        }

        IBasicItem::~IBasicItem() {}

        std::string IBasicItem::getLabel() const
        {
            return _p->label;
        }

        const otio::Item* IBasicItem::getOtioItem() const
        {
            return _p->otioItem;
        }
        
        void IBasicItem::setDisplayOptions(const DisplayOptions& value)
        {
            const bool changed = value != _displayOptions;
            IItem::setDisplayOptions(value);
            TLRENDER_P();
            if (changed)
            {
                _textUpdate();
            }
        }

        void IBasicItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            const bool displayScaleChanged =
                event.displayScale != _displayScale;
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(
                    ui::SizeRole::MarginInside, _displayScale);
                p.size.border = event.style->getSizeRole(
                    ui::SizeRole::Border, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo = image::FontInfo(
                    _displayOptions.regularFont,
                    _displayOptions.fontSize * _displayScale);
                p.size.fontMetrics =
                    event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.labelSize =
                    _displayOptions.clipInfo
                        ? event.fontSystem->getSize(p.label, p.size.fontInfo)
                        : math::Size2i();
                p.size.durationSize =
                    _displayOptions.clipInfo
                        ? event.fontSystem->getSize(
                              p.durationLabel, p.size.fontInfo)
                        : math::Size2i();
                p.draw.labelGlyphs.clear();
                p.draw.durationGlyphs.clear();
                p.draw.markerGlyphs.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;

            _sizeHint.w =
                _timeRange.duration().rescaled_to(1.0).value() * _scale;
            _sizeHint.h = 0;
            if (_displayOptions.clipInfo)
            {
                _sizeHint.h +=
                    p.size.fontMetrics.lineHeight + p.size.margin * 2;
            }
            if (_displayOptions.markers)
            {
                for (const auto& marker : p.markers)
                {
                    _sizeHint.h += p.size.fontMetrics.lineHeight +
                                   p.size.margin * 2 + p.size.border * 2;
                }
            }
            _sizeHint.h += p.size.border * 4;
        }

        void IBasicItem::clipEvent(const math::Box2i& clipRect, bool clipped)
        {
            IItem::clipEvent(clipRect, clipped);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.labelGlyphs.clear();
                p.draw.durationGlyphs.clear();
                p.draw.markerGlyphs.clear();
            }
        }

        void IBasicItem::drawEvent(
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

            const math::Box2i g2 = g.margin(-(p.size.border * 2));
            event.render->drawRect(g2, event.style->getColorRole(p.colorRole));

            const timeline::ClipRectEnabledState clipRectEnabledState(
                event.render);
            const timeline::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(g2.intersect(drawRect));
            
            std::vector<timeline::TextInfo> textInfos;

            if (_displayOptions.clipInfo)
            {
                const math::Box2i labelGeometry(
                    g2.min.x + p.size.margin, g2.min.y + p.size.margin,
                    p.size.labelSize.w, p.size.fontMetrics.lineHeight);
                if (drawRect.intersects(labelGeometry))
                {
                    if (!p.label.empty() && p.draw.labelGlyphs.empty())
                    {
                        p.draw.labelGlyphs = event.fontSystem->getGlyphs(
                            p.label, p.size.fontInfo);
                    }

                    
                    event.render->appendText(
                        textInfos,
                        p.draw.labelGlyphs,
                        math::Vector2i(
                            labelGeometry.min.x,
                            labelGeometry.min.y + p.size.fontMetrics.ascender));
                }

                const math::Box2i durationGeometry(
                    g2.max.x - p.size.durationSize.w - p.size.margin,
                    g2.min.y + p.size.margin, p.size.durationSize.w,
                    p.size.fontMetrics.lineHeight);
                if (drawRect.intersects(durationGeometry) &&
                    !durationGeometry.intersects(labelGeometry))
                {
                    if (!p.durationLabel.empty() &&
                        p.draw.durationGlyphs.empty())
                    {
                        p.draw.durationGlyphs = event.fontSystem->getGlyphs(
                            p.durationLabel, p.size.fontInfo);
                    }
                    event.render->appendText(
                        textInfos,
                        p.draw.durationGlyphs,
                        math::Vector2i(
                            durationGeometry.min.x,
                            durationGeometry.min.y +
                                p.size.fontMetrics.ascender));
                }
            }

            if (_displayOptions.markers)
            {
                p.draw.markerGlyphs.resize(p.markers.size());

                float y = g2.max.y + 1 -
                          (p.size.fontMetrics.lineHeight + p.size.margin * 2 +
                           p.size.border * 2) *
                              p.markers.size();
                for (size_t i = 0; i < p.markers.size(); ++i)
                {
                    const int x0 = _geometry.min.x + p.markers[i]
                                                             .range.start_time()
                                                             .rescaled_to(1.0)
                                                             .value() *
                                                         _scale;
                    const int x1 = _geometry.min.x +
                                   p.markers[i]
                                           .range.end_time_exclusive()
                                           .rescaled_to(1.0)
                                           .value() *
                                       _scale -
                                   1;
                    math::Box2i mg;
                    mg.min.x = std::max(x0, g2.min.x);
                    mg.min.y = y;
                    mg.max.x = std::min(x1, g2.max.x);
                    mg.max.y = y + p.size.border * 2 - 1;
                    event.render->drawRect(mg, p.markers[i].color);

                    y += p.size.border * 2;

                    // \@bug: mrv2 would crash as markerSizes would sometimes
                    //        not be initialized properly.
                    if (p.markers.size() > p.size.markerSizes.size())
                    {
                        p.size.markerSizes.clear();
                        for (const auto& marker : p.markers)
                        {
                            p.size.markerSizes.push_back(
                                event.fontSystem->getSize(
                                    marker.name, p.size.fontInfo));
                        }
                    }

                    const math::Box2i labelGeometry = math::Box2i(
                        g2.min.x + p.size.margin, y + p.size.margin,
                        p.size.markerSizes[i].w, p.size.fontMetrics.lineHeight);
                    if (drawRect.intersects(labelGeometry))
                    {
                        if (!p.markers[i].name.empty() &&
                            p.draw.markerGlyphs[i].empty())
                        {
                            p.draw.markerGlyphs[i] =
                                event.fontSystem->getGlyphs(
                                    p.markers[i].name, p.size.fontInfo);
                        }
                        event.render->appendText(
                            textInfos,
                            p.draw.markerGlyphs[i],
                            math::Vector2i(
                                labelGeometry.min.x,
                                labelGeometry.min.y +
                                    p.size.fontMetrics.ascender));
                    }

                    y += p.size.fontMetrics.lineHeight + p.size.margin * 2;
                }
            }
            
            for (const auto& textInfo : textInfos)
            {
                event.render->drawText(textInfo, math::Vector2i(),
                                       event.style->getColorRole(ui::ColorRole::Text));
            }
            
        }

        int IBasicItem::_getMargin() const
        {
            return _p->size.margin;
        }

        int IBasicItem::_getLineHeight() const
        {
            return _p->size.fontMetrics.lineHeight;
        }

        math::Box2i IBasicItem::_getInsideGeometry() const
        {
            return _geometry.margin(-(_p->size.border * 2));
        }

        void IBasicItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
        }

        void IBasicItem::_textUpdate()
        {
            TLRENDER_P();
            p.durationLabel = _getDurationLabel(_timeRange.duration());
            p.size.textInit = true;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }
    } // namespace TIMELINEUI
} // namespace tl

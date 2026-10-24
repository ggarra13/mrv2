// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimelineUI/TimelineRuler.h>

#include <tlCore/RenderUtil.h>

#include <cmath>

namespace tl
{
    namespace ui
    {
        namespace
        {
            //! The cache is drawn as one mesh rather than a rectangle per
            //! cached range, which is a draw call per frame instead of one per
            //! range.
            void addRect(geom::TriangleMesh2& mesh, const math::Box2i& box)
            {
                const size_t v = mesh.v.size();
                mesh.v.emplace_back(math::Vector2f(box.min.x, box.min.y));
                mesh.v.emplace_back(math::Vector2f(box.max.x + 1, box.min.y));
                mesh.v.emplace_back(math::Vector2f(box.max.x + 1, box.max.y + 1));
                mesh.v.emplace_back(math::Vector2f(box.min.x, box.max.y + 1));
                mesh.triangles.push_back({ (v + 1), (v + 3), (v + 2) });
                mesh.triangles.push_back({ (v + 3), (v + 1), (v + 4) });
            }

            void clearMesh(geom::TriangleMesh2& mesh)
            {
                mesh.v.clear();
                mesh.c.clear();
                mesh.triangles.clear();
            }
        }

        struct TimelineRuler::Private
        {
            std::shared_ptr<ItemData> data;
            std::shared_ptr<timeline::Player> player;
            OTIO_NS::TimeRange timeRange;
            double scale = 500.0;
            OTIO_NS::RationalTime offset;
            int scrollPos = 0;
            std::vector<int> frameMarkers;
            std::vector<Marker> markers;
            DisplayOptions displayOptions;
            ItemOptions options;
            bool stopOnScrub = true;
            std::shared_ptr<observer::Value<bool> > scrub;
            std::shared_ptr<observer::Value<std::optional<OTIO_NS::RationalTime> > > timeScrub;

            enum class MouseMode
            {
                None,
                CurrentTime
            };
            MouseMode mouseMode = MouseMode::None;

            // Where the press found the player, for a press that is
            // cancelled: the first finger of a touch gesture is not
            // scrubbing, and puts back what it changed.
            std::optional<OTIO_NS::RationalTime> pressTime;
            Playback pressPlayback = Playback::Stop;

            std::optional<OTIO_NS::RationalTime> currentTime;
            std::optional<OTIO_NS::TimeRange> inOutRange;
            PlayerCacheInfo cacheInfo;

            struct SizeData
            {
                bool init = true;
                int margin = 0;
                int border = 0;
                int handle = 0;
                ftk::FontInfo fontInfo;
                ftk::FontMetrics fontMetrics;
                ftk::Size2I sizeHint;
            };
            SizeData size;

            math::TriangleMesh2 cacheMesh;

            std::shared_ptr<observer::Value<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<observer::Value<OTIO_NS::TimeRange> > inOutRangeObserver;
            std::shared_ptr<observer::Value<PlayerCacheInfo> > cacheInfoObserver;
        };

        void TimelineRuler::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<ItemData>& data,
            const std::shared_ptr<IWidget>& parent)
        {
            const OTIO_NS::TimeRange timeRange;
            const OTIO_NS::TimeRange trimmedRange;
            IItem::_init(
                "tl::TIMELINEUI::TimelineItem", timeRange, trimmedRange,
                trimmedRange, scale,
                options, displayOptions, itemData, context, parent);
            IWidget::_init(context, "tl::ui::TimelineRuler", parent);
            TLRENDER_P();
            p.data = data;
            p.scrub = observer::ValueObserver<bool>::create(false);
            p.timeScrub = observer::ValueObserver<std::optional<OTIO_NS::RationalTime> >::create();

            // The same binding the timelines take, so that the ruler is
            // dragged to scrub as the timelines under it are.
            _setMousePressEnabled(true, 0 /*left*/, 0 /*no alt/cmd/shift*/);
        }

        TimelineRuler::TimelineRuler() :
            _p(new Private)
        {}

        TimelineRuler::~TimelineRuler()
        {}

        std::shared_ptr<TimelineRuler> TimelineRuler::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<ItemData>& data,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineRuler>(new TimelineRuler);
            out->_init(context, data, parent);
            return out;
        }

        void TimelineRuler::setItemData(const std::shared_ptr<ItemData>& value)
        {
            TLRENDER_P();
            p.data = value;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setPlayer(const std::shared_ptr<Player>& value)
        {
            TLRENDER_P();
            if (value == p.player)
                return;

            p.currentTimeObserver.reset();
            p.inOutRangeObserver.reset();
            p.cacheInfoObserver.reset();
            p.currentTime.reset();
            p.inOutRange.reset();
            p.cacheInfo = PlayerCacheInfo();
            p.timeRange = OTIO_NS::TimeRange();

            p.player = value;

            if (p.player)
            {
                p.timeRange = p.player->getTimeRange();

                p.currentTimeObserver = ftk::Observer<OTIO_NS::RationalTime>::create(
                    p.player->observeCurrentTime(),
                    [this](const OTIO_NS::RationalTime& value)
                    {
                        _p->currentTime = value;
                        _updates |= ui::Update::Draw;
                    });

                p.inOutRangeObserver = ftk::Observer<OTIO_NS::TimeRange>::create(
                    p.player->observeInOutRange(),
                    [this](const OTIO_NS::TimeRange value)
                    {
                        _p->inOutRange = value;
                        _updates |= ui::Update::Draw;
                    });

                p.cacheInfoObserver = ftk::Observer<PlayerCacheInfo>::create(
                    p.player->observeCacheInfo(),
                    [this](const PlayerCacheInfo& value)
                    {
                        _p->cacheInfo = value;
                        _updates |= ui::Update::Draw;
                    });
            }

            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setScale(double value)
        {
            TLRENDER_P();
            if (value == p.scale)
                return;
            p.scale = value;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setOffset(const OTIO_NS::RationalTime& value)
        {
            TLRENDER_P();
            if (compareExact(value, p.offset))
                return;
            p.offset = value;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setScrollPos(int value)
        {
            TLRENDER_P();
            if (value == p.scrollPos)
                return;
            p.scrollPos = value;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setFrameMarkers(const std::vector<int>& value)
        {
            TLRENDER_P();
            if (value == p.frameMarkers)
                return;
            p.frameMarkers = value;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setMarkers(const std::vector<Marker>& value)
        {
            TLRENDER_P();
            if (value == p.markers)
                return;
            p.markers = value;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setDisplayOptions(const DisplayOptions& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setOptions(const ItemOptions& value)
        {
            _p->options = value;
        }

        void TimelineRuler::setStopOnScrub(bool value)
        {
            _p->stopOnScrub = value;
        }

        std::shared_ptr<observer::IValueObservable<bool> > TimelineRuler::observeScrub() const
        {
            return _p->scrub;
        }

        std::shared_ptr<observer::IValueObservable<std::optional<OTIO_NS::RationalTime> > > TimelineRuler::observeTimeScrub() const
        {
            return _p->timeScrub;
        }

        void TimelineRuler::mouseMoveEvent(ftk::MouseMoveEvent& event)
        {
            IMouseWidget::mouseMoveEvent(event);
            TLRENDER_P();
            if (Private::MouseMode::CurrentTime == p.mouseMode)
            {
                const OTIO_NS::RationalTime time = _posToTimeClamped(event.pos.x);
                p.timeScrub->setIfChanged(time);
                p.player->seek(time);
            }
        }

        void TimelineRuler::mousePressEvent(ftk::MouseClickEvent& event)
        {
            IMouseWidget::mousePressEvent(event);
            TLRENDER_P();
            takeKeyFocus();
            if (p.player &&
                p.options.inputEnabled &&
                ftk::MouseButton::Left == event.button &&
                0 == event.modifiers)
            {
                p.mouseMode = Private::MouseMode::CurrentTime;
                p.pressTime = p.player->getCurrentTime();
                p.pressPlayback = p.player->getPlayback();
                if (p.stopOnScrub)
                {
                    p.player->stop();
                }
                const OTIO_NS::RationalTime time = _posToTimeClamped(event.pos.x);
                p.scrub->setIfChanged(true);
                p.timeScrub->setIfChanged(time);
                p.player->seek(time);
            }
        }

        void TimelineRuler::mouseReleaseEvent(ftk::MouseClickEvent& event)
        {
            IMouseWidget::mouseReleaseEvent(event);
            TLRENDER_P();
            if (Private::MouseMode::CurrentTime == p.mouseMode)
            {
                if (event.cancel && p.player)
                {
                    // Back to the frame, and playing if it was.
                    if (p.pressTime.has_value())
                    {
                        p.player->seek(p.pressTime.value());
                    }
                    p.player->setPlayback(p.pressPlayback);
                }
            }
            p.scrub->setIfChanged(false);
            p.mouseMode = Private::MouseMode::None;
        }

        void TimelineRuler::mouseLeaveEvent()
        {
            IMouseWidget::mouseLeaveEvent();
            TLRENDER_P();
        }

        OTIO_NS::RationalTime TimelineRuler::_posToTimeClamped(float value) const
        {
            TLRENDER_P();
            // Clamped, unlike the one the ticks are drawn from: a seek has to
            // land inside the range, while a tick past the end is only a tick
            // that is not drawn.
            return math::clamp(
                _posToTime(value),
                p.timeRange.start_time(),
                p.timeRange.end_time_inclusive());
        }

        int TimelineRuler::timeToPos(const OTIO_NS::RationalTime& value) const
        {
            TLRENDER_P();
            const math::Box2i& g = getGeometry();
            const OTIO_NS::RationalTime t = value - p.timeRange.start_time();
            return
                g.min.x -
                p.scrollPos +
                (t.rescaled_to(1.0).value() +
                    p.offset.rescaled_to(1.0).value()) * p.scale;
        }

        OTIO_NS::RationalTime TimelineRuler::_posToTime(float value) const
        {
            TLRENDER_P();
            OTIO_NS::RationalTime out = p.timeRange.start_time();
            const double duration = p.timeRange.duration().rescaled_to(1.0).value();
            if (duration > 0.0)
            {
                const math::Box2i& g = getGeometry();
                const double normalized =
                    (value - g.min.x + p.scrollPos -
                        p.offset.rescaled_to(1.0).value() * p.scale) /
                    (duration * p.scale);
                out = OTIO_NS::RationalTime(
                    p.timeRange.start_time() +
                    OTIO_NS::RationalTime(
                        p.timeRange.duration().value() * normalized,
                        p.timeRange.duration().rate())).
                    round();
            }
            return out;
        }

        std::string TimelineRuler::_timeLabel(
            const OTIO_NS::RationalTime& value) const
        {
            TLRENDER_P();
            // A position, so it is named in the media's time. The frame the
            // ruler counts off is not the frame the media calls it when the
            // sequence was built out of the frames it has.
            return p.data->timeUnitsModel->getLabel(
                p.data->toMediaTime ? p.data->toMediaTime(value) : value);
        }

        ftk::Size2I TimelineRuler::_getLabelMaxSize(
            const std::shared_ptr<ftk::FontSystem>& fontSystem) const
        {
            TLRENDER_P();
            const std::string labelMax =
                p.data->timeUnitsModel->getLabel(p.timeRange.duration());
            return fontSystem->getSize(labelMax, p.size.fontInfo);
        }

        double TimelineRuler::_getSecondsInc(
            const std::shared_ptr<ftk::FontSystem>& fontSystem)
        {
            TLRENDER_P();
            double out = 0.0;
            const double duration = p.timeRange.duration().rescaled_to(1.0).value();
            if (duration > 0.0)
            {
                // Against the width the timelines take at this scale, not the
                // width on screen: the ticks are spaced along the timeline,
                // and only some of it is in view.
                const int w = duration * p.scale;
                const int secondsTick = 1.0 / duration * w;
                const int minutesTick = 60.0 / duration * w;
                const int hoursTick = 3600.0 / duration * w;
                const ftk::Size2I labelMaxSize = _getLabelMaxSize(fontSystem);
                const int distanceMin = p.size.border + p.size.margin + labelMaxSize.w;
                if (secondsTick >= distanceMin)
                {
                    out = 1.0;
                }
                else if (minutesTick >= distanceMin)
                {
                    out = 60.0;
                }
                else if (hoursTick >= distanceMin)
                {
                    out = 3600.0;
                }
            }
            return out;
        }

        ftk::Size2I TimelineRuler::getSizeHint() const
        {
            return _p->size.sizeHint;
        }

        void TimelineRuler::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IMouseWidget::sizeHintEvent(event);
            TLRENDER_P();
            if (p.size.init || p.size.fontInfo != event.style->getFont(
                ftk::FontType::Mono, event.displayScale))
            {
                p.size.init = false;
                p.size.margin = event.style->getSizeRole(
                    ftk::SizeRole::MarginInside, event.displayScale);
                p.size.border = event.style->getSizeRole(
                    ftk::SizeRole::Border, event.displayScale);
                p.size.handle = event.style->getSizeRole(
                    ftk::SizeRole::Handle, event.displayScale);
                p.size.fontInfo = event.style->getFont(
                    ftk::FontType::Mono, event.displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
            }

            // The width is whatever it is given: the ruler is as wide as the
            // view, and which part of the timeline that is comes from the
            // scroll position.
            p.size.sizeHint = ftk::Size2I(
                0,
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4 +
                p.size.border);
        }

        void TimelineRuler::drawEvent(
            const math::Box2i& drawRect,
            const ftk::DrawEvent& event)
        {
            IMouseWidget::drawEvent(drawRect, event);
            TLRENDER_P();
            if (!p.player)
                return;

            const math::Box2i& g = getGeometry();

            const int h =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4;
            // No background means an overlay layout is providing it.
            if (p.displayOptions.background != ftk::ColorRole::None)
            {
                event.render->drawRect(
                    math::Box2i(g.min.x, g.min.y, g.w(), h),
                    event.style->getColorRole(ftk::ColorRole::Base));
                event.render->drawRect(
                    math::Box2i(g.min.x, g.min.y + h, g.w(), p.size.border),
                    event.style->getColorRole(ftk::ColorRole::Border));
            }

            // Clipped to the ruler, since the times run the length of the
            // timelines and only this much of them is in view.
            const ftk::ClipRectEnabledState clipEnabledState(event.render);
            const ftk::ClipRectState clipState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(intersect(g, drawRect));

            // Before the in/out points: the two share the strip along the
            // top of the ruler, and the in/out band drawing over a span's
            // band is the right answer -- an applied span reads as the
            // in/out display.
            _drawMarkers(drawRect, event);
            _drawInOutPoints(drawRect, event);
            _drawFrameMarkers(drawRect, event);
            _drawCacheInfo(drawRect, event);
            _drawTimeLabels(drawRect, event);
            _drawTimeTicks(drawRect, event);
            _drawCurrentTime(drawRect, event);
        }

        void TimelineRuler::_drawInOutPoints(
            const math::Box2i& drawRect,
            const ftk::DrawEvent& event)
        {
            TLRENDER_P();
            if (p.inOutRange.has_value() &&
                !compareExact(*p.inOutRange, p.timeRange))
            {
                const math::Box2i& g = getGeometry();
                const ftk::Color4F color(.4F, .5F, .9F);
                const int h = p.size.border * 2;
                switch (p.displayOptions.inOutDisplay)
                {
                case InOutDisplay::InsideRange:
                {
                    const int x0 = timeToPos(p.inOutRange->start_time());
                    const int x1 = timeToPos(p.inOutRange->end_time_exclusive());
                    event.render->drawRect(
                        math::Box2i(x0, g.min.y, x1 - x0 + 1, h), color);
                    break;
                }
                case InOutDisplay::OutsideRange:
                {
                    int x0 = timeToPos(p.timeRange.start_time());
                    int x1 = timeToPos(p.inOutRange->start_time());
                    event.render->drawRect(
                        math::Box2i(x0, g.min.y, x1 - x0 + 1, h), color);
                    x0 = timeToPos(p.inOutRange->end_time_exclusive());
                    x1 = timeToPos(p.timeRange.end_time_exclusive());
                    event.render->drawRect(
                        math::Box2i(x0, g.min.y, x1 - x0 + 1, h), color);
                    break;
                }
                default: break;
                }
            }
        }

        void TimelineRuler::_drawFrameMarkers(
            const math::Box2i& drawRect,
            const ftk::DrawEvent& event)
        {
            TLRENDER_P();
            const math::Box2i& g = getGeometry();
            const double rate = p.timeRange.duration().rate();
            const ftk::Color4F color(.6F, .4F, .2F);
            std::vector<math::Box2i> rects;
            for (const auto& frameMarker : p.frameMarkers)
            {
                const math::Box2i g2(
                    timeToPos(OTIO_NS::RationalTime(frameMarker, rate)),
                    g.min.y,
                    p.size.border * 2,
                    p.size.margin +
                    p.size.fontMetrics.lineHeight +
                    p.size.margin +
                    p.size.border * 4);
                if (ftk::intersects(g2, drawRect))
                {
                    rects.emplace_back(g2);
                }
            }
            if (!rects.empty())
            {
                event.render->drawRects(rects, color);
            }
        }

        void TimelineRuler::_drawMarkers(
            const math::Box2i& drawRect,
            const ftk::DrawEvent& event)
        {
            TLRENDER_P();
            const math::Box2i& g = getGeometry();
            const int h =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4;
            for (const auto& marker : p.markers)
            {
                // A single frame draws as a tick, the shape the frame
                // markers use; a span draws as a band along the top of the
                // ruler, the strip the in/out points use.
                const bool single = marker.range.duration().value() <= 1.0;
                const int x0 = timeToPos(marker.range.start_time());
                const math::Box2i g2 = single ?
                    math::Box2i(
                        x0,
                        g.min.y,
                        p.size.border * 2,
                        h) :
                    math::Box2i(
                        x0,
                        g.min.y,
                        timeToPos(marker.range.end_time_exclusive()) - x0 + 1,
                        p.size.border * 2);
                if (ftk::intersects(g2, drawRect))
                {
                    event.render->drawRect(g2, marker.color);
                }
            }
        }

        void TimelineRuler::_drawCacheInfo(
            const math::Box2i& drawRect,
            const ftk::DrawEvent& event)
        {
            TLRENDER_P();
            const math::Box2i& g = getGeometry();

            if (CacheDisplay::VideoAndAudio == p.displayOptions.cacheDisplay ||
                CacheDisplay::VideoOnly == p.displayOptions.cacheDisplay)
            {
                ftk::TriMesh2F& mesh = p.cacheMesh;
                clearMesh(mesh);
                for (const auto& t : p.cacheInfo.video)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const int h =
                        CacheDisplay::VideoAndAudio == p.displayOptions.cacheDisplay ?
                        p.size.border * 2 :
                        p.size.border * 4;
                    const math::Box2i box(
                        x0,
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin,
                        x1 - x0 + 1,
                        h);
                    if (ftk::intersects(box, drawRect))
                    {
                        addRect(mesh, box);
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(mesh, ftk::Color4F(.2F, .4F, .4F));
                }
            }

            if (CacheDisplay::VideoAndAudio == p.displayOptions.cacheDisplay)
            {
                ftk::TriMesh2F& mesh = p.cacheMesh;
                clearMesh(mesh);
                for (const auto& t : p.cacheInfo.audio)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const math::Box2i box(
                        x0,
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin +
                        p.size.border * 2,
                        x1 - x0 + 1,
                        p.size.border * 2);
                    if (ftk::intersects(box, drawRect))
                    {
                        addRect(mesh, box);
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(mesh, ftk::Color4F(.3F, .25F, .4F));
                }
            }
        }

        void TimelineRuler::_drawTimeLabels(
            const math::Box2i& drawRect,
            const ftk::DrawEvent& event)
        {
            TLRENDER_P();
            if (p.timeRange.duration().value() > 0.0)
            {
                const math::Box2i& g = getGeometry();
                const double rate = p.timeRange.duration().rate();
                const double seconds = _getSecondsInc(event.fontSystem);
                if (seconds > 0.0)
                {
                    const double t0 = std::floor(
                        _posToTime(g.min.x).rescaled_to(1.0).value() / seconds) * seconds;
                    const double t1 = std::ceil(
                        _posToTime(g.max.x).rescaled_to(1.0).value() / seconds) * seconds;
                    for (double t = t0; t <= t1; t += seconds)
                    {
                        const int x = timeToPos(OTIO_NS::RationalTime(t, 1.0));
                        const std::string label = _timeLabel(
                            OTIO_NS::RationalTime(t, 1.0).rescaled_to(rate));
                        event.render->drawText(
                            event.fontSystem->getGlyphs(label, p.size.fontInfo),
                            p.size.fontMetrics,
                            ftk::V2I(
                                x + p.size.border + p.size.margin,
                                g.min.y + p.size.margin),
                            event.style->getColorRole(ftk::ColorRole::TextDisabled));
                    }
                }
            }
        }

        void TimelineRuler::_drawTimeTicks(
            const math::Box2i& drawRect,
            const ftk::DrawEvent& event)
        {
            TLRENDER_P();
            if (p.timeRange.duration().value() > 0.0)
            {
                const math::Box2i& g = getGeometry();
                const double duration = p.timeRange.duration().rescaled_to(1.0).value();
                const int w = duration * p.scale;
                std::vector<math::Box2i> rects;

                // The frame ticks, once a frame is wide enough that they are
                // marks rather than a solid band. Shorter than the second
                // ticks, so the two read apart.
                const int frameTick = 1.0 / p.timeRange.duration().value() * w;
                if (duration > 0.0 && frameTick >= p.size.handle)
                {
                    const OTIO_NS::RationalTime t0 = _posToTime(g.min.x);
                    const OTIO_NS::RationalTime t1 = _posToTime(g.max.x);
                    const OTIO_NS::RationalTime inc(
                        1.0, p.timeRange.duration().rate());
                    for (OTIO_NS::RationalTime t = t0; t <= t1; t += inc)
                    {
                        rects.emplace_back(math::Box2i(
                            timeToPos(t),
                            g.min.y +
                            p.size.margin +
                            p.size.fontMetrics.lineHeight,
                            p.size.border,
                            p.size.margin +
                            p.size.border * 4));
                    }
                }

                const double seconds = _getSecondsInc(event.fontSystem);
                if (duration > 0.0 && seconds > 0.0)
                {
                    const double t0 = std::floor(
                        _posToTime(g.min.x).rescaled_to(1.0).value() / seconds) * seconds;
                    const double t1 = std::ceil(
                        _posToTime(g.max.x).rescaled_to(1.0).value() / seconds) * seconds;
                    for (double t = t0; t <= t1; t += seconds)
                    {
                        rects.emplace_back(math::Box2i(
                            timeToPos(OTIO_NS::RationalTime(t, 1.0)),
                            g.min.y +
                            p.size.margin +
                            p.size.fontMetrics.lineHeight,
                            p.size.border,
                            p.size.margin +
                            p.size.fontMetrics.lineHeight +
                            p.size.margin +
                            p.size.border * 4));
                    }
                }

                if (!rects.empty())
                {
                    event.render->drawRects(
                        rects,
                        event.style->getColorRole(ftk::ColorRole::TextDisabled));
                }
            }
        }

        void TimelineRuler::_drawCurrentTime(
            const math::Box2i& drawRect,
            const ftk::DrawEvent& event)
        {
            TLRENDER_P();
            if (!p.currentTime.has_value())
                return;

            const math::Box2i& g = getGeometry();
            const int x = timeToPos(*p.currentTime);

            event.render->drawRect(
                math::Box2i(x, g.min.y, p.size.border * 2, g.h()),
                event.style->getColorRole(ftk::ColorRole::Red));

            const std::string label = _timeLabel(*p.currentTime);
            ftk::V2I labelPos(
                x + p.size.border * 2 + p.size.margin,
                g.min.y + p.size.margin);
            const ftk::Size2I labelSize =
                event.fontSystem->getSize(label, p.size.fontInfo);
            if (labelPos.x + labelSize.w > g.max.x)
            {
                const ftk::V2I labelPos2(
                    x - p.size.border * 2 - p.size.margin - labelSize.w,
                    g.min.y + p.size.margin);
                if (labelPos2.x > g.min.x)
                {
                    labelPos = labelPos2;
                }
            }
            event.render->drawText(
                event.fontSystem->getGlyphs(label, p.size.fontInfo),
                p.size.fontMetrics,
                labelPos,
                event.style->getColorRole(ftk::ColorRole::Text));
        }
    }
}

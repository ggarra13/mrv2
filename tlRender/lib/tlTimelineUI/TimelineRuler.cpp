// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#include "TimelineRuler.h"

#include <tlTimeline/Util.h>

#include <tlCore/Mesh.h>
#include <tlCore/Time.h>

#include <algorithm>
#include <cmath>

namespace tl
{
    namespace TIMELINEUI
    {
        namespace
        {
            //! Append a box to a mesh (triangle indices are 1-based).
            void addBox(
                geom::TriangleMesh2& mesh, size_t& i, const math::Box2i& box)
            {
                mesh.v.push_back(math::Vector2f(box.min.x, box.min.y));
                mesh.v.push_back(math::Vector2f(box.max.x + 1, box.min.y));
                mesh.v.push_back(math::Vector2f(box.max.x + 1, box.max.y + 1));
                mesh.v.push_back(math::Vector2f(box.min.x, box.max.y + 1));
                mesh.triangles.push_back({i + 0, i + 1, i + 2});
                mesh.triangles.push_back({i + 2, i + 3, i + 0});
                i += 4;
            }

            //! Get the range of multiples of `step` (in seconds) whose
            //! position can be visible, so only those are walked rather than
            //! every tick of a (possibly very long) timeline.
            //!
            //! \param padLeft Width in pixels of what is drawn to the right of
            //! the tick position (so ticks just left of the clip still count).
            void visibleSteps(
                const math::Box2i& clip, int originX, double scale,
                double padLeft, double step, int64_t& first, int64_t& last)
            {
                const double a = (clip.min.x - originX - padLeft) / scale;
                const double b = (clip.max.x - originX) / scale;
                first = static_cast<int64_t>(
                    std::floor(std::max(a, 0.0) / step));
                last = static_cast<int64_t>(std::ceil(b / step));
            }
        } // namespace

        struct TimelineRuler::Private
        {
            std::shared_ptr<timeline::ITimeUnitsModel> timeUnitsModel;
            std::shared_ptr<timeline::Player> player;
            OTIO_NS::TimeRange timeRange;

            ItemOptions itemOptions;
            DisplayOptions displayOptions;

            std::optional<OTIO_NS::TimeRange> inOutRange;
            std::optional<OTIO_NS::RationalTime> currentTime;
            timeline::PlayerCacheInfo cacheInfo;
            std::vector<int> frameMarkers;

            double scale = 500.0;
            int originX = 0;
            math::Box2i viewport;

            bool stopOnScrub = true;
            bool scrubbing = false;
            std::shared_ptr<observer::Value<bool> > scrub;
            std::shared_ptr<observer::Value<OTIO_NS::RationalTime> > timeScrub;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
                int border = 0;
                int handle = 0;
                int height = 0;
                image::FontInfo fontInfo;
                image::FontMetrics fontMetrics;
            };
            SizeData size;

            //! The time labels are shaped once and reused until something
            //! that affects them changes.
            struct LabelCache
            {
                bool valid = false;
                math::Box2i clip;
                int originX = 0;
                double scale = 0.0;
                OTIO_NS::RationalTime currentTime = time::invalidTime;
                std::vector<timeline::TextInfo> textInfos;
            };
            LabelCache labels;

            std::shared_ptr<observer::ValueObserver<bool> > timeUnitsObserver;
            std::shared_ptr<observer::ValueObserver<OTIO_NS::RationalTime> >
                currentTimeObserver;
            std::shared_ptr<observer::ValueObserver<OTIO_NS::TimeRange> >
                timeRangeObserver;
            std::shared_ptr<observer::ValueObserver<OTIO_NS::TimeRange> >
                inOutRangeObserver;
            std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> >
                cacheInfoObserver;
        };

        void TimelineRuler::_init(
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::TIMELINEUI::TimelineRuler", context, parent);
            TLRENDER_P();

            _setMouseHover(true);
            _setMousePress(true, 0, 0);

            p.timeUnitsModel = timeUnitsModel;
            p.scrub = observer::Value<bool>::create(false);
            p.timeScrub =
                observer::Value<OTIO_NS::RationalTime>::create(time::invalidTime);
            p.timeUnitsObserver = observer::ValueObserver<bool>::create(
                timeUnitsModel->observeTimeUnitsChanged(),
                [this](bool) { _timeUnitsUpdate(); });
        }

        TimelineRuler::TimelineRuler() :
            _p(new Private)
        {
        }

        TimelineRuler::~TimelineRuler() {}

        std::shared_ptr<TimelineRuler> TimelineRuler::create(
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineRuler>(new TimelineRuler);
            out->_init(timeUnitsModel, context, parent);
            return out;
        }

        void TimelineRuler::setPlayer(
            const std::shared_ptr<timeline::Player>& value)
        {
            TLRENDER_P();

            p.currentTimeObserver.reset();
            p.timeRangeObserver.reset();
            p.inOutRangeObserver.reset();
            p.cacheInfoObserver.reset();
            p.currentTime.reset();
            p.inOutRange.reset();
            p.cacheInfo = timeline::PlayerCacheInfo();
            p.timeRange = OTIO_NS::TimeRange();

            if (p.scrubbing)
            {
                p.scrubbing = false;
                p.scrub->setIfChanged(false);
            }

            p.player = value;
            p.labels.valid = false;

            if (p.player)
            {
                p.timeRange = p.player->getTimeRange();
                p.currentTimeObserver =
                    observer::ValueObserver<OTIO_NS::RationalTime>::create(
                        p.player->observeCurrentTime(),
                        [this](const OTIO_NS::RationalTime& value)
                        {
                            _p->currentTime = value;
                            _updates |= ui::Update::Draw;
                        });

                p.inOutRangeObserver =
                    observer::ValueObserver<OTIO_NS::TimeRange>::create(
                        p.player->observeInOutRange(),
                        [this](const OTIO_NS::TimeRange& value)
                        {
                            _p->inOutRange = value;
                            _updates |= ui::Update::Draw;
                        });

                p.timeRangeObserver =
                    observer::ValueObserver<OTIO_NS::TimeRange>::create(
                        p.player->observeTimeRange(),
                        [this](const OTIO_NS::TimeRange& value)
                        {
                            _p->timeRange = value;
                            _updates |= ui::Update::Draw;
                        });

                p.cacheInfoObserver =
                    observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                        p.player->observeCacheInfo(),
                        [this](const timeline::PlayerCacheInfo& value)
                        {
                            _p->cacheInfo = value;
                            _updates |= ui::Update::Draw;
                        });
            }
            _updates |= ui::Update::Draw;
            _updates |= ui::Update::Size;
        }

        void TimelineRuler::setTimeRange(const OTIO_NS::TimeRange& value)
        {
            TLRENDER_P();
            if (value == p.timeRange)
                return;
            p.timeRange = value;
            p.labels.valid = false;
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

        void TimelineRuler::setScale(double value)
        {
            TLRENDER_P();
            if (value == p.scale)
                return;
            p.scale = value;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setTimelineOrigin(int value)
        {
            TLRENDER_P();
            if (value == p.originX)
                return;
            p.originX = value;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setViewport(const math::Box2i& value)
        {
            TLRENDER_P();
            if (value == p.viewport)
                return;
            p.viewport = value;
            _geometryUpdate();
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setItemOptions(const ItemOptions& value)
        {
            _p->itemOptions = value;
        }

        void TimelineRuler::setDisplayOptions(const DisplayOptions& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            p.size.sizeInit = true;
            p.labels.valid = false;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::setStopOnScrub(bool value)
        {
            _p->stopOnScrub = value;
        }

        std::shared_ptr<observer::IValue<bool> >
        TimelineRuler::observeScrub() const
        {
            return _p->scrub;
        }

        std::shared_ptr<observer::IValue<OTIO_NS::RationalTime> >
        TimelineRuler::observeTimeScrub() const
        {
            return _p->timeScrub;
        }

        int TimelineRuler::getHeight() const
        {
            return _p->size.height;
        }

        int TimelineRuler::timeToPos(const OTIO_NS::RationalTime& value) const
        {
            TLRENDER_P();
            return p.originX +
                   (value - p.timeRange.start_time()).rescaled_to(1.0).value() *
                       p.scale;
        }

        OTIO_NS::RationalTime TimelineRuler::posToTime(int value) const
        {
            TLRENDER_P();
            if (p.timeRange == time::invalidTimeRange || p.scale <= 0.0)
                return time::invalidTime;

            const double seconds = (value - p.originX) / p.scale;
            OTIO_NS::RationalTime out =
                p.timeRange.start_time() +
                OTIO_NS::RationalTime(seconds, 1.0)
                    .rescaled_to(p.timeRange.duration().rate());
            out = out.floor();

            const OTIO_NS::RationalTime first = p.timeRange.start_time();
            const OTIO_NS::RationalTime last = p.timeRange.end_time_inclusive();
            if (out < first)
                out = first;
            if (out > last)
                out = last;
            return out;
        }

        void TimelineRuler::setGeometry(const math::Box2i&)
        {
            // The geometry is dictated by the viewport, not by the parent.
            _geometryUpdate();
        }

        void TimelineRuler::_geometryUpdate()
        {
            TLRENDER_P();
            const math::Box2i box(
                p.viewport.min.x, p.viewport.min.y, p.viewport.w(),
                p.size.height);
            if (box != _geometry)
            {
                IWidget::setGeometry(box);
            }
        }

        void TimelineRuler::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            const bool displayScaleChanged =
                event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(
                    ui::SizeRole::MarginInside, _displayScale);
                p.size.border = event.style->getSizeRole(
                    ui::SizeRole::Border, _displayScale);
                p.size.handle = event.style->getSizeRole(
                    ui::SizeRole::Handle, _displayScale);
                p.size.fontInfo = image::FontInfo(
                    p.displayOptions.monoFont,
                    p.displayOptions.fontSize * _displayScale);
                p.size.fontMetrics =
                    event.fontSystem->getMetrics(p.size.fontInfo);
                p.labels.valid = false;
            }
            p.size.sizeInit = false;

            // Must match the space TimelineItem reserves above its tracks.
            p.size.height = p.size.margin + p.size.fontMetrics.lineHeight +
                            p.size.margin + p.size.border * 4 + p.size.border;

            // Reserve nothing in the parent layout.
            _sizeHint = math::Size2i();

            _geometryUpdate();
        }

        void TimelineRuler::drawEvent(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            if (p.timeRange == time::invalidTimeRange || p.scale <= 0.0 ||
                p.timeRange.duration().rescaled_to(1.0).value() <= 0.0)
                return;

            // Everything is clipped to where the timeline is visible, which
            // is also what keeps the current time line off the neighbours
            // of the TimelineWidget.
            const math::Box2i clip = math::intersect(p.viewport, drawRect);
            if (clip.w() <= 0 || clip.h() <= 0)
                return;

            const bool clipEnabled = event.render->getClipRectEnabled();
            const math::Box2i clipPrev = event.render->getClipRect();
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(clip);

            const math::Box2i& g = _geometry;
            const int w = p.timeRange.duration().rescaled_to(1.0).value() *
                          p.scale;
            int y = g.min.y;
            int h = p.size.margin + p.size.fontMetrics.lineHeight +
                    p.size.margin + p.size.border * 4;
            event.render->drawRect(
                math::Box2i(p.originX, y, w, h),
                event.style->getColorRole(ui::ColorRole::Window));

            y += h;
            h = p.size.border;
            event.render->drawRect(
                math::Box2i(p.originX, y, w, h),
                event.style->getColorRole(ui::ColorRole::Border));

            _drawInOutPoints(clip, event);   // draws in-out points
            _drawTimeTicks(clip, event);     // draws time ticks
            _drawFrameMarkers(clip, event);  // draws optional markers
            _drawTimeLabels(clip, event);    // draws labels next to time ticks
            _drawCacheInfo(clip, event);     // draws audio and video cache
            _drawCurrentTime(clip, event);   // draws red line and frame text

            event.render->setClipRect(clipPrev);
            event.render->setClipRectEnabled(clipEnabled);
        }

        void TimelineRuler::mouseMoveEvent(ui::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            TLRENDER_P();
            if (p.scrubbing)
            {
                _scrub(event.pos.x);
            }
        }

        void TimelineRuler::mousePressEvent(ui::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            if (p.player && p.itemOptions.inputEnabled && 0 == event.button &&
                0 == event.modifiers)
            {
                p.scrubbing = true;
                if (p.stopOnScrub)
                {
                    p.player->setPlayback(timeline::Playback::Stop);
                }
                _scrub(event.pos.x);
            }
        }

        void TimelineRuler::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            TLRENDER_P();
            p.scrubbing = false;
            p.scrub->setIfChanged(false);
        }

        void TimelineRuler::_releaseMouse()
        {
            IWidget::_releaseMouse();
            TLRENDER_P();
            p.scrubbing = false;
            p.scrub->setIfChanged(false);
        }

        void TimelineRuler::_timeUnitsUpdate()
        {
            TLRENDER_P();
            p.labels.valid = false;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void TimelineRuler::_scrub(int x)
        {
            TLRENDER_P();
            const OTIO_NS::RationalTime time = posToTime(x);
            if (time.is_invalid_time())
                return;
            p.scrub->setIfChanged(true);
            p.timeScrub->setIfChanged(time);
            p.player->seek(time);
        }

        void TimelineRuler::_drawInOutPoints(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();
            if (p.inOutRange.has_value() &&
                !time::compareExact(*p.inOutRange, p.timeRange))
            {
                const int y = _geometry.min.y;
                const int h = p.size.margin + p.size.fontMetrics.lineHeight +
                              p.size.margin;
                const auto color =
                    event.style->getColorRole(ui::ColorRole::InOut);

                switch (p.displayOptions.inOutDisplay)
                {
                case InOutDisplay::InsideRange:
                {
                    const int x0 = timeToPos(p.inOutRange->start_time());
                    const int x1 =
                        timeToPos(p.inOutRange->end_time_exclusive());
                    event.render->drawRect(
                        math::Box2i(x0, y, x1 - x0 + 1, h), color);
                    break;
                }
                case InOutDisplay::OutsideRange:
                {
                    int x0 = timeToPos(p.timeRange.start_time());
                    int x1 = timeToPos(p.inOutRange->start_time());
                    event.render->drawRect(
                        math::Box2i(x0, y, x1 - x0 + 1, h), color);
                    x0 = timeToPos(p.inOutRange->end_time_exclusive());
                    x1 = timeToPos(p.timeRange.end_time_exclusive());
                    event.render->drawRect(
                        math::Box2i(x0, y, x1 - x0 + 1, h), color);
                    break;
                }
                default:
                    break;
                }
            }
        }

        math::Size2i TimelineRuler::_getLabelMaxSize(
            const std::shared_ptr<image::FontSystem>& fontSystem) const
        {
            TLRENDER_P();
            const std::string labelMax =
                p.timeUnitsModel->getLabel(p.timeRange.duration());
            return fontSystem->getSize(labelMax, p.size.fontInfo);
        }

        void TimelineRuler::_getTimeTicks(
            const std::shared_ptr<image::FontSystem>& fontSystem,
            double& seconds, int& tick) const
        {
            TLRENDER_P();
            // Pixels between the ticks of each unit.
            const int secondsTick = p.scale;
            const int minutesTick = 60.0 * p.scale;
            const int hoursTick = 3600.0 * p.scale;
            const math::Size2i labelMaxSize = _getLabelMaxSize(fontSystem);
            const int distanceMin =
                p.size.border + p.size.margin + labelMaxSize.w;
            seconds = 0.0;
            tick = 0;
            if (secondsTick >= distanceMin)
            {
                seconds = 1.0;
                tick = secondsTick;
            }
            else if (minutesTick >= distanceMin)
            {
                seconds = 60.0;
                tick = minutesTick;
            }
            else if (hoursTick >= distanceMin)
            {
                seconds = 3600.0;
                tick = hoursTick;
            }
        }

        void TimelineRuler::_drawTimeTicks(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();
            const int y0 = _geometry.min.y;
            const double duration =
                p.timeRange.duration().rescaled_to(1.0).value();
            const double rate = p.timeRange.duration().rate();

            // One tick per frame, when they are far enough apart.
            const int frameTick = p.scale / rate;
            if (frameTick >= p.size.handle)
            {
                const double step = 1.0 / rate;
                int64_t first = 0;
                int64_t last = 0;
                visibleSteps(
                    drawRect, p.originX, p.scale, p.size.border, step, first,
                    last);
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (int64_t n = first; n <= last && n * step < duration; ++n)
                {
                    const math::Box2i box(
                        p.originX + n * step * p.scale,
                        y0 + p.size.margin + p.size.fontMetrics.lineHeight,
                        p.size.border, p.size.margin + p.size.border * 4);
                    if (math::intersects(box, drawRect))
                    {
                        addBox(mesh, i, box);
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh, math::Vector2i(),
                        event.style->getColorRole(ui::ColorRole::Button));
                }
            }

            // One tick per second, minute or hour.
            double seconds = 0;
            int tick = 0;
            _getTimeTicks(event.fontSystem, seconds, tick);
            if (seconds > 0.0 && tick > 0)
            {
                int64_t first = 0;
                int64_t last = 0;
                visibleSteps(
                    drawRect, p.originX, p.scale, p.size.border, seconds,
                    first, last);
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (int64_t n = first; n <= last && n * seconds < duration;
                     ++n)
                {
                    const math::Box2i box(
                        p.originX + n * seconds * p.scale, y0, p.size.border,
                        p.size.margin + p.size.fontMetrics.lineHeight +
                            p.size.margin + p.size.border * 4);
                    if (math::intersects(box, drawRect))
                    {
                        addBox(mesh, i, box);
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh, math::Vector2i(),
                        event.style->getColorRole(ui::ColorRole::Button),
                        "timeticks");
                }
            }
        }

        void TimelineRuler::_drawFrameMarkers(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();
            const double rate = p.timeRange.duration().rate();
            for (const auto& frameMarker : p.frameMarkers)
            {
                const math::Box2i g2(
                    timeToPos(OTIO_NS::RationalTime(frameMarker, rate)),
                    _geometry.min.y, p.size.border * 2,
                    p.size.margin + p.size.fontMetrics.lineHeight +
                        p.size.margin + p.size.border * 4);
                if (math::intersects(g2, drawRect))
                {
                    event.render->drawRect(
                        g2,
                        event.style->getColorRole(ui::ColorRole::FrameMarker));
                }
            }
        }

        void TimelineRuler::_drawTimeLabels(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();
            double seconds = 0;
            int tick = 0;
            _getTimeTicks(event.fontSystem, seconds, tick);
            if (seconds <= 0.0 || tick <= 0)
                return;

            auto& cache = p.labels;
            if (!cache.valid || cache.clip != drawRect ||
                cache.originX != p.originX || cache.scale != p.scale ||
                cache.currentTime != p.currentTime)
            {
                cache.textInfos.clear();
                cache.valid = true;
                cache.clip = drawRect;
                cache.originX = p.originX;
                cache.scale = p.scale;
                cache.currentTime = *p.currentTime;

                const double duration =
                    p.timeRange.duration().rescaled_to(1.0).value();
                const math::Size2i labelMaxSize =
                    _getLabelMaxSize(event.fontSystem);
                int64_t first = 0;
                int64_t last = 0;
                visibleSteps(
                    drawRect, p.originX, p.scale,
                    p.size.border + p.size.margin + labelMaxSize.w, seconds,
                    first, last);
                for (int64_t n = first; n <= last && n * seconds < duration;
                     ++n)
                {
                    const double t = n * seconds;
                    const OTIO_NS::RationalTime time =
                        p.timeRange.start_time() +
                        OTIO_NS::RationalTime(t, 1.0).rescaled_to(
                            p.timeRange.duration().rate());
                    const math::Box2i box(
                        p.originX + t * p.scale + p.size.border + p.size.margin,
                        _geometry.min.y + p.size.margin, labelMaxSize.w,
                        p.size.fontMetrics.lineHeight);
                    if (time != p.currentTime &&
                        math::intersects(box, drawRect))
                    {
                        const std::string label =
                            p.timeUnitsModel->getLabel(time);
                        event.render->appendText(
                            cache.textInfos,
                            event.fontSystem->getGlyphs(
                                label, p.size.fontInfo),
                            math::Vector2i(
                                box.min.x,
                                box.min.y + p.size.fontMetrics.ascender));
                    }
                }
            }
            for (const auto& textInfo : cache.textInfos)
            {
                event.render->drawText(
                    textInfo, math::Vector2i(),
                    event.style->getColorRole(ui::ColorRole::TextDisabled));
            }
        }

        void TimelineRuler::_drawCacheInfo(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();
            const int y0 = _geometry.min.y;
            const CacheDisplay display = p.displayOptions.cacheDisplay;

            if (CacheDisplay::VideoAndAudio == display ||
                CacheDisplay::VideoOnly == display)
            {
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.videoFrames)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const int h = CacheDisplay::VideoAndAudio == display
                                      ? p.size.border * 2
                                      : p.size.border * 4;
                    const math::Box2i box(
                        x0,
                        y0 + p.size.margin + p.size.fontMetrics.lineHeight +
                            p.size.margin,
                        x1 - x0 + 1, h);
                    if (math::intersects(box, drawRect))
                    {
                        addBox(mesh, i, box);
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh, math::Vector2i(),
                        event.style->getColorRole(ui::ColorRole::VideoCache),
                        "video cache");
                }
            }

            if (CacheDisplay::VideoAndAudio == display)
            {
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.audioFrames)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const math::Box2i box(
                        x0,
                        y0 + p.size.margin + p.size.fontMetrics.lineHeight +
                            p.size.margin + p.size.border * 2,
                        x1 - x0 + 1, p.size.border * 2);
                    if (math::intersects(box, drawRect))
                    {
                        addBox(mesh, i, box);
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh, math::Vector2i(),
                        event.style->getColorRole(ui::ColorRole::AudioCache),
                        "audio cache");
                }
            }
        }

        void TimelineRuler::_drawCurrentTime(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();
            if (!p.currentTime.has_value())
                return;

            const math::Vector2i pos(
                timeToPos(*p.currentTime), _geometry.min.y);

            // The line runs down over the tracks, to the bottom of the
            // viewport.
            event.render->setClipRectEnabled(false);
            event.render->drawRect(
                math::Box2i(
                    pos.x, pos.y, p.size.border * 2,
                    p.viewport.max.y - pos.y + 1),
                event.style->getColorRole(ui::ColorRole::Red));
            event.render->setClipRectEnabled(true);

            const std::string label =
                p.timeUnitsModel->getLabel(*p.currentTime);

            math::Vector2i labelPos(
                pos.x + p.size.border * 2 + p.size.margin,
                pos.y + p.size.margin + p.size.fontMetrics.ascender);

            // Re-position the time label to the left of the line if it would
            // go out of the visible area.
            const math::Size2i labelSize =
                event.fontSystem->getSize(label, p.size.fontInfo);
            if (labelPos.x + labelSize.w > p.viewport.max.x)
            {
                const math::Vector2i labelPos2(
                    pos.x - p.size.border * 2 - p.size.margin - labelSize.w,
                    pos.y + p.size.margin + p.size.fontMetrics.ascender);
                if (labelPos2.x > p.viewport.min.x)
                {
                    labelPos = labelPos2;
                }
            }

            std::vector<timeline::TextInfo> textInfos;
            event.render->appendText(
                textInfos,
                event.fontSystem->getGlyphs(label, p.size.fontInfo), labelPos);
            for (const auto& textInfo : textInfos)
            {
                event.render->drawText(
                    textInfo, math::Vector2i(),
                    event.style->getColorRole(ui::ColorRole::Text));
            }
        }
    } // namespace TIMELINEUI
} // namespace tl

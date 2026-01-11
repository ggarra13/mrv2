// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#include "TimelineItemPrivate.h"

#include "AudioClipItem.h"
#include "GapItem.h"
#include "VideoClipItem.h"
#include "EffectItem.h"

#include <tlUI/DrawUtil.h>
#include <tlUI/ScrollArea.h>

#include <tlTimeline/Edit.h>
#include <tlTimeline/Util.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace TIMELINEUI
    {
#ifdef OPENGL_BACKEND
        void TimelineItem::_init(
            const std::shared_ptr<timeline::Player>& player,
            const otio::SerializableObject::Retainer<otio::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<gl::GLFWWindow>& window,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
#endif

#ifdef VULKAN_BACKEND
        void TimelineItem::_init(
            const std::shared_ptr<timeline::Player>& player,
            const otio::SerializableObject::Retainer<otio::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
#endif
        {
            const otime::TimeRange timeRange = player->getTimeRange();
            const otime::TimeRange trimmedRange(
                otime::RationalTime(0.0, timeRange.duration().rate()),
                timeRange.duration());
            IItem::_init(
                "tl::TIMELINEUI::TimelineItem", timeRange, trimmedRange, scale,
                options, displayOptions, itemData, context, parent);
            TLRENDER_P();

            _setMouseHover(true);
            _setMousePress(true, 0, 0);

            p.player = player;

            p.scrub = observer::Value<bool>::create(false);
            p.timeScrub =
                observer::Value<otime::RationalTime>::create(time::invalidTime);

#ifdef OPENGL_BACKEND
            p.thumbnailGenerator = TIMELINEUI::ThumbnailGenerator::create(
                context->getSystem<TIMELINEUI::ThumbnailSystem>()->getCache(), context,
                window);
#endif

#ifdef VULKAN_BACKEND
            if (!context->getSystem<timelineui_vk::ThumbnailSystem>())
            {
                context->addSystem(timelineui_vk::ThumbnailSystem::create(context, ctx));
            }
                
            p.thumbnailGenerator = timelineui_vk::ThumbnailGenerator::create(
                context->getSystem<timelineui_vk::ThumbnailSystem>()->getCache(), context,
                ctx);
#endif

            const auto otioTimeline = p.player->getTimeline()->getTimeline();
            for (const auto& child : otioTimeline->tracks()->children())
            {
                if (auto otioTrack =
                        otio::dynamic_retainer_cast<otio::Track>(child))
                {
                    if (!otioTrack->enabled())
                        continue;
                    Private::Track track;
                    int otioIndex = 0;
                    track.index = p.tracks.size();
                    std::string trackLabel = otioTrack->name();
                    if (otio::Track::Kind::video == otioTrack->kind())
                    {
                        track.type = TrackType::Video;
                        if (trackLabel.empty())
                        {
                            trackLabel = "Video Track";
                        }
                    }
                    else if (otio::Track::Kind::audio == otioTrack->kind())
                    {
                        track.type = TrackType::Audio;
                        if (trackLabel.empty())
                        {
                            trackLabel = "Audio Track";
                        }
                    }
                    track.timeRange = otioTrack->trimmed_range();
                    track.label = ui::Label::create(
                        trackLabel, context, shared_from_this());
                    track.label->setMarginRole(ui::SizeRole::MarginInside);
                    track.durationLabel =
                        ui::Label::create(context, shared_from_this());
                    track.durationLabel->setMarginRole(
                        ui::SizeRole::MarginInside);

                    for (const auto& child : otioTrack->children())
                    {
                        if (auto clip =
                                otio::dynamic_retainer_cast<otio::Clip>(child))
                        {
                            switch (track.type)
                            {
                            case TrackType::Video:
                                track.items.push_back(VideoClipItem::create(
                                    clip, scale, options, displayOptions,
                                    itemData, p.thumbnailGenerator, context,
                                    shared_from_this()));
                                break;
                            case TrackType::Audio:
                                track.items.push_back(AudioClipItem::create(
                                    clip, scale, options, displayOptions,
                                    itemData, p.thumbnailGenerator, context,
                                    shared_from_this()));
                                break;
                            default:
                                break;
                            }
                            track.otioIndexes.push_back(otioIndex);

                            auto effects = clip->effects();
                            std::vector<std::shared_ptr<EffectItem> > effectItems;

                            otio::Item* item = clip;
                            
                            effectItems.reserve(effects.size());
                            for (const auto effect : effects)
                            {
                                effectItems.push_back(EffectItem::create(
                                                          effect, item, scale, options, displayOptions,
                                                          itemData, context, shared_from_this()));
                            }

                            if (!effectItems.empty())
                            {
                                track.effects[otioIndex] = effectItems;
                            }
                        }
                        else if (
                            auto gap =
                                otio::dynamic_retainer_cast<otio::Gap>(child))
                        {
                            track.items.push_back(GapItem::create(
                                TrackType::Video == track.type
                                    ? ui::ColorRole::VideoGap
                                    : ui::ColorRole::AudioGap,
                                gap, scale, options, displayOptions, itemData,
                                context, shared_from_this()));
                            track.otioIndexes.push_back(otioIndex);
                        }
                        else if (
                            auto transition =
                                otio::dynamic_retainer_cast<otio::Transition>(
                                    child))
                        {
                            track.transitions.push_back(TransitionItem::create(
                                transition, scale, options, displayOptions,
                                itemData, context, shared_from_this()));
                            track.otioTransitionIndexes.push_back(otioIndex);
                        }
                        ++otioIndex;
                    }

                    p.tracks.push_back(track);
                }
            }

            _tracksUpdate();
            _textUpdate();

            p.currentTimeObserver =
                observer::ValueObserver<otime::RationalTime>::create(
                    p.player->observeCurrentTime(),
                    [this](const otime::RationalTime& value)
                    {
                        _p->currentTime = value;
                        _updates |= ui::Update::Draw;
                    });

            p.inOutRangeObserver =
                observer::ValueObserver<otime::TimeRange>::create(
                    p.player->observeInOutRange(),
                    [this](const otime::TimeRange value)
                    {
                        _p->inOutRange = value;
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

        TimelineItem::~TimelineItem() {}

#ifdef OPENGL_BACKEND
        TimelineItem::TimelineItem() :
            _p(new Private)
        {
        }

        std::shared_ptr<TimelineItem> TimelineItem::create(
            const std::shared_ptr<timeline::Player>& player,
            const otio::SerializableObject::Retainer<otio::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<gl::GLFWWindow>& window,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineItem>(new TimelineItem);
            out->_init(
                player, stack, scale, options, displayOptions, itemData, window,
                context, parent);
            return out;
        }
#endif

#ifdef VULKAN_BACKEND
        TimelineItem::TimelineItem(Fl_Vk_Context& ctx) :
            ctx(ctx),
            _p(new Private)
        {
        }

        std::shared_ptr<TimelineItem> TimelineItem::create(
            const std::shared_ptr<timeline::Player>& player,
            const otio::SerializableObject::Retainer<otio::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            Fl_Vk_Context& ctx,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineItem>(new TimelineItem(ctx));
            out->_init(
                player, stack, scale, options, displayOptions, itemData,
                context, parent);
            return out;
        }
#endif

        void TimelineItem::setEditable(bool value)
        {
            _p->editable = value;
        }
        
        void TimelineItem::setEditMode(const timeline::EditMode value)
        {
            _p->editMode = value;
        }

        void TimelineItem::setStopOnScrub(bool value)
        {
            _p->stopOnScrub = value;
        }

        std::shared_ptr<observer::IValue<bool> >
        TimelineItem::observeScrub() const
        {
            return _p->scrub;
        }

        std::shared_ptr<observer::IValue<otime::RationalTime> >
        TimelineItem::observeTimeScrub() const
        {
            return _p->timeScrub;
        }

        void TimelineItem::setFrameMarkers(const std::vector<int>& value)
        {
            TLRENDER_P();
            if (value == p.frameMarkers)
                return;
            p.frameMarkers = value;
            _updates |= ui::Update::Draw;
        }

        int TimelineItem::getMinimumHeight() const
        {
            return _p->minimumHeight;
        }

        void TimelineItem::setDisplayOptions(const DisplayOptions& value)
        {
            const bool changed = value != _displayOptions;
            IItem::setDisplayOptions(value);
            TLRENDER_P();
            if (changed)
            {
                p.size.sizeInit = true;
                _tracksUpdate();
            }
        }

        void TimelineItem::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();

            const math::Box2i& g = _geometry;
            float y = p.size.margin + p.size.fontMetrics.lineHeight +
                      p.size.margin + p.size.border * 4 + p.size.border +
                      g.min.y;
            for (const auto& track : p.tracks)
            {
                const bool visible = _isTrackVisible(track.index);

                math::Size2i labelSizeHint;
                if (visible && _displayOptions.trackInfo)
                {
                    labelSizeHint = track.label->getSizeHint();
                }
                track.label->setGeometry(
                    math::Box2i(g.min.x, y, labelSizeHint.w, labelSizeHint.h));
                math::Size2i durationSizeHint;
                if (visible && _displayOptions.trackInfo)
                {
                    durationSizeHint = track.durationLabel->getSizeHint();
                }
                track.durationLabel->setGeometry(math::Box2i(
                    g.min.x + track.size.w - durationSizeHint.w, y,
                    durationSizeHint.w, durationSizeHint.h));

                for (const auto& item : track.items)
                {
                    const auto i = std::find_if(
                        p.mouse.items.begin(), p.mouse.items.end(),
                        [item](const std::shared_ptr<Private::MouseItemData>&
                                   value) { return item == value->p; });
                    if (i != p.mouse.items.end() &&
                        p.editMode != timeline::EditMode::Select)
                    {
                        continue;
                    }
                    const otime::TimeRange& timeRange = item->getTimeRange();
                    math::Size2i sizeHint;
                    if (visible)
                    {
                        sizeHint = item->getSizeHint();
                    }
                    item->setGeometry(math::Box2i(
                        _geometry.min.x +
                            timeRange.start_time().rescaled_to(1.0).value() *
                                _scale,
                        y + std::max(labelSizeHint.h, durationSizeHint.h),
                        sizeHint.w, track.clipHeight));

                    for (auto index : track.otioIndexes)
                    {
                        auto i = track.effects.find(index);
                        if (i == track.effects.end())
                            continue;

                        auto effects = i->second;
                        int effectsY = y;
                        for (const auto& effect : effects)
                        {
                            const math::Size2i& sizeHint = effect->getSizeHint();
                        
                            const otime::TimeRange& timeRange =
                                effect->getTimeRange();
                            effect->setGeometry(math::Box2i(
                                                    _geometry.min.x + timeRange.start_time()
                                                    .rescaled_to(1.0)
                                                    .value() *
                                                    _scale,
                                                    effectsY, sizeHint.w, sizeHint.h));

                            effectsY += sizeHint.h;
                        }
                    }
                }

                if (visible)
                {
                    y += track.size.h;

                    int transitionH = 0;
                    for (const auto& item : track.transitions)
                    {
                        const math::Size2i& sizeHint = item->getSizeHint();
                        transitionH = sizeHint.h;
                        
                        const auto i = std::find_if(
                            p.mouse.items.begin(), p.mouse.items.end(),
                            [item](const std::shared_ptr<Private::MouseItemData>&
                                   value) { return item == value->p; });
                        if (i != p.mouse.items.end() &&
                            p.editMode != timeline::EditMode::Select)
                        {
                            continue;
                        }
                        const otime::TimeRange& timeRange =
                            item->getTimeRange();
                        item->setGeometry(math::Box2i(
                            _geometry.min.x + timeRange.start_time()
                                                      .rescaled_to(1.0)
                                                      .value() *
                                                  _scale,
                            y, sizeHint.w, sizeHint.h));
                    }

                    y += transitionH;
                }
            }

            if (auto scrollArea = getParentT<ui::ScrollArea>())
            {
                p.size.scrollPos = scrollArea->getScrollPos();
            }
            p.size.textInfos.clear();
        }

        void TimelineItem::sizeHintEvent(const ui::SizeHintEvent& event)
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
                p.size.handle = event.style->getSizeRole(
                    ui::SizeRole::Handle, _displayScale);
                p.size.fontInfo = image::FontInfo(
                    _displayOptions.monoFont,
                    _displayOptions.fontSize * _displayScale);
                p.size.fontMetrics =
                    event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textInfos.clear();
            }
            p.size.sizeInit = false;

            int tracksHeight = 0;
            bool minimumTrackHeightInit = true;
            int minimumTrackHeight = 0;
            for (int i = 0; i < p.tracks.size(); ++i)
            {
                auto& track = p.tracks[i];
                const bool visible = _isTrackVisible(track.index);

                track.size.w =
                    track.timeRange.duration().rescaled_to(1.0).value() *
                    _scale;
                track.size.h = 0;
                track.clipHeight = 0;
                if (visible)
                {
                    for (const auto& item : track.items)
                    {
                        const math::Size2i& sizeHint = item->getSizeHint();
                        track.size.h = std::max(track.size.h, sizeHint.h);
                    }
                    track.clipHeight = track.size.h;
                    if (_displayOptions.trackInfo)
                    {
                        track.size.h += std::max(
                            track.label->getSizeHint().h,
                            track.durationLabel->getSizeHint().h);
                    }
                    tracksHeight += track.size.h;
                    if (minimumTrackHeightInit)
                    {
                        minimumTrackHeightInit = false;
                        minimumTrackHeight = track.size.h;
                    }
                }
            }

            _sizeHint = math::Size2i(
                _timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.margin + p.size.fontMetrics.lineHeight + p.size.margin +
                    p.size.border * 4 + p.size.border + tracksHeight);

            p.minimumHeight = p.size.margin + p.size.fontMetrics.lineHeight +
                              p.size.margin + p.size.border * 4 +
                              p.size.border + minimumTrackHeight;
        }

        void TimelineItem::drawOverlayEvent(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            IItem::drawOverlayEvent(drawRect, event);
            TLRENDER_P();
            
            const math::Box2i& g = _geometry;

            int y = p.size.scrollPos.y + g.min.y;
            int h = p.size.margin + p.size.fontMetrics.lineHeight +
                    p.size.margin + p.size.border * 4;
            event.render->drawRect(
                math::Box2i(g.min.x, y, g.w(), h),
                event.style->getColorRole(ui::ColorRole::Window));

            y = y + h;
            h = p.size.border;
            event.render->drawRect(
                math::Box2i(g.min.x, y, g.w(), h),
                event.style->getColorRole(ui::ColorRole::Border));

            _drawInOutPoints(drawRect, event); // draws in-out points
            _drawTimeTicks(drawRect, event);   // draws time ticks
            _drawFrameMarkers(drawRect, event);  // draws optional markers
            _drawTimeLabels(drawRect, event);  // draws labels next to time ticks
            _drawCacheInfo(drawRect, event);     // draws audio and video cache
            _drawCurrentTime(drawRect, event);   // draws red line and frame text

            if (p.mouse.currentDropTarget >= 0 &&
                p.mouse.currentDropTarget < p.mouse.dropTargets.size())
            {
                const auto& dt = p.mouse.dropTargets[p.mouse.currentDropTarget];
                event.render->drawRect(
                    dt.draw, event.style->getColorRole(ui::ColorRole::Green));
            }
        }

        void TimelineItem::mouseMoveEvent(ui::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            TLRENDER_P();
            switch(p.editMode)
            {
            case timeline::EditMode::kNone:
            case timeline::EditMode::Select:
                return;
            case timeline::EditMode::Fill:
                _mouseMoveEventFill(event);
                break;
            case timeline::EditMode::Insert:
                break;
            case timeline::EditMode::Move:
                _mouseMoveEventMove(event);
                break;
            case timeline::EditMode::Overwrite:
                break;
            case timeline::EditMode::Ripple:
                _mouseMoveEventRipple(event);
                break;
            case timeline::EditMode::Roll:
                _mouseMoveEventRoll(event);
                break;
            case timeline::EditMode::Slice:
                _mouseMoveEventSlice(event);
                break;
            case timeline::EditMode::Slide:
                _mouseMoveEventSlide(event);
                break;
            case timeline::EditMode::Slip:
                _mouseMoveEventSlip(event);
                break;
            case timeline::EditMode::Trim:
                _mouseMoveEventTrim(event);
                break;
            default:
                throw std::runtime_error("Unhandled Edit Mode");
            }
        }
            
        void TimelineItem::mousePressEvent(ui::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            bool saveUndo = false;
            bool doSelection = false;
            if (p.editMode == timeline::EditMode::Select)
            {
                doSelection = true;
                if (0 == event.button)
                {
                    if (0 == event.modifiers)
                    {
                        p.mouse.items.clear();
                    }
                }
            }
            else
            {
                if (_options.inputEnabled && 0 == event.button &&
                    0 == event.modifiers)
                    doSelection = true;
            }
            
            if (p.editMode == timeline::EditMode::Slip)
            {
                saveUndo = true;
            }
            
            if (doSelection)
            {
                takeKeyFocus();

                p.mouse.mode = Private::MouseMode::kNone;
                p.mouse.side = Private::MouseClick::Center;

                const math::Box2i& g = _geometry;
                if (p.editable)
                {
                    for (int i = 0; i < p.tracks.size(); ++i)
                    {
                        if (_isTrackVisible(i))
                        {
                            const auto& items = p.tracks[i].items;
                            for (int j = 0; j < items.size(); ++j)
                            {
                                const auto& item = items[j];
                                if (item->getGeometry().contains(event.pos))
                                {
                                    p.mouse.mode = Private::MouseMode::Item;
                                    p.mouse.items.push_back(
                                        std::make_shared<
                                            Private::MouseItemData>(
                                            item, j, i));
                                    p.mouse.dropTargets =
                                        p.getDropTargets(g, j, i);
                                    moveToFront(item);
                                    if (_options.editAssociatedClips)
                                    {
                                        if (auto associated =
                                                p.getAssociated(item, j, i))
                                        {
                                            p.mouse.items.push_back(
                                                std::make_shared<
                                                    Private::MouseItemData>(
                                                    associated, j, i));
                                            moveToFront(associated);
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        if (!p.mouse.items.empty())
                        {
                            break;
                        }
                    }
                }

                if (p.mouse.items.empty())
                {
                    for (int i = 0; i < p.tracks.size(); ++i)
                    {
                        if (_isTrackVisible(i))
                        {
                            const auto& transitions = p.tracks[i].transitions;
                            for (int j = 0; j < transitions.size(); ++j)
                            {
                                const auto& item = transitions[j];
                                if (item->getGeometry().contains(event.pos))
                                {
                                    p.mouse.mode = Private::MouseMode::Transition;
                                    p.mouse.items.push_back(
                                        std::make_shared<
                                            Private::MouseItemData>(
                                            item, j, i));
                                    moveToFront(item);
                                    break;
                                }
                            }
                        }
                        if (!p.mouse.items.empty())
                        {
                            break;
                        }
                    }
                }
                
                if (p.mouse.items.empty())
                {
                    p.mouse.mode = Private::MouseMode::CurrentTime;
                    if (p.stopOnScrub)
                    {
                        p.player->setPlayback(timeline::Playback::Stop);
                    }
                    const otime::RationalTime time = posToTime(event.pos.x);
                    p.scrub->setIfChanged(true);
                    p.timeScrub->setIfChanged(time);
                    p.player->seek(time);
                }
                else
                {
                    const math::Box2i& g = p.mouse.items[0]->geometry;                    
                    const int midpoint = (g.x() + g.w() / 2);
                    if (_mouse.pressPos.x < midpoint)
                    {
                        p.mouse.side = Private::MouseClick::Left;
                    }
                    else
                    {
                        p.mouse.side = Private::MouseClick::Right;
                    }
                }
            }

            if (saveUndo)
            {
                 _storeUndo();
            }
        }

        void TimelineItem::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            TLRENDER_P();
            p.scrub->setIfChanged(false);
            switch(p.editMode)
            {
            case timeline::EditMode::kNone:
            case timeline::EditMode::Select:
                return;
            case timeline::EditMode::Fill:
                _mouseReleaseEventFill(event);
                break;
            case timeline::EditMode::Insert:
                break;
            case timeline::EditMode::Move:
                _mouseReleaseEventMove(event);
                break;
            case timeline::EditMode::Overwrite:
                break;
            case timeline::EditMode::Ripple:
                _mouseReleaseEventRipple(event);
                break;
            case timeline::EditMode::Roll:
                _mouseReleaseEventRoll(event);
                break;
            case timeline::EditMode::Slice:
                _mouseReleaseEventSlice(event);
                break;
            case timeline::EditMode::Slide:
                _mouseReleaseEventSlide(event);
                break;
            case timeline::EditMode::Slip:
                _mouseReleaseEventSlip(event);
                break;
            case timeline::EditMode::Trim:
                _mouseReleaseEventTrim(event);
                break;
            default:
                throw std::runtime_error("Unhandled Edit Mode");
            }
            p.mouse.items.clear();
            p.mouse.mode = Private::MouseMode::kNone;
        }

        bool TimelineItem::isDraggingClip() const
        {
            TLRENDER_P();
            return (p.mouse.mode == Private::MouseMode::Item &&
                    p.editMode == timeline::EditMode::Move &&
                    !p.mouse.items.empty());
        }

        void TimelineItem::setMoveCallback(
            const std::function<void(const std::vector<timeline::MoveData>&)>&
                value)
        {
            _p->moveCallback = value;
        }

        /*void TimelineItem::keyPressEvent(ui::KeyEvent& event)
        {
            TLRENDER_P();
            if (isEnabled() &&
                _options.inputEnabled &&
                0 == event.modifiers)
            {
                switch (event.key)
                {
                default: break;
                }
            }
        }

        void TimelineItem::keyReleaseEvent(ui::KeyEvent& event)
        {
            event.accept = true;
        }*/

        void TimelineItem::_addOneFrameGap(const otime::RationalTime& videoTime,
                                           otime::TimeRange& timeRange)
        {
            otime::RationalTime one_frame(1.0, videoTime.rate());
            const otime::RationalTime& startTime =
                timeRange.start_time() + one_frame;
            const otime::RationalTime& duration =
                timeRange.duration() - one_frame;
            timeRange = otime::TimeRange(startTime, duration);
        }
        
        void TimelineItem::_getTransitionItems(std::vector<IBasicItem*>& items,
                                               const int trackNumber,
                                               const otime::TimeRange& transitionRange)
        {
            TLRENDER_P();
            
            for (const auto& item : p.tracks[trackNumber].items)
            {
                IBasicItem* clip = static_cast<IBasicItem*>(item.get());
                otime::TimeRange itemRange = clip->getTimeRange();
                if (itemRange.intersects(transitionRange))
                {
                    items.push_back(clip);
                }
            }
        }
        
        void TimelineItem::_getTransitionTimeRanges(std::vector<otime::TimeRange>& timeRanges,
                                                    const int trackNumber,
                                                    const otime::TimeRange& transitionRange)
        {
            TLRENDER_P();
            
            std::vector<IBasicItem*> items;
            _getTransitionItems(items, trackNumber, transitionRange);

            for (const auto& item : items)
            {
                otime::TimeRange itemRange = item->getTimeRange();
                timeRanges.push_back(itemRange);
            }

            if (timeRanges.size() != 2)
                throw std::runtime_error("Broken transition in otio file");
        }
        
        void TimelineItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void TimelineItem::_releaseMouse()
        {
            TLRENDER_P();
            IWidget::_releaseMouse();
            p.mouse.items.clear();
        }

        bool TimelineItem::_isTrackVisible(int index) const
        {
            return _displayOptions.tracks.empty() ||
                   std::find(
                       _displayOptions.tracks.begin(),
                       _displayOptions.tracks.end(),
                       index) != _displayOptions.tracks.end();
        }

        void TimelineItem::_drawInOutPoints(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();
            if (!time::compareExact(_p->inOutRange, time::invalidTimeRange) &&
                !time::compareExact(_p->inOutRange, _timeRange))
            {
                const math::Box2i& g = _geometry;

                switch (_displayOptions.inOutDisplay)
                {
                case InOutDisplay::InsideRange:
                {
                    const int x0 = timeToPos(_p->inOutRange.start_time());
                    const int x1 =
                        timeToPos(_p->inOutRange.end_time_exclusive());
                    const math::Box2i box(
                        x0, p.size.scrollPos.y + g.min.y, x1 - x0 + 1,
                        p.size.margin + p.size.fontMetrics.lineHeight +
                            p.size.margin);
                    event.render->drawRect(
                        box, event.style->getColorRole(ui::ColorRole::InOut));
                    break;
                }
                case InOutDisplay::OutsideRange:
                {
                    int x0 = timeToPos(_timeRange.start_time());
                    int x1 = timeToPos(_p->inOutRange.start_time());
                    math::Box2i box(
                        x0, p.size.scrollPos.y + g.min.y, x1 - x0 + 1,
                        p.size.margin + p.size.fontMetrics.lineHeight +
                            p.size.margin);
                    event.render->drawRect(
                        box, event.style->getColorRole(ui::ColorRole::InOut));
                    x0 = timeToPos(_p->inOutRange.end_time_exclusive());
                    x1 = timeToPos(_timeRange.end_time_exclusive());
                    box = math::Box2i(
                        x0, p.size.scrollPos.y + g.min.y, x1 - x0 + 1,
                        p.size.margin + p.size.fontMetrics.lineHeight +
                            p.size.margin);
                    event.render->drawRect(
                        box, event.style->getColorRole(ui::ColorRole::InOut));
                    break;
                }
                default:
                    break;
                }
            }
        }

        math::Size2i TimelineItem::_getLabelMaxSize(
            const std::shared_ptr<image::FontSystem>& fontSystem) const
        {
            TLRENDER_P();
            const std::string labelMax =
                _data->timeUnitsModel->getLabel(_timeRange.duration());
            const math::Size2i labelMaxSize =
                fontSystem->getSize(labelMax, p.size.fontInfo);
            return labelMaxSize;
        }

        void TimelineItem::_getTimeTicks(
            const std::shared_ptr<image::FontSystem>& fontSystem,
            double& seconds, int& tick)
        {
            TLRENDER_P();
            const int w = _sizeHint.w;
            const float duration =
                _timeRange.duration().rescaled_to(1.0).value();
            const int secondsTick = 1.0 / duration * w;
            const int minutesTick = 60.0 / duration * w;
            const int hoursTick = 3600.0 / duration * w;
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

        void TimelineItem::_drawTimeTicks(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();
            if (_timeRange != time::invalidTimeRange)
            {
                const math::Box2i& g = _geometry;
                const int w = _sizeHint.w;
                const float duration =
                    _timeRange.duration().rescaled_to(1.0).value();
                const int frameTick = 1.0 / _timeRange.duration().value() * w;
                if (frameTick >= p.size.handle)
                {
                    geom::TriangleMesh2 mesh;
                    size_t i = 1;
                    for (double t = 0.0; t < duration;
                         t += 1.0 / _timeRange.duration().rate())
                    {
                        const math::Box2i box(
                            g.min.x + t / duration * w,
                            p.size.scrollPos.y + g.min.y + p.size.margin +
                                p.size.fontMetrics.lineHeight,
                            p.size.border, p.size.margin + p.size.border * 4);
                        if (box.intersects(drawRect))
                        {
                            mesh.v.push_back(
                                math::Vector2f(box.min.x, box.min.y));
                            mesh.v.push_back(
                                math::Vector2f(box.max.x + 1, box.min.y));
                            mesh.v.push_back(
                                math::Vector2f(box.max.x + 1, box.max.y + 1));
                            mesh.v.push_back(
                                math::Vector2f(box.min.x, box.max.y + 1));
                            mesh.triangles.push_back({i + 0, i + 1, i + 2});
                            mesh.triangles.push_back({i + 2, i + 3, i + 0});
                            i += 4;
                        }
                    }
                    if (!mesh.v.empty())
                    {
                        event.render->drawMesh(
                            mesh, math::Vector2i(),
                            event.style->getColorRole(ui::ColorRole::Button));
                    }
                }

                double seconds = 0;
                int tick = 0;
                _getTimeTicks(event.fontSystem, seconds, tick);
                if (seconds > 0.0 && tick > 0)
                {
                    geom::TriangleMesh2 mesh;
                    size_t i = 1;
                    for (double t = 0.0; t < duration; t += seconds)
                    {
                        const math::Box2i box(
                            g.min.x + t / duration * w,
                            p.size.scrollPos.y + g.min.y, p.size.border,
                            p.size.margin + p.size.fontMetrics.lineHeight +
                                p.size.margin + p.size.border * 4);
                        if (box.intersects(drawRect))
                        {
                            mesh.v.push_back(
                                math::Vector2f(box.min.x, box.min.y));
                            mesh.v.push_back(
                                math::Vector2f(box.max.x + 1, box.min.y));
                            mesh.v.push_back(
                                math::Vector2f(box.max.x + 1, box.max.y + 1));
                            mesh.v.push_back(
                                math::Vector2f(box.min.x, box.max.y + 1));
                            mesh.triangles.push_back({i + 0, i + 1, i + 2});
                            mesh.triangles.push_back({i + 2, i + 3, i + 0});
                            i += 4;
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
        }

        void TimelineItem::_drawFrameMarkers(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();
            const math::Box2i& g = _geometry;
            const double rate = _timeRange.duration().rate();
            for (const auto& frameMarker : p.frameMarkers)
            {
                const math::Box2i g2(
                    timeToPos(otime::RationalTime(frameMarker, rate)),
                    p.size.scrollPos.y + g.min.y, p.size.border * 2,
                    p.size.margin + p.size.fontMetrics.lineHeight +
                        p.size.margin + p.size.border * 4);
                if (g2.intersects(drawRect))
                {
                    event.render->drawRect(
                        g2,
                        event.style->getColorRole(ui::ColorRole::FrameMarker));
                }
            }
        }

        void TimelineItem::_drawTimeLabels(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();
            if (_timeRange != time::invalidTimeRange)
            {
                const math::Box2i& g = _geometry;
                const int w = _sizeHint.w;
                const float duration =
                    _timeRange.duration().rescaled_to(1.0).value();
                double seconds = 0;
                int tick = 0;
                _getTimeTicks(event.fontSystem, seconds, tick);
                if (seconds > 0.0 && tick > 0)
                {
                    if (p.size.textInfos.empty())
                    {
                        const math::Size2i labelMaxSize =
                            _getLabelMaxSize(event.fontSystem);
                        for (double t = 0.0; t < duration; t += seconds)
                        {
                            const otime::RationalTime time =
                                _timeRange.start_time() +
                                otime::RationalTime(t, 1.0).rescaled_to(
                                    _timeRange.duration().rate());
                            const math::Box2i box(
                                g.min.x + t / duration * w + p.size.border +
                                p.size.margin,
                                p.size.scrollPos.y + g.min.y + p.size.margin,
                                labelMaxSize.w, p.size.fontMetrics.lineHeight);
                            if (time != p.currentTime && box.intersects(drawRect))
                            {
                                const std::string label =
                                    _data->timeUnitsModel->getLabel(time);
                                event.render->appendText(p.size.textInfos,
                                                         event.fontSystem->getGlyphs(
                                                             label, p.size.fontInfo),
                                                         math::Vector2i(
                                                             box.min.x,
                                                             box.min.y + p.size.fontMetrics.ascender));
                            }
                        }
                    }
                    for (const auto& textInfo : p.size.textInfos)
                    {
                        event.render->drawText(textInfo, math::Vector2i(),
                                               event.style->getColorRole(
                                                   ui::ColorRole::TextDisabled));
                    }
                }
            }
        }

        void TimelineItem::_drawCacheInfo(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::Box2i& g = _geometry;

            if (CacheDisplay::VideoAndAudio == _displayOptions.cacheDisplay ||
                CacheDisplay::VideoOnly == _displayOptions.cacheDisplay)
            {
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.videoFrames)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const int h = CacheDisplay::VideoAndAudio ==
                                          _displayOptions.cacheDisplay
                                      ? p.size.border * 2
                                      : p.size.border * 4;
                    const math::Box2i box(
                        x0,
                        p.size.scrollPos.y + g.min.y + p.size.margin +
                            p.size.fontMetrics.lineHeight + p.size.margin,
                        x1 - x0 + 1, h);
                    if (box.intersects(drawRect))
                    {
                        mesh.v.push_back(math::Vector2f(box.min.x, box.min.y));
                        mesh.v.push_back(
                            math::Vector2f(box.max.x + 1, box.min.y));
                        mesh.v.push_back(
                            math::Vector2f(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(
                            math::Vector2f(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({i + 0, i + 1, i + 2});
                        mesh.triangles.push_back({i + 2, i + 3, i + 0});
                        i += 4;
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

            if (CacheDisplay::VideoAndAudio == _displayOptions.cacheDisplay)
            {
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.audioFrames)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const math::Box2i box(
                        x0,
                        p.size.scrollPos.y + g.min.y + p.size.margin +
                            p.size.fontMetrics.lineHeight + p.size.margin +
                            p.size.border * 2,
                        x1 - x0 + 1, p.size.border * 2);
                    if (box.intersects(drawRect))
                    {
                        mesh.v.push_back(math::Vector2f(box.min.x, box.min.y));
                        mesh.v.push_back(
                            math::Vector2f(box.max.x + 1, box.min.y));
                        mesh.v.push_back(
                            math::Vector2f(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(
                            math::Vector2f(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({i + 0, i + 1, i + 2});
                        mesh.triangles.push_back({i + 2, i + 3, i + 0});
                        i += 4;
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

        void TimelineItem::_drawCurrentTime(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::Box2i& g = _geometry;

            if (!p.currentTime.is_invalid_time())
            {
                const math::Vector2i pos(
                    timeToPos(p.currentTime), p.size.scrollPos.y + g.min.y);

                event.render->drawRect(
                    math::Box2i(pos.x, pos.y, p.size.border * 2, g.h()),
                    event.style->getColorRole(ui::ColorRole::Red));

                const std::string label =
                    _data->timeUnitsModel->getLabel(p.currentTime);
                std::vector<timeline::TextInfo> textInfos;
                event.render->appendText(textInfos,
                                         event.fontSystem->getGlyphs(
                                             label, p.size.fontInfo),
                                         math::Vector2i(
                                             pos.x + p.size.border * 2 + p.size.margin,
                                             pos.y + p.size.margin + p.size.fontMetrics.ascender));
                            
                for (const auto& textInfo : textInfos)
                {
                    event.render->drawText(textInfo, math::Vector2i(),
                                           event.style->getColorRole(
                                               ui::ColorRole::Text));
                }
            }
        }

        void TimelineItem::_tracksUpdate()
        {
            TLRENDER_P();
            for (const auto& track : p.tracks)
            {
                const bool visible = _isTrackVisible(track.index);
                track.label->setVisible(_displayOptions.trackInfo && visible);
                track.durationLabel->setVisible(
                    _displayOptions.trackInfo && visible);
                for (const auto& item : track.items)
                {
                    item->setVisible(visible);
                }
            }
        }

        void TimelineItem::_textUpdate()
        {
            TLRENDER_P();
            
            p.size.textInfos.clear();
            for (const auto& track : p.tracks)
            {
                const otime::RationalTime duration = track.timeRange.duration();
                const bool khz = TrackType::Audio == track.type
                                     ? (duration.rate() >= 1000.0)
                                     : false;
                const otime::RationalTime rescaled =
                    duration.rescaled_to(_data->speed);
                const std::string label =
                    string::Format("{0}, {1}{2}")
                        .arg(_data->timeUnitsModel->getLabel(rescaled))
                        .arg(khz ? (duration.rate() / 1000.0) : duration.rate())
                        .arg(khz ? "kHz" : "FPS");
                track.durationLabel->setText(label);
            }
        }

        bool TimelineItem::_transitionIntersects(const std::shared_ptr<IItem> item,
                                                 const int transitionTrack,
                                                 const otime::TimeRange& timeRange)
        {
            TLRENDER_P();
            
            bool out = false;
            for (const auto& transition : p.tracks[transitionTrack].transitions)
            {
                if (item == transition)
                    continue;
                otime::TimeRange transitionRange = transition->getTimeRange();
                if (transitionRange.intersects(timeRange))
                {
                    out = true;
                    break;
                }
            }
            return out;
        }

        std::vector<const otio::Item*> TimelineItem::getSelectedItems() const
        {
            TLRENDER_P();
            
            std::vector<const otio::Item*> out;
            for (const auto& item : p.mouse.items)
            {
                if (auto clip = dynamic_cast<const IBasicItem*>(item->p.get()))
                    out.push_back(clip->getOtioItem());
            }
            return out;
        }
        
    
        TimelineItem::Private::MouseItemData::MouseItemData() {}

        TimelineItem::Private::MouseItemData::MouseItemData(
            const std::shared_ptr<IItem>& item, int index, int track) :
            p(item),
            index(index),
            track(track)
        {
            if (p)
            {
                p->setSelectRole(ui::ColorRole::Checked);
                geometry = p->getGeometry();
            }
        }

        TimelineItem::Private::MouseItemData::~MouseItemData()
        {
            if (p)
            {
                p->setSelectRole(ui::ColorRole::kNone);
                p->setGeometry(geometry);
            }
        }

        std::shared_ptr<IItem> TimelineItem::Private::getAssociated(
            const std::shared_ptr<IItem>& item, int& index,
            int& trackIndex) const
        {
            std::shared_ptr<IItem> out;
            if (trackIndex >= 0 && trackIndex < tracks.size() &&
                tracks.size() > 1)
            {
                const otime::TimeRange& timeRange = item->getTimeRange();
                if (TrackType::Video == tracks[trackIndex].type &&
                    trackIndex < tracks.size() - 1 &&
                    TrackType::Audio == tracks[trackIndex + 1].type)
                {
                    for (size_t i = 0; i < tracks[trackIndex + 1].items.size();
                         ++i)
                    {
                        const otime::TimeRange& audioTimeRange =
                            tracks[trackIndex + 1].items[i]->getTimeRange();
                        const otime::RationalTime audioStartTime =
                            audioTimeRange.start_time().rescaled_to(
                                timeRange.start_time().rate());
                        const otime::RationalTime audioDuration =
                            audioTimeRange.duration().rescaled_to(
                                timeRange.duration().rate());
                        if (math::fuzzyCompare(
                                audioStartTime.value(),
                                timeRange.start_time().value()) &&
                            math::fuzzyCompare(
                                audioDuration.value(),
                                timeRange.duration().value()))
                        {
                            out = tracks[trackIndex + 1].items[i];
                            index = i;
                            trackIndex = trackIndex + 1;
                            break;
                        }
                    }
                }
                else if (
                    TrackType::Audio == tracks[trackIndex].type &&
                    trackIndex > 0 &&
                    TrackType::Video == tracks[trackIndex - 1].type)
                {
                    for (size_t i = 0; i < tracks[trackIndex - 1].items.size();
                         ++i)
                    {
                        const otime::TimeRange& videoTimeRange =
                            tracks[trackIndex - 1].items[i]->getTimeRange();
                        const otime::RationalTime videoStartTime =
                            videoTimeRange.start_time().rescaled_to(
                                timeRange.start_time().rate());
                        const otime::RationalTime videoDuration =
                            videoTimeRange.duration().rescaled_to(
                                timeRange.duration().rate());
                        if (math::fuzzyCompare(
                                videoStartTime.value(),
                                timeRange.start_time().value()) &&
                            math::fuzzyCompare(
                                videoDuration.value(),
                                timeRange.duration().value()))
                        {
                            out = tracks[trackIndex - 1].items[i];
                            index = i;
                            trackIndex = trackIndex - 1;
                            break;
                        }
                    }
                }
            }
            return out;
        }

        std::vector<TimelineItem::Private::MouseItemDropTarget>
        TimelineItem::Private::getDropTargets(
            const math::Box2i& geometry, int index, int trackIndex)
        {
            std::vector<MouseItemDropTarget> out;
            if (trackIndex >= 0 && trackIndex < tracks.size())
            {
                const auto& track = tracks[trackIndex];
                if (track.type == tracks[trackIndex].type)
                {
                    size_t i = 0;
                    math::Box2i g;
                    for (; i < track.items.size(); ++i)
                    {
                        const auto& item = track.items[i];
                        g = item->getGeometry();
                        if (i == index || i == (index + 1))
                        {
                            continue;
                        }
                        MouseItemDropTarget dt;
                        dt.index = i;
                        dt.track = trackIndex;
                        dt.mouse = math::Box2i(
                            g.min.x - size.handle, g.min.y, size.handle * 2,
                            g.h());
                        dt.draw = math::Box2i(
                            g.min.x - size.border * 2,
                            size.scrollPos.y + geometry.min.y, size.border * 4,
                            geometry.h());
                        out.push_back(dt);
                    }
                    if (!track.items.empty() &&
                        index < (track.items.size() - 1))
                    {
                        MouseItemDropTarget dt;
                        dt.index = i;
                        dt.track = trackIndex;
                        dt.mouse = math::Box2i(
                            g.max.x - size.handle, g.min.y, size.handle * 2,
                            g.h());
                        dt.draw = math::Box2i(
                            g.max.x - size.border * 2,
                            size.scrollPos.y + geometry.min.y, size.border * 4,
                            geometry.h());
                        out.push_back(dt);
                    }
                }
            }
            return out;
        }

        void TimelineItem::_storeUndo()
        {
            TLRENDER_P();
            
            std::vector<timeline::MoveData> moveData;
            moveData.push_back(
                {
                    timeline::MoveType::UndoOnly
                });
            if (p.moveCallback)
                p.moveCallback(moveData);
        }
        
        bool TimelineItem::_clampRangeToNeighborTransitions(const otio::Item* item,
                                                            const otime::TimeRange& proposedRange,
                                                            otime::TimeRange& clampedRange)
        {
            if (!item) {
                return false;
            }

            const auto* track = dynamic_cast<const otio::Track*>(item->parent());
            if (!track) {
                return false;
            }

            int index = track->index_of_child(item);
            if (index < 0) {
                return false;
            }

            otime::RationalTime proposed_start = proposedRange.start_time();
            otime::RationalTime proposed_duration = proposedRange.duration();
            otime::RationalTime proposed_end_excl = proposed_start + proposed_duration;

            // We need the CURRENT range to understand where the transition boundaries are
            // in the parent's coordinate space.
            const otime::TimeRange currentRange = track->range_of_child(item);
            otime::RationalTime current_duration = currentRange.duration();

            otime::RationalTime clamped_start = proposed_start;
            otime::RationalTime clamped_end_excl = proposed_end_excl;

            // ---------------------------------------------------
            // Incoming transition (at the start of the item)
            // ---------------------------------------------------
            otime::RationalTime transition_end = proposed_end_excl;
            otime::RationalTime transition_out_offset(0, proposed_start.rate());
            otime::RationalTime transition_start = proposed_start;
            otime::RationalTime transition_in_offset(0, proposed_start.rate());
            
            if (index > 0) {
                if (auto prevTransition = otio::dynamic_retainer_cast<otio::Transition>(track->children()[index - 1])) {
                    transition_out_offset = prevTransition->out_offset();
                    transition_end = currentRange.start_time() + transition_out_offset;
                }
            }

            // ---------------------------------------------------
            // Outgoing transition (at the end of the item)
            // ---------------------------------------------------
            if (index + 1 < static_cast<int>(track->children().size())) {
                if (auto nextTransition = otio::dynamic_retainer_cast<otio::Transition>(track->children()[index + 1])) {
                    transition_in_offset = nextTransition->in_offset();
                    transition_start = currentRange.end_time_exclusive() - transition_in_offset;
                }
            }

            // Check start and ending transitions
            if (proposed_end_excl < transition_start) {
                clamped_end_excl = proposed_end_excl = transition_start;
            }
            if (proposed_start > transition_start) {
                clamped_start = proposed_start = transition_start;
            }
    
            if (proposed_start > transition_end) {
                clamped_start = proposed_start = transition_end;
            }
            if (proposed_end_excl < transition_end) {
                clamped_end_excl = proposed_end_excl = transition_end;
            }

            // Check overlapping transitions
            if (clamped_start + transition_out_offset > clamped_end_excl - transition_in_offset)
            {
                clamped_end_excl = clamped_start + transition_in_offset + transition_out_offset;
            }

            otime::RationalTime clamped_duration = clamped_end_excl - clamped_start;
            clampedRange = otime::TimeRange(clamped_start, clamped_duration);

            return !(clampedRange == proposedRange);
        }
        
    } // namespace TIMELINEUI
} // namespace tl

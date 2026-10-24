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

#include <algorithm>
#include <cmath>

namespace tl
{
    namespace TIMELINEUI
    {
#ifdef OPENGL_BACKEND
        void TimelineItem::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<timeline::Timeline>& timeline,
            const std::shared_ptr<timeline::Player>& player,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<gl::GLFWWindow>& window,
            const std::shared_ptr<IWidget>& parent)
#endif

#ifdef VULKAN_BACKEND
        void TimelineItem::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<timeline::Timeline>& timeline,
            const std::shared_ptr<timeline::Player>& player,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<IWidget>& parent)
#endif
        {
            const OTIO_NS::TimeRange timeRange = player ?
                player->getTimeRange() :
                timeline->getTimeRange();
            const OTIO_NS::TimeRange trimmedRange(
                OTIO_NS::RationalTime(0.0, timeRange.duration().rate()),
                timeRange.duration());
            IItem::_init(
                "tl::TIMELINEUI::TimelineItem", timeRange, trimmedRange,
                trimmedRange, scale,
                options, displayOptions, itemData, context, parent);
            TLRENDER_P();

            _setMouseHover(true);
            _setMousePress(true, 0, 0);

            p.player = player;

            p.scrub = observer::Value<bool>::create(false);
            p.timeScrub =
                observer::Value<otime::RationalTime>::create(time::invalidTime);

            p.thumbnailSystem = context->getSystem<TIMELINEUI::ThumbnailSystem>();

            const auto otioTimeline = timeline->getTimeline();
            for (const auto& child : otioTimeline->tracks()->children())
            {
                if (auto otioTrack =
                        OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(child))
                {
                    if (!otioTrack->enabled())
                        continue;
                    Private::Track track;
                    int otioIndex = 0;
                    track.index = p.tracks.size();
                    std::string trackLabel = otioTrack->name();
                    if (OTIO_NS::Track::Kind::video == otioTrack->kind())
                    {
                        track.type = TrackType::Video;
                        if (trackLabel.empty())
                        {
                            trackLabel = "Video Track";
                        }
                    }
                    else if (OTIO_NS::Track::Kind::audio == otioTrack->kind())
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
                                OTIO_NS::dynamic_retainer_cast<OTIO_NS::Clip>(child))
                        {
                            switch (track.type)
                            {
                            case TrackType::Video:
                                track.items.push_back(VideoClipItem::create(
                                    context, timeline, clip, scale, options,
                                    displayOptions,
                                    itemData, p.thumbnailSystem,
                                    shared_from_this()));
                                break;
                            case TrackType::Audio:
                                track.items.push_back(AudioClipItem::create(
                                    context, timeline, clip, scale, options,
                                    displayOptions,
                                    itemData, p.thumbnailSystem,
                                    shared_from_this()));
                                break;
                            default:
                                break;
                            }
                            track.otioIndexes.push_back(otioIndex);

                            // Note: effects are created and laid out by the
                            // VideoClipItem itself (they are its children).
                        }
                        else if (
                            auto gap =
                                OTIO_NS::dynamic_retainer_cast<OTIO_NS::Gap>(child))
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
                                OTIO_NS::dynamic_retainer_cast<OTIO_NS::Transition>(
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
        }

        TimelineItem::~TimelineItem()
        {
        }

#ifdef OPENGL_BACKEND
        TimelineItem::TimelineItem() :
            _p(new Private)
        {
        }

        std::shared_ptr<TimelineItem> TimelineItem::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<timeline::Player>& player,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<gl::GLFWWindow>& window,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineItem>(new TimelineItem);
            out->_init(
                context,
                player->getTimeline(),
                player, stack,
                scale, options,
                displayOptions,
                itemData, window, parent);
            return out;
        }

        std::shared_ptr<TimelineItem> TimelineItem::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<timeline::Timeline>& timeline,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<gl::GLFWWindow>& window,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineItem>(new TimelineItem);
            out->_init(
                context,
                timeline,
                nullptr,
                stack,
                scale,
                options,
                displayOptions,
                itemData,
                window,
                parent);
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
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<timeline::Player>& player,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            Fl_Vk_Context& ctx,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineItem>(new TimelineItem(ctx));
            out->_init(
                context, player->getTimeline(), player, stack, scale, options, displayOptions,
                itemData, parent);
            return out;
        }

        std::shared_ptr<TimelineItem> TimelineItem::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<timeline::Timeline>& timeline,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            Fl_Vk_Context& ctx,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineItem>(new TimelineItem(ctx));
            out->_init(
                context, timeline, nullptr, stack, scale, options, displayOptions,
                itemData, parent);
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

        std::shared_ptr<observer::IValue<OTIO_NS::RationalTime> >
        TimelineItem::observeTimeScrub() const
        {
            return _p->timeScrub;
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
                    const OTIO_NS::TimeRange& timeRange = item->getTimeRange();
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
                        const OTIO_NS::TimeRange& timeRange =
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

            // The ruler strip (in/out points, ticks, markers, labels, cache
            // and current time) is drawn by TimelineRuler. The space for it
            // is still reserved above the tracks in sizeHintEvent() and
            // setGeometry().

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
            bool selectionMode = false;
            if (p.editMode == timeline::EditMode::Select)
            {
                doSelection = true;
                selectionMode = true;
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
                                if (math::contains(item->getGeometry(),
                                                   event.pos))
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
                                        // Note: j and i are passed by value
                                        // so the enclosing loops are not
                                        // disturbed.
                                        for (const auto& associated :
                                             p.getAssociated(item, i))
                                        {
                                            p.mouse.items.push_back(
                                                std::make_shared<
                                                    Private::MouseItemData>(
                                                    associated.item,
                                                    associated.index,
                                                    associated.track));
                                            moveToFront(associated.item);
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        if (!selectionMode && !p.mouse.items.empty())
                        {
                            break;
                        }
                    }
                }

                if (selectionMode || p.mouse.items.empty())
                {
                    for (int i = 0; i < p.tracks.size(); ++i)
                    {
                        if (_isTrackVisible(i))
                        {
                            const auto& transitions = p.tracks[i].transitions;
                            for (int j = 0; j < transitions.size(); ++j)
                            {
                                const auto& item = transitions[j];
                                if (math::contains(item->getGeometry(),
                                                   event.pos))
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
                        if (!selectionMode && !p.mouse.items.empty())
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
                    const OTIO_NS::RationalTime time = posToTime(event.pos.x);
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

        void TimelineItem::_addOneFrameGap(const OTIO_NS::RationalTime& videoTime,
                                           OTIO_NS::TimeRange& timeRange)
        {
            OTIO_NS::RationalTime one_frame(1.0, videoTime.rate());
            const OTIO_NS::RationalTime& startTime =
                timeRange.start_time() + one_frame;
            const OTIO_NS::RationalTime& duration =
                timeRange.duration() - one_frame;
            timeRange = OTIO_NS::TimeRange(startTime, duration);
        }

        void TimelineItem::_getTransitionItems(std::vector<IBasicItem*>& items,
                                               const int trackNumber,
                                               const OTIO_NS::TimeRange& transitionRange)
        {
            TLRENDER_P();

            for (const auto& item : p.tracks[trackNumber].items)
            {
                IBasicItem* clip = static_cast<IBasicItem*>(item.get());
                OTIO_NS::TimeRange itemRange = clip->getTimeRange();
                if (itemRange.intersects(transitionRange))
                {
                    items.push_back(clip);
                }
            }
        }

        void TimelineItem::_getTransitionTimeRanges(std::vector<OTIO_NS::TimeRange>& timeRanges,
                                                    const int trackNumber,
                                                    const OTIO_NS::TimeRange& transitionRange)
        {
            TLRENDER_P();

            std::vector<IBasicItem*> items;
            _getTransitionItems(items, trackNumber, transitionRange);

            for (const auto& item : items)
            {
                OTIO_NS::TimeRange itemRange = item->getTimeRange();
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
                const OTIO_NS::RationalTime duration = track.timeRange.duration();
                const bool khz = TrackType::Audio == track.type
                                     ? (duration.rate() >= 1000.0)
                                     : false;
                const OTIO_NS::RationalTime rescaled =
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
                                                 const OTIO_NS::TimeRange& timeRange)
        {
            TLRENDER_P();

            bool out = false;
            for (const auto& transition : p.tracks[transitionTrack].transitions)
            {
                if (item == transition)
                    continue;
                OTIO_NS::TimeRange transitionRange = transition->getTimeRange();
                if (transitionRange.intersects(timeRange))
                {
                    out = true;
                    break;
                }
            }
            return out;
        }

        std::vector<timeline::MoveData>
        TimelineItem::getSelectedItems() const
        {
            TLRENDER_P();

            std::vector<timeline::MoveData> out;
            for (const auto& item : p.mouse.items)
            {
                if (!dynamic_cast<VideoClipItem*>(item->p.get()) &&
                    !dynamic_cast<AudioClipItem*>(item->p.get()))
                    continue;
                timeline::MoveData move;
                move.fromTrack = move.toTrack = item->track;
                move.fromIndex = move.toIndex = item->index;
                out.push_back(move);
            }
            return out;
        }

        std::vector<timeline::MoveData>
        TimelineItem::getSelectedTransitions() const
        {
            TLRENDER_P();

            std::vector<timeline::MoveData> out;
            for (const auto& item : p.mouse.items)
            {
                if (!dynamic_cast<TransitionItem*>(item->p.get()))
                    continue;
                timeline::MoveData move;
                move.fromTrack = move.toTrack = item->track;
                move.fromIndex = move.toIndex = item->index;
                out.push_back(move);
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

        std::vector<TimelineItem::Private::Associated>
        TimelineItem::Private::getAssociated(
            const std::shared_ptr<IItem>& item, int trackIndex) const
        {
            std::vector<Associated> out;
            const int tracksSize = static_cast<int>(tracks.size());
            if (!item || trackIndex < 0 || trackIndex >= tracksSize)
                return out;

            // Look for items on tracks of the opposite type.
            const TrackType otherType =
                TrackType::Video == tracks[trackIndex].type ?
                    TrackType::Audio :
                    TrackType::Video;

            const OTIO_NS::TimeRange& timeRange = item->getTimeRange();
            const double start = timeRange.start_time().to_seconds();
            const double end = timeRange.end_time_exclusive().to_seconds();
            const double rate = timeRange.start_time().rate();

            // Scored candidates (lower is better).
            struct Candidate
            {
                Associated associated;
                double score = 0.0;
            };
            std::vector<Candidate> candidates;

            for (int t = 0; t < tracksSize; ++t)
            {
                if (t == trackIndex || tracks[t].type != otherType)
                    continue;

                const auto& items = tracks[t].items;
                for (int i = 0; i < static_cast<int>(items.size()); ++i)
                {
                    const auto& other = items[i];
                    if (!other || std::dynamic_pointer_cast<GapItem>(other))
                        continue;

                    const OTIO_NS::TimeRange& otherRange =
                        other->getTimeRange();

                    // Tolerance is half a frame at the slowest of the two
                    // rates (so a 48kHz audio range does not make it
                    // vanishingly small).
                    double minRate = rate;
                    const double otherRate = otherRange.start_time().rate();
                    if (otherRate > 0.0 && (minRate <= 0.0 || otherRate < minRate))
                        minRate = otherRate;
                    const double tolerance =
                        0.5 / (minRate > 0.0 ? minRate : 24.0);

                    const double ds = std::abs(
                        otherRange.start_time().to_seconds() - start);
                    const double de = std::abs(
                        otherRange.end_time_exclusive().to_seconds() - end);
                    if (ds <= tolerance && de <= tolerance)
                    {
                        Candidate c;
                        c.associated = {other, i, t};
                        // Closest match first; ties go to the nearest track.
                        c.score = ds + de + 1e-9 * std::abs(t - trackIndex);
                        candidates.push_back(c);
                    }
                }
            }

            std::stable_sort(
                candidates.begin(), candidates.end(),
                [](const Candidate& a, const Candidate& b)
                { return a.score < b.score; });

            out.reserve(candidates.size());
            for (const auto& c : candidates)
                out.push_back(c.associated);
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

        bool TimelineItem::_clampRangeToNeighborTransitions(const OTIO_NS::Item* item,
                                                            const OTIO_NS::TimeRange& proposedRange,
                                                            OTIO_NS::TimeRange& clampedRange)
        {
            if (!item) {
                return false;
            }

            const auto* track = dynamic_cast<const OTIO_NS::Track*>(item->parent());
            if (!track) {
                return false;
            }

            int index = track->index_of_child(item);
            if (index < 0) {
                return false;
            }

            OTIO_NS::RationalTime proposed_start = proposedRange.start_time();
            OTIO_NS::RationalTime proposed_duration = proposedRange.duration();
            OTIO_NS::RationalTime proposed_end_excl = proposed_start + proposed_duration;

            // We need the CURRENT range to understand where the transition boundaries are
            // in the parent's coordinate space.
            const OTIO_NS::TimeRange currentRange = track->range_of_child(item);
            OTIO_NS::RationalTime current_duration = currentRange.duration();

            OTIO_NS::RationalTime clamped_start = proposed_start;
            OTIO_NS::RationalTime clamped_end_excl = proposed_end_excl;

            // ---------------------------------------------------
            // Incoming transition (at the start of the item)
            // ---------------------------------------------------
            OTIO_NS::RationalTime transition_end = proposed_end_excl;
            OTIO_NS::RationalTime transition_out_offset(0, proposed_start.rate());
            OTIO_NS::RationalTime transition_start = proposed_start;
            OTIO_NS::RationalTime transition_in_offset(0, proposed_start.rate());

            if (index > 0) {
                if (auto prevTransition = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Transition>(track->children()[index - 1])) {
                    transition_out_offset = prevTransition->out_offset();
                    transition_end = currentRange.start_time() + transition_out_offset;
                }
            }

            // ---------------------------------------------------
            // Outgoing transition (at the end of the item)
            // ---------------------------------------------------
            if (index + 1 < static_cast<int>(track->children().size())) {
                if (auto nextTransition = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Transition>(track->children()[index + 1])) {
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

            OTIO_NS::RationalTime clamped_duration = clamped_end_excl - clamped_start;
            clampedRange = OTIO_NS::TimeRange(clamped_start, clamped_duration);

            return !(clampedRange == proposedRange);
        }

    } // namespace TIMELINEUI
} // namespace tl

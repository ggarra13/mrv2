// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "TimelineItem.h"
#include "TimelineItemPrivate.h"

namespace tl
{
    namespace TIMELINEUI
    {

        void TimelineItem::_mouseMoveEventTrim(ui::MouseMoveEvent& event)
        {
            TLRENDER_P();
            if (p.mouse.items.empty())
                return;
            switch (p.mouse.mode)
            {
            case Private::MouseMode::kNone:
                break;
            case Private::MouseMode::Transition:
            {
                _mouseMoveEventRoll(event);
                break;
            }
            case Private::MouseMode::Item:
            {
                _mouse.pos.y = _mouse.pressPos.y;
                const int offset = _mouse.pos.x - _mouse.pressPos.x;
                math::Box2i move;

                for (const auto& item : p.mouse.items)
                {
                    const math::Box2i& g = item->geometry;
                    auto clip = static_cast<IBasicItem*>(item->p.get());
                    OTIO_NS::TimeRange timeRange = clip->getTimeRange();
                    const OTIO_NS::Item* otioItem = clip->getOtioItem();

                    if (p.mouse.side == Private::MouseClick::Left)
                    {
                        math::Size2i size = g.getSize();
                        size.w -= offset;

                        move = math::Box2i(
                            g.min + _mouse.pos - _mouse.pressPos,
                            size );
                    }
                    else
                    {
                        math::Size2i size = g.getSize();
                        size.w += offset;
                        move = math::Box2i(g.min, size);
                    }


                    OTIO_NS::RationalTime startTime = posToTime(move.x());
                    OTIO_NS::RationalTime duration  = posToTime(move.x() + move.w()) - startTime;
                    startTime -= _timeRange.start_time();

                    // Clamp on clips.
                    OTIO_NS::ErrorStatus status;
                    const auto& availableRange = otioItem->available_range(&status);
                    if (!OTIO_NS::is_error(status))
                    {
                        if (startTime < availableRange.start_time())
                            continue;
                        if (duration > availableRange.duration())
                            continue;
                    }
                    else
                    {
                        if (startTime < OTIO_NS::RationalTime(0, startTime.rate()))
                            continue;
                        if (duration.value() <= 1.F)
                        continue;
                    }

                    item->p->setGeometry(move);
                }
                break;
            }
            default:
                break;
            }
        }

        void TimelineItem::_mouseReleaseEventTrim(ui::MouseClickEvent& event)
        {
            TLRENDER_P();
            if (p.mouse.items.empty())
                return;
            switch (p.mouse.mode)
            {
            case Private::MouseMode::CurrentTime:
            {
                break;
            }
            case Private::MouseMode::Item:
            {
                _mouse.pos.y = _mouse.pressPos.y;
                const int offset = _mouse.pos.x - _mouse.pressPos.x;
                math::Box2i move;

                //
                // Store an undo in callback
                //
                _storeUndo();

                const auto otioTimeline = p.player->getTimeline()->getTimeline();
                for (const auto& item : p.mouse.items)
                {
                    const math::Box2i& g = item->geometry;
                    OTIO_NS::TimeRange timeRange = item->p->getTimeRange();
                    OTIO_NS::TimeRange origRange = timeRange;

                    math::Size2i size = g.getSize();
                    if (p.mouse.side == Private::MouseClick::Left)
                    {
                        size.w -= offset;
                        move = math::Box2i(
                            g.min + _mouse.pos - _mouse.pressPos,
                            size);
                    }
                    else if (p.mouse.side == Private::MouseClick::Right)
                    {
                        size.w += offset;
                        move = math::Box2i(g.min, size);
                    }

                    OTIO_NS::RationalTime startTime = posToTime(move.x());
                    OTIO_NS::RationalTime duration  = posToTime(move.x() + move.w()) -
                                                    startTime;

                    startTime -= _timeRange.start_time();
                    timeRange = OTIO_NS::TimeRange(startTime, duration);

                    auto startOffset = timeRange.start_time() - origRange.start_time();
                    auto durationOffset = timeRange.duration() - origRange.duration();

                    const int trackIndex = item->track;
                    const int itemIndex = item->index;
                    const int otioItemIndex = p.tracks[trackIndex].otioIndexes[itemIndex];

                    OTIO_NS::ErrorStatus status;
                    const auto& child = otioTimeline->tracks()->children()[trackIndex];
                    if (auto otioTrack = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(child))
                    {
                        const auto& otioChild = otioTrack->children()[otioItemIndex];
                        auto otioItem = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Item>(otioChild);


                        auto parentRange = otioItem->trimmed_range_in_parent().value();
                        auto proposedRange = OTIO_NS::TimeRange(parentRange.start_time() + startOffset,
                                                             parentRange.duration() + durationOffset);
                        OTIO_NS::TimeRange clampedRange;

                        _clampRangeToNeighborTransitions(otioItem, proposedRange, clampedRange);

                        // Calculate the "Correction" applied by the clamp
                        // If the clamp moved the start by +2 frames, we need to subtract that from our offset
                        auto startCorrection = clampedRange.start_time() - proposedRange.start_time();
                        auto durationCorrection = clampedRange.duration() - proposedRange.duration();

                        // Apply the correction to your offsets
                        startOffset += startCorrection;
                        durationOffset += durationCorrection;

                        if (auto otioClip = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Clip>(otioChild))
                        {
                            origRange = otioClip->source_range().value();

                            const auto& availableRange = otioClip->available_range(&status);
                            auto startTime = origRange.start_time() + startOffset;
                            auto duration  = origRange.duration() + durationOffset;
                            timeRange = OTIO_NS::TimeRange(startTime, duration);

                            // Clamp to available range if present
                            if (!OTIO_NS::is_error(status))
                            {
                                timeRange = timeRange.clamped(availableRange);
                            }
                            else
                            {
                                if (startTime.value() < 0.F)
                                    startTime = OTIO_NS::RationalTime(0.F, startTime.rate());
                                if (duration.value() < 1.F)
                                    duration = OTIO_NS::RationalTime(1.F, duration.rate());
                                timeRange = OTIO_NS::TimeRange(startTime, duration);
                            }

                            otioClip->set_source_range(timeRange);
                        }
                        else if (auto otioGap = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Gap>(otioChild))
                        {
                            auto startTime = origRange.start_time() + startOffset;
                            auto duration  = origRange.duration() + durationOffset;

                            if (startTime.value() < 0.F)
                                startTime = OTIO_NS::RationalTime(0.F, startTime.rate());
                            if (duration.value() < 1.F)
                                duration = OTIO_NS::RationalTime(1.F, duration.rate());

                            timeRange = OTIO_NS::TimeRange(startTime, duration);

                            otioGap->set_source_range(timeRange);
                        }
                    }
                }
                p.player->setTimeline(otioTimeline);
                break;
            }
            case Private::MouseMode::Transition:
            {
                _mouseReleaseEventRoll(event);
            }
            default:
                break;
            }
        }

    }
}

// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "TimelineItem.h"
#include "TimelineItemPrivate.h"

#include <opentimelineio/gap.h>

namespace tl
{
    namespace TIMELINEUI
    {
        
        void TimelineItem::_mouseMoveEventRoll(ui::MouseMoveEvent& event)
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
                _mouse.pos.y = _mouse.pressPos.y;
                const int offset = _mouse.pos.x - _mouse.pressPos.x;
                math::Box2i move;
                    
                for (const auto& item : p.mouse.items)
                {
                    const math::Box2i& g = item->geometry;
                    auto transitionItem = static_cast<TransitionItem*>(item->p.get());
                    OTIO_NS::TimeRange timeRange = item->p->getTimeRange();

                    // Get transition items (ie. the clips associated to the
                    // transition) time ranges.
                    const int transitionTrack = item->track;
                    std::vector<OTIO_NS::TimeRange> itemRanges;
                    _getTransitionTimeRanges(itemRanges, transitionTrack,
                                             timeRange);
                        
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

                    const OTIO_NS::RationalTime& startTime = posToTime(move.x());
                    const OTIO_NS::RationalTime& duration  = posToTime(move.x() + move.w()) - startTime;
                        
                    timeRange = OTIO_NS::TimeRange(startTime - _timeRange.start_time(), duration);
                    const OTIO_NS::RationalTime in_offset = itemRanges[1].start_time() -
                                                          timeRange.start_time();
                    const OTIO_NS::RationalTime out_offset = timeRange.end_time_exclusive() -
                                                           itemRanges[1].start_time();

                    if (in_offset.value() <= 1.F ||
                        out_offset.value() <= 1.F)
                        continue;

                    // Clamp on clips.
                    if (duration.value() <= 2.F)
                    {
                        continue;
                    }

                    if (timeRange.start_time() <= itemRanges[0].start_time())
                    {
                        continue;
                    }
                    
                    if (timeRange.start_time() >= itemRanges[0].end_time_exclusive())
                    {
                        continue;
                    }
                    
                    if (timeRange.end_time_exclusive() >= itemRanges[1].end_time_inclusive())
                    {
                        continue;
                    }
                    
                    if (timeRange.end_time_exclusive() <= itemRanges[0].end_time_inclusive())
                    {
                        continue;
                    }
                    
                    // Clamp on other transitions.
                    if (_transitionIntersects(item->p, transitionTrack, timeRange))
                    {
                        continue;
                    }
                    
                    transitionItem->setDurationLabel(std::to_string(int(duration.value())));
                    item->p->setGeometry(move);
                }
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
                    const OTIO_NS::RationalTime& duration  = posToTime(move.x() + move.w()) - startTime;
                        

                    startTime -= _timeRange.start_time();
                    // Clamp on clips.
                    // Create new time range.
                    timeRange = OTIO_NS::TimeRange(startTime, duration);

                    // Clamp on available range if present.
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

        void TimelineItem::_mouseReleaseEventRoll(ui::MouseClickEvent& event)
        {
            TLRENDER_P();
            if (p.mouse.items.empty())
                return;
            switch (p.mouse.mode)
            {
            case Private::MouseMode::kNone:
                break;
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
                // Store an undo in before modifying timeline.
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
                            size );

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

                    const auto startOffset = timeRange.start_time() - origRange.start_time();
                    const auto durationOffset = timeRange.duration() - origRange.duration();
                    
                    const int trackIndex = item->track;
                    const int itemIndex = item->index;
                    const int otioItemIndex = p.tracks[trackIndex].otioIndexes[itemIndex];

                    OTIO_NS::ErrorStatus status;
                    const auto& child = otioTimeline->tracks()->children()[trackIndex];
                    if (auto otioTrack = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(child))
                    {
                        const auto& otioChild = otioTrack->children()[otioItemIndex];
                        if (auto otioItem = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Clip>(otioChild))
                        {
                            origRange = otioItem->source_range().value();

                            const auto& availableRange = otioItem->available_range(&status);            
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
                                timeRange = timeRange.clamped(OTIO_NS::TimeRange(
                                                                  OTIO_NS::RationalTime(0.F, startTime.rate()),
                                                                  OTIO_NS::RationalTime(1.F, duration.rate())));
                            }
                            
                            otioItem->set_source_range(timeRange);

                            OTIO_NS::TimeRange gapRange;
                            int otioGapIndex = 0;
                            if (p.mouse.side == Private::MouseClick::Left &&
                                timeRange.start_time() > origRange.start_time())
                            {
                                gapRange = OTIO_NS::TimeRange(
                                    OTIO_NS::RationalTime(0,
                                                        timeRange.duration().rate()),
                                    -durationOffset);
                                otioGapIndex = otioItemIndex;
                                OTIO_NS::Gap* gap = new OTIO_NS::Gap(gapRange);
                                otioTrack->insert_child(otioGapIndex, gap);
                            }
                            else if (p.mouse.side == Private::MouseClick::Right &&
                                     timeRange.duration() < origRange.duration())
                            {
                                gapRange = OTIO_NS::TimeRange(
                                    OTIO_NS::RationalTime(0,
                                                        timeRange.duration().rate()),
                                    -durationOffset);
                                otioGapIndex = otioItemIndex + 1;
                                OTIO_NS::Gap* gap = new OTIO_NS::Gap(gapRange);
                                otioTrack->insert_child(otioGapIndex, gap);
                            }
                        }
                    }
                }
                p.player->setTimeline(otioTimeline);
                break;
            }
            case Private::MouseMode::Transition:
            {
                _mouse.pos.y = _mouse.pressPos.y;
                const int offset = _mouse.pos.x - _mouse.pressPos.x;
                math::Box2i move;
                std::vector<timeline::MoveData> moveData;
                for (const auto& item : p.mouse.items)
                {
                    const math::Box2i& g = item->geometry;
                    OTIO_NS::TimeRange timeRange = item->p->getTimeRange();
                    
                    // Get transition items (ie. the clips associated to the
                    // transition) time ranges.
                    const int transitionTrack = item->track;
                    const int transitionIndex = item->index;
                    std::vector<OTIO_NS::TimeRange> itemRanges;
                    _getTransitionTimeRanges(itemRanges, transitionTrack,
                                             timeRange);
                    
                    const OTIO_NS::RationalTime oneFrame =
                        OTIO_NS::RationalTime(1.F, timeRange.duration().rate());

                    if (p.mouse.side == Private::MouseClick::Left)
                    {
                        math::Size2i size = g.getSize();
                        size.w -= offset;
                        
                        move = math::Box2i(
                            g.min + _mouse.pos - _mouse.pressPos,
                            size );

                    }
                    else if (p.mouse.side == Private::MouseClick::Right)
                    {
                        math::Size2i size = g.getSize();
                        size.w += offset;
                        move = math::Box2i(g.min, size);
                    }

                    OTIO_NS::RationalTime startTime = posToTime(move.x());
                    OTIO_NS::RationalTime duration  = posToTime(move.x() + move.w()) -
                                                    startTime;

                    startTime -= _timeRange.start_time();
                    timeRange = OTIO_NS::TimeRange(startTime, duration);

                    // Clamp on clips.
                    if (duration.value() <= 2.F)
                    {
                        duration  = OTIO_NS::RationalTime(2.F, duration.rate());
                        timeRange = OTIO_NS::TimeRange(startTime, duration);
                        
                    }
                    
                    
                    if (timeRange.start_time() <= itemRanges[0].start_time())
                    {
                        auto diff = itemRanges[0].start_time() - timeRange.start_time()
                                    + oneFrame;
                        timeRange = OTIO_NS::TimeRange(itemRanges[0].start_time() + oneFrame,
                                                     timeRange.duration() - diff);
                    }
                    
                    if (timeRange.start_time() >= itemRanges[0].end_time_exclusive())
                    {
                        timeRange = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                            itemRanges[1].start_time() - oneFrame,
                            timeRange.end_time_inclusive());
                    }
                        
                    if (timeRange.end_time_exclusive() >= itemRanges[1].end_time_inclusive())
                    {
                        timeRange = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                            timeRange.start_time(),
                            itemRanges[1].end_time_inclusive());
                        timeRange = timeRange.duration_extended_by(-oneFrame);
                    }
                    
                    if (timeRange.end_time_exclusive() <= itemRanges[0].end_time_inclusive())
                    {
                        timeRange = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                            timeRange.start_time(),
                            itemRanges[0].end_time_inclusive());
                        timeRange = timeRange.duration_extended_by(oneFrame);
                    }
                    
                    // Clamp on other transitions.
                    if (_transitionIntersects(item->p, transitionTrack, timeRange))
                    {
                        for (const auto& transition : p.tracks[transitionTrack].transitions)
                        {
                            if (item->p == transition)
                                continue;
                            const OTIO_NS::TimeRange transitionRange = transition->getTimeRange();
                            if (timeRange.start_time() <= transitionRange.end_time_exclusive())
                            {
                                auto diff = transitionRange.end_time_inclusive() - timeRange.start_time();
                                timeRange = OTIO_NS::TimeRange(timeRange.start_time() + diff + oneFrame,
                                                             timeRange.duration() - diff - oneFrame);

                            }
                            else if (timeRange.end_time_exclusive() >= transitionRange.start_time())
                            {
                                timeRange = OTIO_NS::TimeRange(transitionRange.start_time() - oneFrame,
                                                             timeRange.duration() + oneFrame);
                            }
                        }
                    }
                    
                    const OTIO_NS::RationalTime in_offset = itemRanges[1].start_time() -
                                                          timeRange.start_time();
                    const OTIO_NS::RationalTime out_offset = timeRange.end_time_exclusive() -
                                                           itemRanges[1].start_time();
                    assert(timeRange.contains(itemRanges[1].start_time()));
                    assert(in_offset.value() >= 1.F);
                    assert(out_offset.value() >= 1.F);
                    
                    const int transitionOtioIndex =
                        p.tracks[transitionTrack].otioTransitionIndexes[transitionIndex];
                    moveData.push_back(
                        {
                            timeline::MoveType::Transition,
                            transitionTrack, transitionIndex, transitionOtioIndex,
                            transitionTrack, transitionIndex, transitionOtioIndex,
                            in_offset, out_offset
                        });
                }
                if (p.moveCallback)
                    p.moveCallback(moveData);
                auto otioTimeline = timeline::move(
                    p.player->getTimeline()->getTimeline().value, moveData);
                p.player->setTimeline(otioTimeline);
            }
            }   
        }

    }
}

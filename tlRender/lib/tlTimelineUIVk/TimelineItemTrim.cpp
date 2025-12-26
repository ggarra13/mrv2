// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <tlTimelineUIVk/TimelineItem.h>
#include <tlTimelineUIVk/TimelineItemPrivate.h>

namespace tl
{
    namespace timelineui_vk
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
                    otime::TimeRange timeRange = clip->getTimeRange();
                    const otio::Item* otioItem = clip->getOtioItem();

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

                    // Move the start and duration to clip time from timeline time.
                    otime::RationalTime startTime = posToTime(move.x());
                    otime::RationalTime duration  = posToTime(move.x() + move.w()) - startTime;
                    startTime -= _timeRange.start_time();

                    // Clamp on available range if present.
                    otio::ErrorStatus status;
                    const auto& availableRange = otioItem->available_range(&status);
                    if (!otio::is_error(status))
                    {
                        if (startTime < availableRange.start_time())
                            continue;
                        if (duration > availableRange.duration())
                            continue;
                    }
                    else
                    {
                        if (startTime < otime::RationalTime(0, startTime.rate()))
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
                
                std::vector<timeline::MoveData> moveData;
                moveData.push_back(
                    {
                        timeline::MoveType::UndoOnly
                    });
                if (p.moveCallback)
                    p.moveCallback(moveData);
                
                const auto otioTimeline = p.player->getTimeline()->getTimeline();
                for (const auto& item : p.mouse.items)
                {
                    const math::Box2i& g = item->geometry;
                    otime::TimeRange timeRange = item->p->getTimeRange();
                    otime::TimeRange origRange = timeRange;
                                        
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

                    otime::RationalTime startTime = posToTime(move.x());
                    otime::RationalTime duration  = posToTime(move.x() + move.w()) -
                                                    startTime;

                    startTime -= _timeRange.start_time();
                    timeRange = otime::TimeRange(startTime, duration);
                    
                    const auto startOffset = timeRange.start_time() - origRange.start_time();
                    const auto durationOffset = timeRange.duration() - origRange.duration();
                    
                    const int trackIndex = item->track;
                    const int itemIndex = item->index;
                    const int otioItemIndex = p.tracks[trackIndex].otioIndexes[itemIndex];

                    otio::ErrorStatus status;
                    const auto& child = otioTimeline->tracks()->children()[trackIndex];
                    if (auto otioTrack = otio::dynamic_retainer_cast<otio::Track>(child))
                    {
                        const auto& otioChild = otioTrack->children()[otioItemIndex];
                        if (auto otioItem = otio::dynamic_retainer_cast<otio::Clip>(otioChild))
                        {
                            origRange = otioItem->source_range().value();
                            
                            const auto& availableRange = otioItem->available_range(&status);              
                            auto startTime = origRange.start_time() + startOffset;
                            auto duration  = origRange.duration() + durationOffset;
                            timeRange = otime::TimeRange(startTime, duration);

                            // Clamp to available range if present
                            if (!otio::is_error(status))
                            {
                                timeRange = timeRange.clamped(availableRange);
                            }
                            else
                            {
                                timeRange = timeRange.clamped(otime::TimeRange(
                                                                  otime::RationalTime(0.F, startTime.rate()),
                                                                  otime::RationalTime(1.F, duration.rate())));
                            }
                            
                            otioItem->set_source_range(timeRange);
                        }
                    }
                }
                p.player->getTimeline()->setTimeline(otioTimeline);
                break;
            }
            case Private::MouseMode::Transition:
            {
                _mouseReleaseEventRoll(event);
            }
            }   
        }

    }
}

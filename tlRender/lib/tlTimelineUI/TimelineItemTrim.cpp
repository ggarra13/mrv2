// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <tlTimelineUI/TimelineItem.h>
#include <tlTimelineUI/TimelineItemPrivate.h>

namespace tl
{
    namespace timelineui
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
                _mouseMoveEventTrim(event);                
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

                    const otime::RationalTime& startTime = posToTime(move.x());
                    const otime::RationalTime& duration  = posToTime(move.x() + move.w()) - startTime;
                        
                    timeRange = otime::TimeRange(startTime - _timeRange.start_time(), duration);

                    // Clamp on clips.
                    if (duration.value() <= 1.F)
                    {
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
#if DEBUG_TIME_RANGES
                    std::cerr << __LINE__ << " shifted time range=" << timeRange << std::endl;
#endif

                    // Clamp on clips.
                    if (duration.value() < 1.F)
                    {
                        duration  = otime::RationalTime(1.F, duration.rate());
                        timeRange = otime::TimeRange(startTime, duration);   
                    }
                    
                    const auto startOffset = timeRange.start_time() - origRange.start_time();
                    const auto durationOffset = timeRange.duration() - origRange.duration();
                    
                    const int trackIndex = item->track;
                    const int itemIndex = item->index;
                    const int otioItemIndex = p.tracks[trackIndex].otioIndexes[itemIndex];

                    const auto& child = otioTimeline->tracks()->children()[trackIndex];
                    if (auto otioTrack = otio::dynamic_retainer_cast<otio::Track>(child))
                    {
                        const auto& otioChild = otioTrack->children()[otioItemIndex];
                        if (auto otioItem = otio::dynamic_retainer_cast<otio::Clip>(otioChild))
                        {
                            origRange = otioItem->source_range().value();
                            timeRange = otime::TimeRange(origRange.start_time() + startOffset,
                                                         origRange.duration() + durationOffset);
                            otioItem->set_source_range(timeRange);
                        }
                    }
                }
                p.player->getTimeline()->setTimeline(otioTimeline);
                break;
            }
            case Private::MouseMode::Transition:
            {
                _mouseReleaseEventTrim(event);
            }
            }   
        }

    }
}

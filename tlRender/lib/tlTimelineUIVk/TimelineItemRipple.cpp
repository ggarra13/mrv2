// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <tlTimelineUIVk/TimelineItem.h>
#include <tlTimelineUIVk/TimelineItemPrivate.h>

namespace tl
{
    namespace timelineui_vk
    {
        
        void TimelineItem::_mouseMoveEventRipple(ui::MouseMoveEvent& event)
        {
            TLRENDER_P();
            if (p.mouse.items.empty())
                return;
            switch (p.mouse.mode)
            {
            case Private::MouseMode::kNone:
                break;
            case Private::MouseMode::Transition:
                break;
            case Private::MouseMode::Item:
            {
                _mouse.pos.y = _mouse.pressPos.y;
                // const int offset = _mouse.pos.x - _mouse.pressPos.x;
                math::Box2i move;
                    
                const auto otioTimeline = p.player->getTimeline()->getTimeline();
                for (const auto& item : p.mouse.items)
                {
                    const math::Box2i& g = item->geometry;
                    auto clip = static_cast<IBasicItem*>(item->p.get());
                    otime::TimeRange timeRange = clip->getTimeRange();
                    otio::Item* otioItem = const_cast<otio::Item*>(clip->getOtioItem());
                    

                    // Move the start and duration to clip time from timeline time.
                    otime::RationalTime startTime = posToTime(_mouse.pressPos.x);
                    otime::RationalTime   endTime = posToTime(_mouse.pos.x);
                    otime::RationalTime   offset  = endTime - startTime;

                    std::cerr << "startTime=" << startTime << std::endl;
                    std::cerr << "  endTime=" << endTime << std::endl; 
                    std::cerr << "   offset=" << offset << std::endl;

                     
                    auto itemRange = otioItem->trimmed_range();
                    startTime = itemRange.start_time() + offset;
                    auto duration = itemRange.duration() + offset;
                    
                    // Clamp on available range if present.
                    otio::ErrorStatus status;
                    const auto& availableRange = otioItem->available_range(&status);
                    if (!otio::is_error(status))
                    {
                        if (startTime < availableRange.start_time())
                            startTime = availableRange.start_time();
                        if (duration > availableRange.duration())
                            duration = availableRange.duration();
                    }
                    else
                    {
                        if (startTime < 
                            otime::RationalTime(0, startTime.rate()))
                            startTime = otime::RationalTime(0,
                                                            startTime.rate());
                        if (duration.value() <= 1.F)
                            duration = otime::RationalTime(1.F,
                                                           duration.rate());
                    }

                    const otime::TimeRange newTimeRange(startTime, duration);
                    if (auto clip = dynamic_cast<otio::Clip*>(otioItem))
                    {
                        clip->set_source_range(newTimeRange);
                    }
                    else if (auto gap = dynamic_cast<otio::Gap*>(otioItem))
                    {
                        gap->set_source_range(newTimeRange);
                    }
                }

                //p.player->getTimeline()->setTimeline(otioTimeline);

            }
            break;
            }
        }

        void TimelineItem::_mouseReleaseEventRipple(ui::MouseClickEvent& event)
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
                break;
            }
            case Private::MouseMode::Item:
            {              
                const auto otioTimeline = p.player->getTimeline()->getTimeline();
                p.player->getTimeline()->setTimeline(otioTimeline);
                break;
            }
            }
        }

    }
}

// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "TimelineItem.h"
#include "TimelineItemPrivate.h"

namespace tl
{
    namespace TIMELINEUI
    {
        
        void TimelineItem::_mouseMoveEventSlip(ui::MouseMoveEvent& event)
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
                for (const auto& item : p.mouse.items)
                {
                    const math::Box2i& g = item->geometry;
                    auto clip = static_cast<IBasicItem*>(item->p.get());
                    otime::TimeRange trimmedRange = clip->getTrimmedRange();
                    otio::Item* otioItem = const_cast<otio::Item*>(clip->getOtioItem());

                    // Move the start and duration to clip time from timeline time.
                    otime::RationalTime startTime = posToTime(event.prev.x);
                    const otime::RationalTime endTime = posToTime(event.pos.x);
                    const otime::RationalTime offset  = endTime - startTime;
                     
                    startTime = trimmedRange.start_time() + offset;
                    auto duration = trimmedRange.duration();
                    
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
                    clip->setTrimmedRange(newTimeRange);
                    
                    if (auto otioClip = dynamic_cast<otio::Clip*>(otioItem))
                    {
                        otioClip->set_source_range(newTimeRange);
                    }
                    
                }
            }
            break;
            }
        }

        void TimelineItem::_mouseReleaseEventSlip(ui::MouseClickEvent& event)
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
                const auto& otioTimeline = p.player->getTimeline()->getTimeline();
                p.player->setTimeline(otioTimeline);
                break;
            }
            }
        }

    }
}

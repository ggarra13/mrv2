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
                    OTIO_NS::TimeRange trimmedRange = clip->getTrimmedRange();
                    OTIO_NS::Item* otioItem = const_cast<OTIO_NS::Item*>(clip->getOtioItem());

                    // Move the start and duration to clip time from timeline time.
                    OTIO_NS::RationalTime startTime = posToTime(event.prev.x);
                    const OTIO_NS::RationalTime endTime = posToTime(event.pos.x);
                    const OTIO_NS::RationalTime offset  = endTime - startTime;
                     
                    startTime = trimmedRange.start_time() + offset;
                    auto duration = trimmedRange.duration();
                    
                    // Clamp on available range if present.
                    OTIO_NS::ErrorStatus status;
                    const auto& availableRange = otioItem->available_range(&status);
                    if (!OTIO_NS::is_error(status))
                    {
                        if (startTime < availableRange.start_time())
                            startTime = availableRange.start_time();

                        if (duration > availableRange.duration())
                            duration = availableRange.duration();
                    }
                    else
                    {
                        if (startTime < 
                            OTIO_NS::RationalTime(0, startTime.rate()))
                            startTime = OTIO_NS::RationalTime(0,
                                                            startTime.rate());
                        if (duration.value() <= 1.F)
                            duration = OTIO_NS::RationalTime(1.F,
                                                           duration.rate());
                    }

                    const OTIO_NS::TimeRange newTimeRange(startTime, duration);
                    clip->setTrimmedRange(newTimeRange);
                    
                    if (auto otioClip = dynamic_cast<OTIO_NS::Clip*>(otioItem))
                    {
                        otioClip->set_source_range(newTimeRange);
                    }
                    
                }
            }
            default:
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
            default:
                break;
            }
        }

    }
}

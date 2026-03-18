// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "TimelineItem.h"
#include "TimelineItemPrivate.h"

namespace tl
{
    namespace TIMELINEUI
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
            {
                _mouseMoveEventRoll(event);                
                break;
            }
            case Private::MouseMode::Item:
            {
                break;
            }
            default:
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
            case Private::MouseMode::CurrentTime:
            {
                break;
            }
            case Private::MouseMode::Item:
            {
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

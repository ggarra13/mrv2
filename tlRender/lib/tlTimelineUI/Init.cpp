// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/ThumbnailSystem.h>
#include <tlTimelineUI/Init.h>

#include <tlUI/Init.h>

#include <tlTimeline/Init.h>

namespace tl
{
    namespace timelineui
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            tl::timeline::init(context);
            tl::ui::init(context);
            if (!context->getSystem<timelineui::ThumbnailSystem>())
            {
                context->addSystem(timelineui::ThumbnailSystem::create(context));
            }
        }
    } // namespace timelineui
} // namespace tl

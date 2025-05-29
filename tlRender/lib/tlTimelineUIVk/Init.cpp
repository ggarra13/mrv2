// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineUIVk/ThumbnailSystem.h>
#include <tlTimelineUIVk/Init.h>

#include <tlUI/Init.h>

#include <tlTimeline/Init.h>

namespace tl
{
    namespace timelineui_vk
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            tl::timeline::init(context);
            tl::ui::init(context);

            // We cannot init thumbnail system here as we need the FLTK context
            // if (!context->getSystem<timelineui_vk::ThumbnailSystem>())
            // {
            //     Fl_Vk_Context ctx;
            //     context->addSystem(timelineui_vk::ThumbnailSystem::create(context, ctx));
            // }
        }
    } // namespace timelineui_vk
} // namespace tl

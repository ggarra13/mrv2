// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "ThumbnailSystem.h"
#include "Init.h"

#include <tlUI/Init.h>

#include <tlTimeline/Init.h>

namespace tl
{
    namespace TIMELINEUI
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            tl::timeline::init(context);
            tl::ui::init(context);

#ifdef OPENGL_BACKEND
            // We cannot init the Vulkan thumbnail system here as we need
            // the FLTK context
            if (!context->getSystem<TIMELINEUI::ThumbnailSystem>())
            {
                context->addSystem(TIMELINEUI::ThumbnailSystem::create(context));
            }
#endif
        }
    } // namespace TIMELINEUI
} // namespace tl

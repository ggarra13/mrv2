// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvBackend.h"

#include "mrvFl/mrvInit.h"

#include <tlTimeline/Init.h>

#ifdef OPENGL_BACKEND
#include <tlTimelineUI/Init.h>
#endif

#ifdef VULKAN_BACKEND
#include <tlTimelineUIVk/Init.h>
#endif

#ifdef TLRENDER_NDI
#    include <tlDevice/Init.h>
#endif

namespace mrv
{
    void init(const std::shared_ptr<tl::system::Context>& context)
    {
        using namespace tl;

#ifdef OPENGL_BACKEND
        timelineui::init(context);
#endif

#ifdef VULKAN_BACKEND
        timelineui_vk::init(context);
#endif

#ifdef TLRENDER_NDI
        device::init(context);
#endif
    }

} // namespace mrv

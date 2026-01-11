// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvBackend.h"

#include "mrvFl/mrvInit.h"

#include <tlTimeline/Init.h>

#include <tlTimelineUI/Init.h>

#ifdef TLRENDER_NDI
#    include <tlDevice/Init.h>
#endif

namespace mrv
{
    void init(const std::shared_ptr<tl::system::Context>& context)
    {
        using namespace tl;

        TIMELINEUI::init(context);

#ifdef TLRENDER_NDI
        device::init(context);
#endif
    }

} // namespace mrv

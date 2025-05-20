// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.
#include <iostream>

#include <tlDevice/Init.h>

#if defined(TLRENDER_BMD)
#    include <tlDevice/BMD/BMDSystem.h>
#endif // TLRENDER_BMD

#if defined(TLRENDER_NDI) && !defined(VULKAN_BACKEND)
#    include <tlDevice/NDI/NDISystem.h>
#endif // TLRENDER_NDI

#include <tlTimeline/Init.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace device
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            timeline::init(context);
#if defined(TLRENDER_BMD)
            if (!context->getSystem<bmd::System>())
            {
                context->addSystem(bmd::System::create(context));
            }
#endif // TLRENDER_BMD
#if defined(TLRENDER_NDI) && !defined(VULKAN_BACKEND)
            if (!context->getSystem<ndi::System>())
            {
                context->addSystem(ndi::System::create(context));
            }
#endif // TLRENDER_NDI
        }
    } // namespace device
} // namespace tl

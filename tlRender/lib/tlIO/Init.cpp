// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/Init.h>

#include <tlIO/System.h>

#ifndef VULKAN_BACKEND
#    include <tlGL/Init.h>
#endif

#include <tlCore/Context.h>

namespace tl
{
    namespace io
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
#ifndef VULKAN_BACKEND
            gl::init(context);
#endif
            if (!context->getSystem<System>())
            {
                context->addSystem(System::create(context));
            }
        }
    } // namespace io
} // namespace tl

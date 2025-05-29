// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/Init.h>

#include <tlIO/Init.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace ui
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            tl::io::init(context);
        }
    } // namespace ui
} // namespace tl

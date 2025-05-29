// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/IRender.h>

namespace tl
{
    namespace timeline
    {
        void IRender::_init(const std::shared_ptr<system::Context>& context)
        {
            _context = context;
        }

        IRender::IRender() {}

        IRender::~IRender() {}
    } // namespace timeline
} // namespace tl

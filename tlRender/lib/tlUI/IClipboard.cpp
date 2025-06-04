// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/IClipboard.h>

namespace tl
{
    namespace ui
    {
        void IClipboard::_init(const std::shared_ptr<system::Context>& context)
        {
            _context = context;
        }

        IClipboard::IClipboard() {}

        IClipboard::~IClipboard() {}
    } // namespace ui
} // namespace tl

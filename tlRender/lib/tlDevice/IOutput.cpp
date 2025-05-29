// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Gonzalo Garramuño
// All rights reserved.

#include <tlDevice/IOutput.h>

namespace tl
{
    namespace device
    {
        void IOutput::_init(const std::shared_ptr<system::Context>& context)
        {
            _context = context;
        }

        IOutput::IOutput() {}

        IOutput::~IOutput() {}
    } // namespace device
} // namespace tl

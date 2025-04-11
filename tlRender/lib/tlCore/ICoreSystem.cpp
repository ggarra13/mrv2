// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/ICoreSystem.h>

namespace tl
{
    namespace system
    {
        void ICoreSystem::_init(
            const std::string& name, const std::shared_ptr<Context>& context)
        {
            _context = context;
            _name = name;
        }

        ICoreSystem::ICoreSystem() {}

        ICoreSystem::~ICoreSystem() {}

        void ICoreSystem::tick() {}

        std::chrono::milliseconds ICoreSystem::getTickTime() const
        {
            return std::chrono::milliseconds(0);
        }
    } // namespace system
} // namespace tl

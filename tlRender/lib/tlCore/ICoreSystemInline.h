// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace system
    {
        inline const std::weak_ptr<Context>& ICoreSystem::getContext() const
        {
            return _context;
        }

        inline const std::string& ICoreSystem::getName() const
        {
            return _name;
        }
    } // namespace system
} // namespace tl

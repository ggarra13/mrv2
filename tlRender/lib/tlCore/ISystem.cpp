// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/ISystem.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace system
    {
        void ISystem::_init(
            const std::string& name, const std::shared_ptr<Context>& context)
        {
            ICoreSystem::_init(name, context);

            _logSystem = context->getSystem<log::System>();

            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print(name, "Create");
            }
        }

        ISystem::ISystem() {}

        ISystem::~ISystem()
        {
            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print(_name, "Delete");
            }
        }

        void ISystem::_log(const std::string& value, log::Type type)
        {
            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print(_name, value, type);
            }
        }
    } // namespace system
} // namespace tl

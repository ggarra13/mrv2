// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/LogSystem.h>

namespace tl
{
    namespace log
    {
        class LogSystem;
    }

    namespace system
    {
        //! Base class for systems.
        class ISystem : public ICoreSystem
        {
        protected:
            void
            _init(const std::string& name, const std::shared_ptr<Context>&);
            ISystem();

        public:
            virtual ~ISystem();

        protected:
            void _log(const std::string&, log::Type = log::Type::Message);

        private:
            std::weak_ptr<log::System> _logSystem;
        };
    } // namespace system
} // namespace tl

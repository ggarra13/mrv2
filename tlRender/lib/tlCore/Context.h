// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/LogSystem.h>

#include <list>
#include <map>

namespace tl
{
    namespace system
    {
        class ICoreSystem;

        //! Context.
        class Context : public std::enable_shared_from_this<Context>
        {
            TLRENDER_NON_COPYABLE(Context);

        protected:
            void _init();
            Context();

        public:
            ~Context();

            //! Create a new context.
            static std::shared_ptr<Context> create();

            //! Add a system.
            void addSystem(const std::shared_ptr<ICoreSystem>&);

            //! Remove  a system.
            void removeSystem(const std::shared_ptr<ICoreSystem>&);
            
            //! Get the log system.
            const std::shared_ptr<log::System>& getLogSystem() const;

            //! Get a system.
            template <typename T> std::shared_ptr<T> getSystem() const;

            //! Print to the log.
            void
            log(const std::string& prefix, const std::string&,
                log::Type = log::Type::Message);

            //! Tick the context.
            void tick();

        private:
            std::shared_ptr<log::System> _logSystem;
            std::list<std::shared_ptr<ICoreSystem> > _systems;
            std::map<
                std::shared_ptr<ICoreSystem>,
                std::chrono::steady_clock::time_point>
                _systemTimes;
            TLRENDER_PRIVATE();
        };
    } // namespace system
} // namespace tl

#include <tlCore/ContextInline.h>

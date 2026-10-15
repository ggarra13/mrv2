// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <chrono>
#include <memory>
#include <string>

namespace tl
{
    //! Systems
    namespace system
    {
        class Context;

        //! Base class for core systems.
        class ICoreSystem : public std::enable_shared_from_this<ICoreSystem>
        {
            TLRENDER_NON_COPYABLE(ICoreSystem);

        protected:
            void
            _init(const std::shared_ptr<Context>&, const std::string& name);
            ICoreSystem();

        public:
            virtual ~ICoreSystem() = 0;

            //! Get the context.
            const std::weak_ptr<Context>& getContext() const;

            //! Get the system name.
            const std::string& getName() const;

            //! Stop the system's threads.
            //!
            //! Called by the context before it lets go of its systems, so that a
            //! system with threads is stopped while the context is still alive
            //! and on the thread that owns it. A thread that has locked the
            //! context weakly and is still running when the last reference goes
            //! would otherwise destroy the context itself, and take its own
            //! system down with it -- leaving the system's destructor joining the
            //! thread it is running on.
            //!
            //! Must be safe to call more than once, and safe to call on a system
            //! that has no threads. The default does nothing.
            virtual void shutdown();

            //! Tick the system.
            virtual void tick();

            //! Get the system tick time interval.
            virtual std::chrono::milliseconds getTickTime() const;

        protected:
            std::weak_ptr<Context> _context;
            std::string _name;
        };
    } // namespace system
} // namespace tl

#include <tlCore/ICoreSystemInline.h>

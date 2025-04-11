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
            _init(const std::string& name, const std::shared_ptr<Context>&);
            ICoreSystem();

        public:
            virtual ~ICoreSystem() = 0;

            //! Get the context.
            const std::weak_ptr<Context>& getContext() const;

            //! Get the system name.
            const std::string& getName() const;

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

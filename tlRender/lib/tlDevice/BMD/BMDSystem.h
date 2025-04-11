// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/OutputData.h>

#include <tlCore/ISystem.h>
#include <tlCore/ListObserver.h>

namespace tl
{
    namespace bmd
    {
        //! BMD system.
        class System : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(System);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            System();

        public:
            ~System() override;

            //! Create a new system.
            static std::shared_ptr<System>
            create(const std::shared_ptr<system::Context>&);

            //! Observe the device information.
            std::shared_ptr<observer::IList<device::DeviceInfo> >
            observeDeviceInfo() const;

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace bmd
} // namespace tl

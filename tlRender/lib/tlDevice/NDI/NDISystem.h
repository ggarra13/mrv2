// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Gonzalo Garramuño
// All rights reserved.
#pragma once

#include <tlDevice/NDI/NDI.h>
#include <tlDevice/OutputData.h>

#include <tlCore/ISystem.h>
#include <tlCore/ListObserver.h>

#include <string>

namespace tl
{
    namespace ndi
    {
        static NDIlib_v5* pNDILib = nullptr;

        //! NDI system.
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

            //! Observe the connections.
            std::shared_ptr<observer::IList<device::DeviceInfo> >
            observeDeviceInfo() const;

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            std::string NDI_library();

            TLRENDER_PRIVATE();
        };
    } // namespace ndi
} // namespace tl

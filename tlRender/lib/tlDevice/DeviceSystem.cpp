// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlDevice/DeviceSystem.h>

#include <tlDevice/IOutput.h>

#include <tlCore/Context.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <atomic>
#include <mutex>
#include <thread>

namespace tl
{
    namespace device
    {
        struct DeviceSystem::Private
        {
            std::shared_ptr<observer::List<DeviceInfo> > deviceInfo;
            struct Mutex
            {
                std::vector<DeviceInfo> deviceInfo;
                std::mutex mutex;
            };
            Mutex mutex;
            std::thread thread;
            std::atomic<bool> running;
        };

        void
        DeviceSystem::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init(context, "tl::device::System");
            TLRENDER_P();

            p.deviceInfo = observer::List<DeviceInfo>::create();

            p.running = true;
            p.thread = std::thread(
                [this]
                {
                    TLRENDER_P();

                    bool log = true;
                    while (p.running)
                    {
                        const auto t0 = std::chrono::steady_clock::now();

                        std::vector<DeviceInfo> deviceInfoList;

                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            log = deviceInfoList != p.mutex.deviceInfo;
                            p.mutex.deviceInfo = deviceInfoList;
                        }

                        if (log)
                        {
                            log = false;
                            if (auto context = _context.lock())
                            {
                                for (const auto& i : deviceInfoList)
                                {
                                    std::vector<std::string> displayModes;
                                    for (const auto& j : i.displayModes)
                                    {
                                        displayModes.push_back(j.name);
                                    }
                                    context->log(
                                        "tl::device::System",
                                        string::Format(
                                            "\n"
                                            "    {0}\n"
                                            "        Display modes: {1}\n"
                                            "        Min video preroll: {2}\n"
                                            "        HDR metadata: {3}\n"
                                            "        Max audio channels: {4}")
                                            .arg(i.name)
                                            .arg(string::join(
                                                displayModes, ", "))
                                            .arg(i.minVideoPreroll)
                                            .arg(i.hdrMetaData)
                                            .arg(i.maxAudioChannels));
                                }
                            }
                        }

                        const auto t1 = std::chrono::steady_clock::now();
                        time::sleep(getTickTime(), t0, t1);
                    }

#if defined(_WIN32)
                    CoUninitialize();
#endif // _WIN32
                });
        }

        DeviceSystem::DeviceSystem(
            const std::shared_ptr<system::Context>& context) :
            ISystem::ISystem(context, "tl::device::System"),
            _p(new Private)
        {
        }

        DeviceSystem::~DeviceSystem()
        {
            TLRENDER_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<DeviceSystem>
        DeviceSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<DeviceSystem>(new DeviceSystem(context));
            out->_init(context);
            return out;
        }

        std::shared_ptr<observer::IList<DeviceInfo> >
        DeviceSystem::observeDeviceInfo() const
        {
            return _p->deviceInfo;
        }

        void DeviceSystem::tick()
        {
            TLRENDER_P();
            std::vector<DeviceInfo> deviceInfo;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                deviceInfo = p.mutex.deviceInfo;
            }
            p.deviceInfo->setIfChanged(deviceInfo);
        }

        std::chrono::milliseconds DeviceSystem::getTickTime() const
        {
            return std::chrono::milliseconds(1000);
        }
    } // namespace device
} // namespace tl

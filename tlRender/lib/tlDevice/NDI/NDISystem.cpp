// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Gonzalo Garramuño
// All rights reserved.

// #define USE_LIBRARY

#include <tlDevice/NDI/NDISystem.h>

#include <tlCore/Context.h>
#include <tlCore/Library.h>
#include <tlCore/LogSystem.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <FL/fl_utf8.h>

#include <atomic>
#include <iostream>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;
#include <mutex>
#include <regex>
#include <thread>
#include <stdexcept>

namespace tl
{
    namespace
    {
        std::string rootpath()
        {
            const char* root = fl_getenv("MRV2_ROOT");
            if (!root)
                root = "..";
            return root;
        }

        bool isReadable(const fs::path& p)
        {
            const std::string& filePath = p.generic_string();
            if (filePath.empty())
                return false;

            std::ifstream f(filePath);
            if (f.is_open())
            {
                f.close();
                return true;
            }

            return false;
        }
    } // namespace
} // namespace tl

namespace tl
{
    namespace ndi
    {
        struct System::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<observer::List<device::DeviceInfo> > deviceInfo;

            LibraryHandle handle;
            NDIlib_find_instance_t NDIfind = nullptr;

            struct Mutex
            {
                std::vector<device::DeviceInfo> deviceInfo;
                std::mutex mutex;
            };
            Mutex mutex;
            std::thread thread;
            std::atomic<bool> running;
        };

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::ndi::System", context);
            TLRENDER_P();

            p.context = context;

            p.deviceInfo = observer::List<device::DeviceInfo>::create();

#ifdef TLRENDER_NDI_DYNAMIC_LIBRARY
            const std::string& ndi_library = NDI_library();
            p.handle = library::loadLibrary(ndi_library);
            if (!p.handle)
            {
                throw std::runtime_error("Could not load NDI library");
            }
            pNDILib = static_cast<NDIlib_v5*>(
                library::findFunction(p.handle, "NDIlib_v5_load"));
            if (!pNDILib)
            {
                throw std::runtime_error("Could not find NDIlib_v5_load in "
                                         "library");
            }

            pNDILib->NDIlib_initialize();
            ;

            NDIlib_find_create_t findOptions;
            p.NDIfind = pNDILib->NDIlib_find_create_v2(&findOptions);
#else
            NDIlib_initialize();
            p.NDIfind = NDIlib_find_create_v2();
#endif
            p.running = true;
            p.thread = std::thread(
                [this]
                {
                    TLRENDER_P();

                    bool log = true;
                    while (p.running)
                    {
                        const auto t0 = std::chrono::steady_clock::now();

                        std::vector<device::DeviceInfo> deviceInfoList;

                        uint32_t no_sources = 0;

                        const NDIlib_source_t* p_sources = NULL;
                        NDIlib_find_wait_for_sources(p.NDIfind, 1000);

                        p_sources = NDIlib_find_get_current_sources(
                            p.NDIfind, &no_sources);

                        for (unsigned i = 0; i < no_sources; ++i)
                        {
                            const std::string deviceName =
                                p_sources[i].p_ndi_name;

                            // Windows has weird items called REMOTE CONNECTION.
                            // We don't allow selecting them.
                            static const std::regex pattern(
                                "remote connection",
                                std::regex_constants::icase);
                            if (std::regex_search(deviceName, pattern))
                                continue;

                            device::DeviceInfo deviceInfo;
                            deviceInfo.name = deviceName;

                            deviceInfo.pixelTypes.push_back(
                                device::PixelType::_8BitBGRA);
                            deviceInfo.pixelTypes.push_back(
                                device::PixelType::_8BitYUV);
                            // deviceInfo.pixelTypes.push_back(device::PixelType::_10BitRGB);
                            // deviceInfo.pixelTypes.push_back(device::PixelType::_10BitRGBX);
                            // deviceInfo.pixelTypes.push_back(device::PixelType::_10BitRGBXLE);
                            // //deviceInfo.pixelTypes.push_back(device::PixelType::_10BitYUV);
                            // deviceInfo.pixelTypes.push_back(device::PixelType::_12BitRGB);
                            // deviceInfo.pixelTypes.push_back(device::PixelType::_12BitRGBLE);
                            deviceInfo.pixelTypes.push_back(
                                device::PixelType::_8BitUYVA);
#ifdef TLRENDER_NDI_ADVANCED
                            deviceInfo.pixelTypes.push_back(
                                device::PixelType::_16BitP216);
                            deviceInfo.pixelTypes.push_back(
                                device::PixelType::_16BitPA16);
#endif
                            // deviceInfo.pixelTypes.push_back(device::PixelType::_8BitI420);
                            // // \@bug: this does nothing
                            deviceInfo.pixelTypes.push_back(
                                device::PixelType::_8BitBGRX);
                            deviceInfo.pixelTypes.push_back(
                                device::PixelType::_8BitRGBA);
                            deviceInfo.pixelTypes.push_back(
                                device::PixelType::_8BitRGBX);

                            deviceInfoList.push_back(deviceInfo);
                        }

                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            log = deviceInfoList != p.mutex.deviceInfo;
                            p.mutex.deviceInfo = deviceInfoList;
                        }
                    }
                });
        }

        System::System() :
            _p(new Private)
        {
        }

        //! Path to NDI (if installed and found)
        std::string System::NDI_library()
        {
            TLRENDER_P();

            const std::string library = NDILIB_LIBRARY_NAME;
#ifdef _WIN32
            std::string libpath = rootpath() + "/bin/";
#else
            std::string libpath = rootpath() + "/lib/";
#endif
            std::string fullpath = libpath + library;
            if (!isReadable(fullpath))
            {
                const char* env = fl_getenv(NDILIB_REDIST_FOLDER);
                if (env)
                    libpath = env;
                if (!libpath.empty())
                {
                    fullpath = libpath + "/" + library;
                }
                else
                {
                    libpath = "/usr/local/lib/";
                    fullpath = libpath + library;
                    if (!isReadable(fullpath))
                    {
                        throw std::runtime_error(NDILIB_LIBRARY_NAME
                                                 " was not found.  "
                                                 "Please download it from "
                                                 "http://ndi.link/NDIRedistV6");
                    }
                }
            }

            std::string msg = string::Format("Will load {0} from {1}.")
                                  .arg(library)
                                  .arg(libpath);

            if (auto context = p.context.lock())
            {
                auto logSystem = context->getSystem<log::System>();
                logSystem->print("ndi::System", msg, log::Type::Status, "ndi");
            }

            return fullpath;
        }

        System::~System()
        {
            TLRENDER_P();

            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }

#ifdef TLRENDER_NDI_DYNAMIC_LIBRARY
            pNDILib->NDIlib_destroy();
            library::closeLibrary(p.handle);
#else
            NDIlib_destroy();
#endif
        }

        std::shared_ptr<System>
        System::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<System>(new System);
            out->_init(context);
            return out;
        }

        std::shared_ptr<observer::IList<device::DeviceInfo> >
        System::observeDeviceInfo() const
        {
            return _p->deviceInfo;
        }

        void System::tick()
        {
            TLRENDER_P();
            std::vector<device::DeviceInfo> deviceInfo;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                deviceInfo = p.mutex.deviceInfo;
            }
            p.deviceInfo->setIfChanged(deviceInfo);
        }

        std::chrono::milliseconds System::getTickTime() const
        {
            return std::chrono::milliseconds(1000);
        }

    } // namespace ndi
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/System.h>

#include <tlIO/Cineon.h>
#include <tlIO/DPX.h>
#include <tlIO/PPM.h>
#include <tlIO/SGI.h>
#if defined(TLRENDER_STB)
#    include <tlIO/STB.h>
#endif
#if defined(TLRENDER_FFMPEG)
#    include <tlIO/FFmpeg.h>
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_JPEG)
#    include <tlIO/JPEG.h>
#endif // TLRENDER_JPEG
#if defined(TLRENDER_EXR)
#    include <tlIO/OpenEXR.h>
#endif // TLRENDER_EXR
#if defined(TLRENDER_NDI)
#    include <tlIO/NDI.h>
#endif // TLRENDER_NDI
#if defined(TLRENDER_PNG)
#    include <tlIO/PNG.h>
#endif // TLRENDER_PNG
#if defined(TLRENDER_TIFF)
#    include <tlIO/TIFF.h>
#endif // TLRENDER_TIFF
#if defined(TLRENDER_USD) && !defined(VULKAN_BACKEND)
#    include <tlIO/USD.h>
#endif // TLRENDER_USD
#if defined(TLRENDER_RAW)
#    include <tlIO/RAW.h>
#endif // TLRENDER_RAW

#include <tlCore/Context.h>
#include <tlCore/File.h>
#include <tlCore/String.h>

#include <iomanip>
#include <sstream>

namespace tl
{
    namespace io
    {
        struct System::Private
        {
            std::shared_ptr<Cache> cache;
            std::vector<std::string> names;
        };

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::io::System", context);
            TLRENDER_P();

            p.cache = Cache::create();

            if (auto context = _context.lock())
            {
                auto logSystem = context->getLogSystem();
                _plugins.push_back(cineon::Plugin::create(p.cache, logSystem));
                _plugins.push_back(dpx::Plugin::create(p.cache, logSystem));
                _plugins.push_back(ppm::Plugin::create(p.cache, logSystem));
                _plugins.push_back(sgi::Plugin::create(p.cache, logSystem));
#if defined(TLRENDER_STB)
                _plugins.push_back(stb::Plugin::create(p.cache, logSystem));
#endif
#if defined(TLRENDER_FFMPEG)
                _plugins.push_back(ffmpeg::Plugin::create(p.cache, logSystem));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_JPEG)
                _plugins.push_back(jpeg::Plugin::create(p.cache, logSystem));
#endif // TLRENDER_JPEG
#if defined(TLRENDER_EXR)
                _plugins.push_back(exr::Plugin::create(p.cache, logSystem));
#endif // TLRENDER_EXR
#if defined(TLRENDER_NDI)
                _plugins.push_back(ndi::Plugin::create(p.cache, logSystem));
#endif // TLRENDER_NDI
#if defined(TLRENDER_PNG)
                _plugins.push_back(png::Plugin::create(p.cache, logSystem));
#endif // TLRENDER_PNG
#if defined(TLRENDER_TIFF)
                _plugins.push_back(tiff::Plugin::create(p.cache, logSystem));
#endif // TLRENDER_TIFF
#if defined(TLRENDER_USD) && !defined(VULKAN_BACKEND)
                _plugins.push_back(usd::Plugin::create(p.cache, logSystem));
#endif // TLRENDER_USD
#if defined(TLRENDER_RAW)
                _plugins.push_back(raw::Plugin::create(p.cache, logSystem));
#endif // TLRENDER_RAW
            }

            for (const auto& plugin : _plugins)
            {
                p.names.push_back(plugin->getName());
            }
        }

        System::System() :
            _p(new Private)
        {
        }

        System::~System() {}

        std::shared_ptr<System>
        System::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = context->getSystem<System>();
            if (!out)
            {
                out = std::shared_ptr<System>(new System);
                out->_init(context);
            }
            return out;
        }

        std::shared_ptr<IPlugin> System::getPlugin(const file::Path& path) const
        {
            const std::string extension = string::toLower(path.getExtension());
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i;
                }
            }
            return nullptr;
        }

        void System::addPlugin(const std::shared_ptr<IPlugin>& plugin)
        {
            _plugins.push_back(plugin);
        }

        void System::removePlugin(const std::shared_ptr<IPlugin>& plugin)
        {
            const auto i = std::find(_plugins.begin(), _plugins.end(), plugin);
            if (i != _plugins.end())
            {
                _plugins.erase(i);
            }
        }

        const std::vector<std::string>& System::getNames() const
        {
            return _p->names;
        }

        std::set<std::string> System::getExtensions(int types) const
        {
            std::set<std::string> out;
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions(types);
                out.insert(extensions.begin(), extensions.end());
            }
            return out;
        }

        FileType System::getFileType(const std::string& extension) const
        {
            FileType out = FileType::Unknown;
            const std::string lower = string::toLower(extension);
            for (const auto& plugin : _plugins)
            {
                for (auto fileType :
                     {FileType::Movie, FileType::Sequence, FileType::Audio})
                {
                    const auto& extensions =
                        plugin->getExtensions(static_cast<int>(fileType));
                    const auto i = extensions.find(lower);
                    if (i != extensions.end())
                    {
                        out = fileType;
                        break;
                    }
                }
            }
            return out;
        }

        std::shared_ptr<IRead>
        System::read(const file::Path& path, const Options& options)
        {
            const std::string extension = string::toLower(path.getExtension());
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i->read(path, options);
                }
            }
            return nullptr;
        }

        std::shared_ptr<IRead> System::read(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const Options& options)
        {
            const std::string extension = string::toLower(path.getExtension());
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i->read(path, memory, options);
                }
            }
            return nullptr;
        }

        std::shared_ptr<IWrite> System::write(
            const file::Path& path, const Info& info, const Options& options)
        {
            const std::string extension = string::toLower(path.getExtension());
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i->write(path, info, options);
                }
            }
            return nullptr;
        }

        const std::shared_ptr<Cache>& System::getCache() const
        {
            return _p->cache;
        }
    } // namespace io
} // namespace tl

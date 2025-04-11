// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/Plugin.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace io
    {
        void IIO::_init(
            const file::Path& path, const Options& options,
            const std::shared_ptr<Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            _path = path;
            _options = options;
            _cache = cache;
            _logSystem = logSystem;
        }

        IIO::IIO() {}

        IIO::~IIO() {}

        void IRead::_init(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const Options& options, const std::shared_ptr<Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            IIO::_init(path, options, cache, logSystem);
            _memory = memory;
        }

        IRead::IRead() {}

        IRead::~IRead() {}

        std::future<VideoData>
        IRead::readVideo(const otime::RationalTime&, const Options&)
        {
            return std::future<VideoData>();
        }

        std::future<AudioData>
        IRead::readAudio(const otime::TimeRange&, const Options&)
        {
            return std::future<AudioData>();
        }

        void IWrite::_init(
            const file::Path& path, const Options& options, const Info& info,
            const std::weak_ptr<log::System>& logSystem)
        {
            IIO::_init(path, options, nullptr, logSystem);
            _info = info;
        }

        IWrite::IWrite() {}

        IWrite::~IWrite() {}

        struct IPlugin::Private
        {
            std::string name;
            std::map<std::string, FileType> extensions;
            std::shared_ptr<Cache> cache;
        };

        void IPlugin::_init(
            const std::string& name,
            const std::map<std::string, FileType>& extensions,
            const std::shared_ptr<Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            TLRENDER_P();
            _cache = cache;
            _logSystem = logSystem;
            p.name = name;
            p.extensions = extensions;
        }

        IPlugin::IPlugin() :
            _p(new Private)
        {
        }

        IPlugin::~IPlugin() {}

        const std::string& IPlugin::getName() const
        {
            return _p->name;
        }

        std::set<std::string> IPlugin::getExtensions(int types) const
        {
            std::set<std::string> out;
            for (const auto& i : _p->extensions)
            {
                if (static_cast<int>(i.second) & types)
                {
                    out.insert(i.first);
                }
            }
            return out;
        }

        bool IPlugin::_isWriteCompatible(
            const image::Info& info, const Options& options) const
        {
            return info.pixelType != image::PixelType::None &&
                   info == getWriteInfo(info, options);
        }
    } // namespace io
} // namespace tl

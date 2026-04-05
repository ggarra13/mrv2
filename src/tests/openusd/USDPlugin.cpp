// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/USDPrivate.h>

#include <tlCore/Error.h>

namespace tl
{
    namespace usd
    {
        struct Plugin::Private
        {
            int64_t id = -1;
            std::mutex mutex;
            std::shared_ptr<Render> render;
        };

        void Plugin::_init(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            IPlugin::_init(
                "USD",
                {{".usd", io::FileType::Sequence},
                 {".usda", io::FileType::Sequence},
                 {".usdc", io::FileType::Sequence},
                 {".usdz", io::FileType::Sequence}},
                cache, logSystem);
            TLRENDER_P();
            p.render = Render::create(cache, logSystem);
        }

        Plugin::Plugin() :
            _p(new Private)
        {
        }

        Plugin::~Plugin() {}

        std::shared_ptr<Plugin> Plugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(cache, logSystem);
            return out;
        }

        std::shared_ptr<io::IRead>
        Plugin::read(const file::Path& path, const io::Options& options)
        {
            TLRENDER_P();
            int64_t id = -1;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                ++(p.id);
                id = p.id;
            }
            return Read::create(
                id, p.render, path, options, _cache, _logSystem);
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            TLRENDER_P();
            int64_t id = -1;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                ++(p.id);
                id = p.id;
            }
            return Read::create(
                id, p.render, path, options, _cache, _logSystem);
        }

        image::Info
        Plugin::getWriteInfo(const image::Info&, const io::Options&) const
        {
            return image::Info();
        }

        std::shared_ptr<io::IWrite>
        Plugin::write(const file::Path&, const io::Info&, const io::Options&)
        {
            return nullptr;
        }
    } // namespace usd
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/USDPrivate.h>

#include <tlCore/Error.h>

namespace tl
{
    namespace usd
    {
        struct ReadPlugin::Private
        {
            int64_t id = -1;
            std::mutex mutex;
            std::shared_ptr<Render> render;
        };

        void ReadPlugin::_init(
            const std::shared_ptr<log::System>& logSystem)
        {
            IReadPlugin::_init(
                "USD",
                {{".usd", io::FileType::Sequence},
                 {".usda", io::FileType::Sequence},
                 {".usdc", io::FileType::Sequence},
                 {".usdz", io::FileType::Sequence}}, logSystem);
            TLRENDER_P();
            p.render = Render::create(logSystem);
        }

        ReadPlugin::ReadPlugin() :
            _p(new Private)
        {
        }

        ReadPlugin::~ReadPlugin() {}

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<io::IVideoRead>
        ReadPlugin::videoRead(const file::Path& path, const io::Options& options)
        {
            TLRENDER_P();
            int64_t id = -1;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                ++(p.id);
                id = p.id;
            }
            return Read::create(id, p.render, path, options, _logSystem.lock());
        }

        std::shared_ptr<io::IVideoRead> ReadPlugin::videoRead(
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
            return Read::create(id, p.render, path, options, _logSystem.lock());
        }
    } // namespace usd
} // namespace tl

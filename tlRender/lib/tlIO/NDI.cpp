// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024 Gonzalo Garramuño
// All rights reserved.

#include <tlIO/NDI.h>

#include <tlCore/LogSystem.h>
#include <tlCore/String.h>

#include <array>

namespace tl
{
    namespace ndi
    {
        AVSampleFormat fromAudioType(audio::DataType value)
        {
            AVSampleFormat out = AV_SAMPLE_FMT_NONE;
            switch (value)
            {
            case audio::DataType::S16:
                out = AV_SAMPLE_FMT_S16;
                break;
            case audio::DataType::S32:
                out = AV_SAMPLE_FMT_S32;
                break;
            case audio::DataType::F32:
                out = AV_SAMPLE_FMT_FLT;
                break;
            case audio::DataType::F64:
                out = AV_SAMPLE_FMT_DBL;
                break;
            default:
                break;
            }
            return out;
        }

        void Plugin::_init(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            IPlugin::_init(
                "ndi",
                {
                    {".ndi", io::FileType::Movie},
                },
                cache, logSystem);

            _logSystemWeak = logSystem;

            if (auto logSystem = _logSystemWeak.lock())
            {
                logSystem->print("tl::io::ndi::Plugin", "");
            }
        }

        Plugin::Plugin() {}

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
            return Read::create(path, options, _cache, _logSystem);
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _cache, _logSystem);
        }

        std::weak_ptr<log::System> Plugin::_logSystemWeak;
    } // namespace ndi
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024 Gonzalo Garramuño
// All rights reserved.

#include <tlIO/NDI.h>

#include <tlCore/Error.h>
#include <tlCore/LogSystem.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <array>
#include <fstream>

namespace tl
{
    namespace ndi
    {
        void ReadPlugin::_init(const std::shared_ptr<log::System>& logSystem)
        {
            IReadPlugin::_init(
                "ndi",
                {
                    {".ndi", io::FileType::Movie},
                }, logSystem);
        }

        ReadPlugin::ReadPlugin()
        {
        }

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
            return VideoRead::create(path, options, _logSystem.lock());
        }

        std::shared_ptr<io::IVideoRead> ReadPlugin::videoRead(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            return VideoRead::create(path, memory, options, _logSystem.lock());
        }

        std::shared_ptr<io::IAudioRead> ReadPlugin::audioRead(
            const file::Path& path,
            const io::Options& options)
        {
            ndi::Options opt;
            {
                std::ifstream s(path.get());

                if (s.is_open())
                {
                    nlohmann::json j;
                    s >> j;
                    opt = j;
                }
            }
            if (opt.noAudio) return nullptr;
            return AudioRead::create(path, options, _logSystem.lock());
        }

        std::shared_ptr<io::IAudioRead> ReadPlugin::audioRead(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            ndi::Options opt;
            {
                std::ifstream s(path.get());

                if (s.is_open())
                {
                    nlohmann::json j;
                    s >> j;
                    opt = j;
                }
            }
            if (opt.noAudio) return nullptr;
            return AudioRead::create(path, memory, options, _logSystem.lock());
        }

        std::string ReadPlugin::getPluginInfo(const io::Options&) const
        {
            return "NDI";
        }

    } // namespace ndi
} // namespace tl

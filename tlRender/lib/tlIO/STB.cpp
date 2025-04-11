// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/STB.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace stb
    {
        Plugin::Plugin() {}

        std::shared_ptr<Plugin> Plugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(
                "STB",
                {
                    {".tga", io::FileType::Sequence},
                    {".bmp", io::FileType::Sequence},
                    {".hdr", io::FileType::Sequence},
                    {".psd", io::FileType::Sequence},
                },
                cache, logSystem);
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

        image::Info Plugin::getWriteInfo(
            const image::Info& info, const io::Options& options) const
        {
            image::Info out;
            out.size = info.size;
            switch (info.pixelType)
            {
            case image::PixelType::L_U8:
            case image::PixelType::LA_U8:
            case image::PixelType::RGB_U8:
            case image::PixelType::RGBA_U8:
            case image::PixelType::RGB_F32:
                out.pixelType = info.pixelType;
                break;
            default:
                break;
            }
            out.layout.endian = memory::Endian::MSB;
            return out;
        }

        std::shared_ptr<io::IWrite> Plugin::write(
            const file::Path& path, const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() ||
                (!info.video.empty() &&
                 !_isWriteCompatible(info.video[0], options)))
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(path.get())
                                             .arg("Unsupported video"));
            return Write::create(path, info, options, _logSystem);
        }
    } // namespace stb
} // namespace tl

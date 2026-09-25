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

        std::shared_ptr<io::IDecode> ReadPlugin::decode(const io::Options&)
        {
            return Decode::create();
        }

        void ReadPlugin::_init(const std::shared_ptr<log::System>& logSystem)
        {
            std::map<std::string, io::FileType> exts;
            exts[".bmp"] = io::FileType::Sequence;
            exts[".hdr"] = io::FileType::Sequence;
            exts[".psd"] = io::FileType::Sequence;
            exts[".tga"] = io::FileType::Sequence;
            IReadPlugin::_init("STB", exts, logSystem);
        }

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(logSystem);
            return out;
        }

        std::string ReadPlugin::getPluginInfo(const io::Options&) const
        {
            return "STB";
        }

        image::Info WritePlugin::getInfo(
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

        void WritePlugin::_init(const std::shared_ptr<log::System>& logSystem)
        {
            std::map<std::string, io::FileType> exts;
            exts[".bmp"] = io::FileType::Sequence;
            exts[".hdr"] = io::FileType::Sequence;
            exts[".psd"] = io::FileType::Sequence;
            exts[".tga"] = io::FileType::Sequence;
            IWritePlugin::_init("STB", exts, logSystem);
        }

        std::shared_ptr<WritePlugin> WritePlugin::create(
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<WritePlugin>(new WritePlugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<io::IWrite> WritePlugin::write(
            const file::Path& path, const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() ||
                (!info.video.empty() &&
                 !_isCompatible(info.video[0], options)))
                throw std::runtime_error(string::Format("{0}: {1}")
                                         .arg(path.get())
                                         .arg("Unsupported video depth"));
            return Write::create(path, info, options, _logSystem.lock());
        }

        std::string WritePlugin::getPluginInfo(const io::Options&) const
        {
            return "STB";
        }
    } // namespace stb
} // namespace tl

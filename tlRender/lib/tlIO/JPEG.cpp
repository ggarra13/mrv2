// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/JPEG.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace jpeg
    {
        void errorFunc(j_common_ptr in)
        {
            auto error = reinterpret_cast<ErrorStruct*>(in->err);
            char message[JMSG_LENGTH_MAX] = "";
            in->err->format_message(in, message);
            error->messages.push_back(message);
            ::longjmp(error->jump, 1);
        }

        void warningFunc(j_common_ptr in, int level)
        {
            if (level > 0)
            {
                return;
            }
            auto error = reinterpret_cast<ErrorStruct*>(in->err);
            char message[JMSG_LENGTH_MAX] = "";
            in->err->format_message(in, message);
            error->messages.push_back(message);
        }

        std::shared_ptr<io::IDecode> ReadPlugin::decode(const io::Options&)
        {
            return Decode::create();
        }

        void ReadPlugin::_init(const std::shared_ptr<log::System>& logSystem)
        {
            std::map<std::string, io::FileType> exts;
            exts[".jpg"] = io::FileType::Sequence;
            exts[".jpeg"] = io::FileType::Sequence;
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
            return "JPEG";
        }

        void WritePlugin::_init(const std::shared_ptr<log::System>& logSystem)
        {
            std::map<std::string, io::FileType> exts;
            exts[".jpg"] = io::FileType::Sequence;
            exts[".jpeg"] = io::FileType::Sequence;
            IWritePlugin::_init("JPEG", exts, logSystem);
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
            return "JPEG";
        }

        image::Info WritePlugin::getInfo(
            const image::Info& info, const io::Options& options) const
        {
            image::Info out;
            out.size = info.size;
            switch (info.pixelType)
            {
            case image::PixelType::L_U8:
            case image::PixelType::RGB_U8:
                out.pixelType = info.pixelType;
                break;
            default:
                break;
            }
            out.layout.mirror.y = true;
            return out;
        }

    } // namespace jpeg
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/PNG.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace png
    {
        extern "C"
        {
            void errorFunc(png_structp in, png_const_charp msg)
            {
                auto error =
                    reinterpret_cast<ErrorStruct*>(png_get_error_ptr(in));
                error->message = msg;
                longjmp(png_jmpbuf(in), 1);
            }

            void warningFunc(png_structp in, png_const_charp msg)
            {
                auto error =
                    reinterpret_cast<ErrorStruct*>(png_get_error_ptr(in));
                error->message = msg;
            }
        }

        Plugin::Plugin() {}

        std::shared_ptr<Plugin> Plugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(
                "PNG", {{".png", io::FileType::Sequence}}, cache, logSystem);
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
            case image::PixelType::L_U16:
            case image::PixelType::LA_U8:
            case image::PixelType::LA_U16:
            case image::PixelType::RGB_U8:
            case image::PixelType::RGB_U16:
            case image::PixelType::RGBA_U8:
            case image::PixelType::RGBA_U16:
                out.pixelType = info.pixelType;
                break;
            case image::PixelType::L_F16:
            case image::PixelType::L_F32:
                out.pixelType = image::PixelType::L_U16;
                break;
            case image::PixelType::LA_F16:
            case image::PixelType::LA_F32:
                out.pixelType = image::PixelType::LA_U16;
                break;
            case image::PixelType::RGB_F16:
            case image::PixelType::RGB_F32:
                out.pixelType = image::PixelType::RGB_U16;
                break;
            case image::PixelType::YUV_420P_U12:
            case image::PixelType::YUV_422P_U12:
            case image::PixelType::YUV_444P_U12:
            case image::PixelType::YUV_420P_U16:
            case image::PixelType::YUV_422P_U16:
            case image::PixelType::YUV_444P_U16:
                out.pixelType = image::PixelType::RGB_U16;
                break;
            case image::PixelType::RGBA_F16:
            case image::PixelType::RGBA_F32:
                out.pixelType = image::PixelType::RGBA_U16;
                break;
            default:
                break;
            }
            out.layout.mirror.y = true;
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
    } // namespace png
} // namespace tl

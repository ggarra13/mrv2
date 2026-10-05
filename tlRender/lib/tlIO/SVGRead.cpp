// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/SVG.h>

#include <tlCore/FileIO.h>
#include <tlCore/StringFormat.h>

#include <lunasvg/lunasvg.h>

#include <cmath>
#include <cstring>
#include <mutex>

namespace tl
{
    namespace svg
    {
        namespace
        {
            image::Size requestedSize(const io::Options& options)
            {
                image::Size out(0, 0);
                if (const auto i = options.find("SVG/Width"); i != options.end())
                {
                    out.w = std::atoi(i->second.c_str());
                }
                if (const auto i = options.find("SVG/Height"); i != options.end())
                {
                    out.h = std::atoi(i->second.c_str());
                }
                return out;
            }

            std::unique_ptr<lunasvg::Document> load(
                const std::string& fileName,
                const file::MemoryRead* memory)
            {
                const auto path = std::filesystem::u8path(fileName);
                auto fileIO = memory ?
                    file::FileIO::create(path, *memory) :
                    file::FileIO::create(path, file::Mode::Read);
                auto out = lunasvg::Document::loadFromData(file::read(fileIO));
                if (!out)
                {
                    throw std::runtime_error(string::Format(
                        "Cannot read file: \"{0}\"").arg(fileName));
                }
                return out;
            }

            //! The size to rasterize at. A requested width or height on its
            //! own keeps the document's aspect ratio, so that scaling one
            //! dimension does not quietly stretch the drawing.
            image::Size renderSize(
                const lunasvg::Document& doc,
                const image::Size& requested,
                const std::string& fileName)
            {
                const int w = doc.width();
                const int h = doc.height();
                if (w <= 0 || h <= 0)
                {
                    throw std::runtime_error(string::Format(
                        "Cannot get the size: \"{0}\"").arg(fileName));
                }
                image::Size out(w, h);
                if (requested.w > 0 && requested.h > 0)
                {
                    out.w = requested.w;
                    out.h = requested.h;
                }
                else if (requested.w > 0)
                {
                    out.w = requested.w;
                    out.h = std::max(
                        static_cast<int>(std::lround(requested.w * h / w)), 1);
                }
                else if (requested.h > 0)
                {
                    out.w = std::max(
                        static_cast<int>(std::lround(requested.h * w / h)), 1);
                    out.h = requested.h;
                }
                return out;
            }

            image::Info imageInfo(const image::Size& size)
            {
                image::Info out(size, image::PixelType::RGBA_U8);
                out.layout.mirror.y = true;
                return out;
            }
        }

        Decode::Decode(const io::Options& options) :
            _requestedSize(requestedSize(options))
        {}

        Decode::~Decode()
        {}

        std::shared_ptr<Decode> Decode::create(const io::Options& options)
        {
            return std::shared_ptr<Decode>(new Decode(options));
        }

        io::Info Decode::getInfo(
            const std::string& fileName,
            const file::MemoryRead* memory)
        {
            auto svg = load(fileName, memory);
            io::Info out;
            const image::Size size = renderSize(*svg, _requestedSize, fileName);
            out.video.push_back(imageInfo(size));
            return out;
        }


        io::VideoData Decode::readVideo(
            const std::string& fileName,
            const file::MemoryRead* memory,
            const OTIO_NS::RationalTime& time,
            const io::Options&)
        {
            io::VideoData out;

            out.time = time;

            auto svg = load(fileName, memory);
            const image::Size size = renderSize(*svg, _requestedSize, fileName);
            auto bitmap = svg->renderToBitmap(size.w, size.h);
            if (bitmap.isNull())
            {
                throw std::runtime_error(string::Format(
                                             "Cannot render file: \"{0}\"").arg(fileName));
            }
            // lunasvg rasterizes to premultiplied ARGB32.
            bitmap.convertToRGBA();
            out.image = image::Image::create(imageInfo(size));

            const size_t rowByteCount = static_cast<size_t>(size.w) * 4;
            for (int y = 0; y < size.h; ++y)
            {
                std::memcpy(
                    out.image->getData() + y * rowByteCount,
                    bitmap.data() + y * bitmap.stride(),
                    rowByteCount);
            }

            image::Tags tags;
            io::addOtioTags(tags, fileName, time);

            return out;
        }
    }
}

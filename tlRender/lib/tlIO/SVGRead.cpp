// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/SVG.h>

#include <tlCore/FileIO.h>
#include <tlCore/StringFormat.h>

#include <FL/Fl_SVG_Image.H>

#include <cmath>
#include <cstring>
#include <mutex>

namespace tl
{
    namespace svg
    {
        static std::mutex svgMutex;

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

            std::unique_ptr<Fl_SVG_Image> load(
                const std::string& fileName,
                const file::MemoryRead* memory)
            {
                const auto path = std::filesystem::u8path(fileName);
                auto fileIO = memory ?
                    file::FileIO::create(path, *memory) :
                    file::FileIO::create(path, file::Mode::Read);
                auto out = std::make_unique<Fl_SVG_Image>(fileName.c_str(),
                                                          file::read(fileIO).c_str());
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
                const Fl_SVG_Image& doc,
                const math::Size2i& requested,
                const std::string& fileName)
            {
                const int w = doc.w();
                const int h = doc.h();
                if (w <= 0 || h <= 0)
                {
                    throw std::runtime_error(string::Format(
                        "Cannot get the size: \"{0}\"").arg(fileName));
                }
                image::Size out(w, h);
                if (requested.w > 0 && requested.h > 0)
                {
                    // pass-thru
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

            image::Info imageInfo(
                const Fl_SVG_Image& svg,
                const image::Size& size)
            {
                image::Info out(size, image::PixelType::RGBA_U8);
                out.layout.mirror.y = true;
                return out;
            }
        }

        Read::Read()
        {}

        Read::~Read()
        {
            _finish();
        }

        void Read::_init(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, cache, logSystem);
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path, const io::Options& options,
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, cache, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, cache, logSystem);
            return out;
        }

        io::Info Read::_getInfo(
            const std::string& fileName,
            const file::MemoryRead* memory)
        {
            auto svg = load(fileName, memory);
            io::Info out;
            const image::Size size = renderSize(*svg, _requestedSize, fileName);
            out.video.push_back(imageInfo(*svg, size));
            out.videoTime =
                OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                    OTIO_NS::RationalTime(_startFrame, _defaultSpeed),
                    OTIO_NS::RationalTime(_endFrame, _defaultSpeed));
            return out;
        }


        io::VideoData Read::_readVideo(
            const std::string& fileName,
            const file::MemoryRead* memory,
            const OTIO_NS::RationalTime& time,
            const io::Options&)
        {
            auto svg = load(fileName, memory);
            if (!svg || svg->fail() != 0)
            {
                throw std::runtime_error(string::Format(
                    "Cannot render file: \"{0}\"").arg(fileName));
            }

            const image::Size size = renderSize(*svg, _requestedSize, fileName);

            {
                std::lock_guard<std::mutex> lock(svgMutex);
                svg->resize(size.w, size.h);
            }

            io::VideoData out;
            out.time = time;
            out.image = image::Image::create(imageInfo(*svg, size));

            if (svg->data() && svg->data()[0])
            {
                const size_t dataSize = svg->data_w() * svg->data_h() * svg->d();
                std::memcpy(out.image->getData(), svg->data()[0], dataSize);
            }
            image::Tags tags;
            io::addOtioTags(tags, fileName, time);

            return out;
        }
    }
}

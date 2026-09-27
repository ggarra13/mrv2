// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/Cineon.h>
#include <tlIO/Normalize.h>

#include <tlCore/Locale.h>
#include <tlCore/StringFormat.h>

#include <sstream>

namespace tl
{
    namespace cineon
    {
        class File
        {
        public:
            File(const std::string& fileName,
                 const file::MemoryRead* memory)
                {
                    auto io =
                        memory ?
                        file::FileIO::create(fileName, *memory) :
                        file::FileIO::create(fileName, file::Mode::Read);
                    const auto header = cineon::read(io, _info);
                }

            io::Info getInfo()
                {
                    return _info;
                }

            io::VideoData read(const std::string& fileName,
                               const file::MemoryRead* memory,
                               const OTIO_NS::RationalTime& time,
                               const io::Options& options)
                {
                    io::VideoData out;
                    out.time = time;

                    auto i = options.find("AutoNormalize");
                    if (i != options.end())
                    {
                        _autoNormalize =
                            static_cast<bool>(std::atoi(i->second.c_str()));
                    }

                    auto io = memory ? file::FileIO::create(fileName, *memory)
                              : file::FileIO::create(fileName, file::Mode::Read);
                    io::Info info;
                    cineon::read(io, info);

                    out.image = image::Image::create(info.video[0]);
                    io->read(
                        out.image->getData(), image::getDataByteCount(info.video[0]));

                    if (_autoNormalize)
                    {
                        math::Vector4f minimum, maximum;
                        io::normalizeImage(
                            minimum, maximum, out.image, info.video[0], 0,
                            info.video[0].size.w, 0, info.video[0].size.h);
                        info.tags["Autonormalize Minimum"] = io::serialize(minimum);
                        info.tags["Autonormalize Maximum"] = io::serialize(maximum);
                    }

                    io::addOtioTags(info.tags, fileName, time);

                    out.image->setTags(info.tags);
                    return out;
                }

        protected:
            bool _autoNormalize = false;
            io::Info _info;
        };

        Decode::Decode()
        {}

        Decode::~Decode()
        {}

        std::shared_ptr<Decode> Decode::create()
        {
            return std::shared_ptr<Decode>(new Decode);
        }

        io::Info Decode::getInfo(
            const std::string& fileName, const file::MemoryRead* memory)
        {
            return File(fileName, memory).getInfo();
        }

        double Decode::getSpeed(const io::Info& info, double defaultSpeed) const
        {
            double out = defaultSpeed;

            locale::SetAndRestore saved;
            auto i = info.tags.find("Film Frame Rate");
            if (i != info.tags.end())
            {
                out = std::stof(i->second);
            }
            else
            {
                i = info.tags.find("TV Frame Rate");
                if (i != info.tags.end())
                {
                    out = std::stof(i->second);
                }
            }
            // Film/TV Rate can be corrupt.  Sanity check here.
            if (out <= 0.F)
                out = defaultSpeed;
            return out;
        }

        io::VideoData Decode::readVideo(
            const std::string& fileName, const file::MemoryRead* memory,
            const OTIO_NS::RationalTime& time, const io::Options& options)
        {
            return File(fileName, memory).read(fileName, memory, time, options);
        }

    } // namespace cineon
} // namespace tl

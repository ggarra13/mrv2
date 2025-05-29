// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/Normalize.h>
#include <tlIO/DPX.h>

#include <tlCore/Locale.h>
#include <tlCore/StringFormat.h>

#include <sstream>

namespace tl
{
    namespace dpx
    {
        void Read::_init(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, cache, logSystem);

            auto option = options.find("AutoNormalize");
            if (option != options.end())
            {
                _autoNormalize =
                    static_cast<bool>(std::atoi(option->second.c_str()));
            }
        }

        Read::Read() {}

        Read::~Read()
        {
            _finish();
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
            const std::string& fileName, const file::MemoryRead* memory)
        {
            io::Info out;
            auto io = memory ? file::FileIO::create(fileName, *memory)
                             : file::FileIO::create(fileName, file::Mode::Read);
            Transfer transfer = Transfer::User;
            const auto header = read(io, out, transfer);
            float speed = _defaultSpeed;

            locale::SetAndRestore saved;
            auto i = out.tags.find("Film Frame Rate");
            if (i != out.tags.end())
            {
                speed = std::stof(i->second);
            }
            else
            {
                i = out.tags.find("TV Frame Rate");
                if (i != out.tags.end())
                {
                    speed = std::stof(i->second);
                }
            }
            // Film/TV Rate can be corrupt.  Sanity check here.
            if (speed <= 0.F)
                speed = _defaultSpeed;
            out.videoTime =
                otime::TimeRange::range_from_start_end_time_inclusive(
                    otime::RationalTime(_startFrame, speed),
                    otime::RationalTime(_endFrame, speed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName, const file::MemoryRead* memory,
            const otime::RationalTime& time, const io::Options&)
        {
            io::VideoData out;
            out.time = time;

            auto io = memory ? file::FileIO::create(fileName, *memory)
                             : file::FileIO::create(fileName, file::Mode::Read);
            io::Info info;
            Transfer transfer = Transfer::User;
            read(io, info, transfer);

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

            _addOtioTags(info.tags, fileName, time);

            out.image->setTags(info.tags);
            return out;
        }
    } // namespace dpx
} // namespace tl

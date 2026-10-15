// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIOTest/FFmpegTest.h>

#include <tlIO/FFmpeg.h>
#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/FileIO.h>

#include <array>
#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        FFmpegTest::FFmpegTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("io_tests::FFmpegTest", context)
        {
        }

        std::shared_ptr<FFmpegTest>
        FFmpegTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<FFmpegTest>(new FFmpegTest(context));
        }

        void FFmpegTest::run()
        {
            _enums();
            _util();
            _io();
        }

        void FFmpegTest::_enums()
        {
            _enum<ffmpeg::Profile>("Profile", ffmpeg::getProfileEnums);
        }

        void FFmpegTest::_util()
        {
            {
                const AVRational r = {1, 2};
                const AVRational rs = ffmpeg::swap(r);
                TLRENDER_ASSERT(r.num == rs.den);
                TLRENDER_ASSERT(r.den == rs.num);
            }
        }

        namespace
        {
            void write(
                const std::shared_ptr<io::IWritePlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, const image::Info& imageInfo,
                const image::Tags& tags, const OTIO_NS::RationalTime& duration,
                const Options& options)
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime =
                    OTIO_NS::TimeRange(OTIO_NS::RationalTime(0.0, 24.0), duration);
                info.tags = tags;
                auto write = plugin->write(path, info, options);
                write->writeHeader();
                for (size_t i = 0; i < static_cast<size_t>(duration.value());
                     ++i)
                {
                    write->writeVideo(OTIO_NS::RationalTime(i, 24.0), image);
                }
            }

            void read(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, bool memoryIO, const image::Tags& tags,
                const OTIO_NS::RationalTime& duration, const Options& options)
            {
                std::vector<uint8_t> memoryData;
                std::vector<file::MemoryRead> memory;
                std::shared_ptr<io::IVideoRead> read;
                if (memoryIO)
                {
                    auto fileIO =
                        file::FileIO::create(path.get(), file::Mode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(
                        file::MemoryRead(nullptr, memoryData.data(), memoryData.size()));
                }
                read = plugin->videoRead(path, options);
                TLRENDER_ASSERT(read);
                const auto ioInfo = read->getInfo().get();
                if (ioInfo.video.empty())
                    return;
                for (size_t i = 0; i < static_cast<size_t>(duration.value());
                     ++i)
                {
                    const auto videoData =
                        read->readVideo(OTIO_NS::RationalTime(i, 24.0)).get();
                    TLRENDER_ASSERT(videoData.image);
                    TLRENDER_ASSERT(
                        videoData.image->getSize() == image->getSize());
                    const auto frameTags = videoData.image->getTags();
                    for (const auto& j : tags)
                    {
                        const auto k = frameTags.find(j.first);
                        TLRENDER_ASSERT(k != frameTags.end());
                        TLRENDER_ASSERT(k->second == j.second);
                    }
                }
            }

            void readError(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, bool memoryIO, const Options& options)
            {
                {
                    auto fileIO =
                        file::FileIO::create(path.get(), file::Mode::Read);
                    const size_t size = fileIO->getSize();
                    fileIO.reset();
                    file::truncateFile(path.get(), size / 2);
                }
                std::vector<uint8_t> memoryData;
                std::vector<file::MemoryRead> memory;
                if (memoryIO)
                {
                    auto fileIO =
                        file::FileIO::create(path.get(), file::Mode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(
                        file::MemoryRead(nullptr, memoryData.data(), memoryData.size()));
                }
                auto read = plugin->videoRead(path, memory, options);

                // This previously hung on truncated files (suspected cause:
                // the avIOBufferSeek() whence/return bug, since fixed). Use
                // a watchdog rather than a bare get() so a regression fails
                // the test instead of hanging the suite. Any result --
                // including empty -- is acceptable; completing is what
                // matters.
                auto future = read->readVideo(OTIO_NS::RationalTime(0.0, 24.0));
                if (future.wait_for(std::chrono::seconds(30)) !=
                    std::future_status::ready)
                {
                    throw std::runtime_error(
                        "Timeout reading a truncated file");
                }
                const auto videoData = future.get();
                // The truncated file must record a queryable error;
                // silently returning empty data is not acceptable.
                if (read->getError().empty() || 0 == read->getErrorCount())
                {
                    throw std::runtime_error(
                        "Expected an error reading a truncated file");
                }
            }
        } // namespace

        void FFmpegTest::_io()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            auto readPlugin = readSystem->getPlugin<ffmpeg::ReadPlugin>();
            auto writeSystem = _context->getSystem<WriteSystem>();
            auto writePlugin = writeSystem->getPlugin<ffmpeg::WritePlugin>();

            const image::Tags tags = {
                {"artist", "artist"},
                {"comment", "comment"},
                {"title", "title"}};
            const std::vector<std::string> fileNames = {"FFmpegTest", "大平原"};
            const std::vector<bool> memoryIOList = {false, true};
            const std::vector<image::Size> sizes = {
                image::Size(640, 480), image::Size(80, 60), image::Size(0, 0)};
            const std::vector<std::pair<std::string, std::string> > options = {
                // {"FFmpeg/HWAccel", "0"}
                // {"FFmpeg/HWAccel", "1"}
                {"FFmpeg/YUVToRGBConversion", "1"},
                {"FFmpeg/ThreadCount", "1"},
                {"FFmpeg/RequestTimeout", "1"},
                {"FFmpeg/VideoBufferSize", "1"},
                {"FFmpeg/AudioBufferSize", "1/1"},
                {"FFmpeg/WriteProfile", "None"},
                {"FFmpeg/WriteProfile", "H264"},
                {"FFmpeg/WriteProfile", "ProRes"},
                {"FFmpeg/WriteProfile", "ProRes_Proxy"},
                {"FFmpeg/WriteProfile", "ProRes_LT"},
                {"FFmpeg/WriteProfile", "ProRes_HQ"},
                {"FFmpeg/WriteProfile", "ProRes_4444"},
                {"FFmpeg/WriteProfile", "ProRes_XQ"}};

            for (const auto& fileName : fileNames)
            {
                for (const bool memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
                    {
                        for (const auto& pixelType : image::getPixelTypeEnums())
                        {
                            for (const auto& option : options)
                            {
                                Options options;
                                options[option.first] = option.second;
                                const auto imageInfo = writePlugin->getInfo(
                                    image::Info(size, pixelType));
                                if (imageInfo.isValid())
                                {
                                    file::Path path;
                                    {
                                        std::stringstream ss;
                                        ss << fileName << '_' << size << '_'
                                           << pixelType << ".mp4";
                                        _print(ss.str());
                                        path = file::Path(ss.str());
                                    }
                                    auto image =
                                        image::Image::create(imageInfo);
                                    image->zero();
                                    image->setTags(tags);
                                    const OTIO_NS::RationalTime duration(
                                        24.0, 24.0);
                                    try
                                    {
                                        write(
                                            writePlugin, image, path, imageInfo,
                                            tags, duration, options);
                                        read(
                                            readPlugin, image, path, memoryIO,
                                            tags, duration, options);
                                        readError(
                                            readPlugin, image, path, memoryIO,
                                            options);
                                    }
                                    catch (const std::exception& e)
                                    {
                                        _error(e.what());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    } // namespace io_tests
} // namespace tl

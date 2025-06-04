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
                const std::shared_ptr<io::IPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, const image::Info& imageInfo,
                const image::Tags& tags, const otime::RationalTime& duration,
                const Options& options)
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime =
                    otime::TimeRange(otime::RationalTime(0.0, 24.0), duration);
                info.tags = tags;
                auto write = plugin->write(path, info, options);
                for (size_t i = 0; i < static_cast<size_t>(duration.value());
                     ++i)
                {
                    write->writeVideo(otime::RationalTime(i, 24.0), image);
                }
            }

            void read(
                const std::shared_ptr<io::IPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, bool memoryIO, const image::Tags& tags,
                const otime::RationalTime& duration, const Options& options)
            {
                std::vector<uint8_t> memoryData;
                std::vector<file::MemoryRead> memory;
                std::shared_ptr<io::IRead> read;
                if (memoryIO)
                {
                    auto fileIO =
                        file::FileIO::create(path.get(), file::Mode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(
                        file::MemoryRead(memoryData.data(), memoryData.size()));
                    read = plugin->read(path, memory, options);
                }
                else
                {
                    read = plugin->read(path, options);
                }
                const auto ioInfo = read->getInfo().get();
                TLRENDER_ASSERT(!ioInfo.video.empty());
                for (size_t i = 0; i < static_cast<size_t>(duration.value());
                     ++i)
                {
                    const auto videoData =
                        read->readVideo(otime::RationalTime(i, 24.0)).get();
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
                for (size_t i = 0; i < static_cast<size_t>(duration.value());
                     ++i)
                {
                    const auto videoData =
                        read->readVideo(otime::RationalTime(i, 24.0)).get();
                }
            }

            void readError(
                const std::shared_ptr<io::IPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, bool memoryIO, const Options& options)
            {
                {
                    auto fileIO =
                        file::FileIO::create(path.get(), file::Mode::Read);
                    const size_t size = fileIO->getSize();
                    fileIO.reset();
                    file::truncate(path.get(), size / 2);
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
                        file::MemoryRead(memoryData.data(), memoryData.size()));
                }
                auto read = plugin->read(path, memory, options);
                //! \bug This causes the test to hang.
                // const auto videoData =
                // read->readVideo(otime::RationalTime(0.0, 24.0)).get();
            }
        } // namespace

        void FFmpegTest::_io()
        {
            auto system = _context->getSystem<System>();
            auto plugin = system->getPlugin<ffmpeg::Plugin>();

            const image::Tags tags = {
                {"artist", "artist"},
                {"comment", "comment"},
                {"title", "title"}};
            const std::vector<std::string> fileNames = {"FFmpegTest", "大平原"};
            const std::vector<bool> memoryIOList = {false, true};
            const std::vector<image::Size> sizes = {
                image::Size(640, 480), image::Size(80, 60), image::Size(0, 0)};
            const std::vector<std::pair<std::string, std::string> > options = {
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
                                const auto imageInfo = plugin->getWriteInfo(
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
                                    const otime::RationalTime duration(
                                        24.0, 24.0);
                                    try
                                    {
                                        write(
                                            plugin, image, path, imageInfo,
                                            tags, duration, options);
                                        read(
                                            plugin, image, path, memoryIO, tags,
                                            duration, options);
                                        system->getCache()->clear();
                                        readError(
                                            plugin, image, path, memoryIO,
                                            options);
                                        system->getCache()->clear();
                                    }
                                    catch (const std::exception& e)
                                    {
                                        _printError(e.what());
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

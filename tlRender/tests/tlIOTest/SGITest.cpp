// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIOTest/SGITest.h>

#include <tlIO/SGI.h>
#include <tlIO/System.h>

#include <tlCore/Assert.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        SGITest::SGITest(const std::shared_ptr<system::Context>& context) :
            ITest("io_test::SGITest", context)
        {
        }

        std::shared_ptr<SGITest>
        SGITest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<SGITest>(new SGITest(context));
        }

        void SGITest::run()
        {
            _enums();
            _io();
        }

        void SGITest::_enums()
        {
        }

        namespace
        {
            void write(
                const std::shared_ptr<io::IWritePlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, const image::Info& imageInfo,
                const image::Tags& tags,
                const io::Options& options = io::Options())
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime = OTIO_NS::TimeRange(
                    OTIO_NS::RationalTime(0.0, 24.0),
                    OTIO_NS::RationalTime(1.0, 24.0));
                info.tags = tags;
                auto write = plugin->write(path, info);
                write->writeVideo(OTIO_NS::RationalTime(0.0, 24.0), image);
            }

            void read(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, bool memoryIO, const image::Tags& tags,
                const Options options)
            {
                std::vector<uint8_t> memoryData;
                std::vector<file::MemoryRead> memory;
                if (memoryIO)
                {
                    auto fileIO =
                        file::FileIO::create(path.get(), file::Mode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(
                        file::MemoryRead(nullptr,
                                         memoryData.data(), memoryData.size()));
                }
                auto decode = plugin->decode(options);
                TLRENDER_ASSERT(decode);
                const file::MemoryRead* mem = !memory.empty() ?
                                              &memory[0] : nullptr;
                const auto ioInfo = decode->getInfo(path.get(), mem);
                TLRENDER_ASSERT(!ioInfo.video.empty());
                const auto videoData = decode->readVideo(
                    path.get(), mem,
                    OTIO_NS::RationalTime(0.0, 24.0), options);
                TLRENDER_ASSERT(videoData.image);
                TLRENDER_ASSERT(videoData.image->getSize() == image->getSize());
                //! \todo Compare image data.
                // TLRENDER_ASSERT(0 == memcmp(
                //     videoData.image->getData(),
                //     image->getData(),
                //     image->getDataByteCount()));
                const auto frameTags = videoData.image->getTags();
                for (const auto& j : tags)
                {
                    const auto k = frameTags.find(j.first);
                    TLRENDER_ASSERT(k != frameTags.end());
                    TLRENDER_ASSERT(k->second == j.second);
                }
            }

            void readError(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, bool memoryIO,
                const Options& options = io::Options())
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

                // The file has been truncated, so this is expected to fail;
                // a reader swallowed that, a decoder reports it.
                try
                {
                    auto decode = plugin->decode(options);
                    decode->readVideo(
                        path.get(),
                        !memory.empty() ? &memory[0] : nullptr,
                        OTIO_NS::RationalTime(0.0, 24.0),
                        options);
                }
                catch (const std::exception&)
                {}
            }
        } // namespace

        void SGITest::_io()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            auto readPlugin = readSystem->getPlugin<sgi::ReadPlugin>();
            auto writeSystem = _context->getSystem<WriteSystem>();
            auto writePlugin = writeSystem->getPlugin<sgi::WritePlugin>();

            const std::vector<std::string> fileNames = {"SGITest", "大平原"};
            const std::vector<bool> memoryIOList = {false, true};
            const std::vector<image::Size> sizes = {
                image::Size(16, 16), image::Size(1, 1), image::Size(0, 0)};

            const image::Tags tags = {};
            const std::vector<io::Options> optionsList =
            {
                {},
            };

            for (const auto& fileName : fileNames)
            {
                for (const bool memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
                    {
                        for (const auto& pixelType : image::getPixelTypeEnums())
                        {
                            for (const auto& options : optionsList)
                            {
                                const auto imageInfo = writePlugin->getInfo(
                                    image::Info(size, pixelType));
                                if (imageInfo.isValid())
                                {
                                    file::Path path;
                                    {
                                        std::stringstream ss;
                                        ss << fileName << '_' << size << '_'
                                           << pixelType << ".0.sgi";
                                        _print(ss.str());
                                        path = file::Path(ss.str());
                                    }
                                    auto image = image::Image::create(imageInfo);
                                    image->zero();
                                    image->setTags(tags);
                                    try
                                    {
                                        write(writePlugin,
                                              image, path, imageInfo, tags, options);
                                        read(readPlugin, image, path, memoryIO, tags, options);
                                        readError(readPlugin, image, path, memoryIO, options);
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

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIOTest/PPMTest.h>

#include <tlIO/PPM.h>
#include <tlIO/System.h>

#include <tlCore/Assert.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        PPMTest::PPMTest(const std::shared_ptr<system::Context>& context) :
            ITest("io_tests::PPMTest", context)
        {
        }

        std::shared_ptr<PPMTest>
        PPMTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<PPMTest>(new PPMTest(context));
        }

        void PPMTest::run()
        {
            _enums();
            _io();
        }

        void PPMTest::_enums()
        {
            _enum<ppm::Data>("Data", ppm::getDataEnums);
        }

        namespace
        {
            void write(
                const std::shared_ptr<io::IWritePlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, const image::Info& imageInfo,
                const Options& options)
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime = otime::TimeRange(
                    otime::RationalTime(0.0, 24.0),
                    otime::RationalTime(1.0, 24.0));
                auto write = plugin->write(path, info, options);
                write->writeVideo(otime::RationalTime(0.0, 24.0), image);
            }

            void read(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, bool memoryIO, const Options& options)
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
                        file::MemoryRead(fileIO,
                                         memoryData.data(), memoryData.size()));
                }
                auto decode = plugin->decode(options);
                TLRENDER_ASSERT(decode);
                const auto ioInfo = decode->getInfo().get();
                TLRENDER_ASSERT(!ioInfo.video.empty());
                const auto videoData =
                    decode->readVideo(
                        path.get(),
                        !memory.empty() ? &memory[0] : nullptr,
                        OTIO_NS::RationalTime(0.0, 24.0),
                        options);
                TLRENDER_ASSERT(videoData.image);
                TLRENDER_ASSERT(videoData.image->getSize() == image->getSize());
                //! \todo Compare image data.
                // TLRENDER_ASSERT(0 == memcmp(
                //     videoData.image->getData(),
                //     image->getData(),
                //     image->getDataByteCount()));
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
                        file::MemoryRead(fileIO, memoryData.data(), memoryData.size()));
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

        void PPMTest::_io()
        {
            auto system = _context->getSystem<io::ReadSystem>();
            auto plugin = system->getPlugin<ppm::ReadPlugin>();

            const std::vector<std::string> fileNames = {"PPMTest", "大平原"};
            const std::vector<bool> memoryIOList = {false, true};
            const std::vector<image::Size> sizes = {
                image::Size(16, 16), image::Size(1, 1), image::Size(0, 0)};
            const std::vector<std::pair<std::string, std::string> > options = {
                {"PPM/Data", "Binary"}, {"PPM/Data", "ASCII"}};

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
                                    image::Info(size, pixelType), options);
                                if (imageInfo.isValid())
                                {
                                    file::Path path;
                                    {
                                        std::stringstream ss;
                                        ss << fileName << '_' << size << '_'
                                           << pixelType << ".0.ppm";
                                        _print(ss.str());
                                        path = file::Path(ss.str());
                                    }
                                    auto image =
                                        image::Image::create(imageInfo);
                                    image->zero();
                                    try
                                    {
                                        write(
                                            plugin, image, path, imageInfo,
                                            options);
                                        read(
                                            plugin, image, path, memoryIO,
                                            options);
                                        readError(
                                            plugin, image, path, memoryIO,
                                            options);
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

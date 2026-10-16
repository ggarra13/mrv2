// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIOTest/DPXTest.h>

#include <tlIO/DPX.h>
#include <tlIO/System.h>

#include <tlCore/Assert.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        DPXTest::DPXTest(const std::shared_ptr<system::Context>& context) :
            ITest("io_tests::DPXTest", context)
        {
        }

        std::shared_ptr<DPXTest>
        DPXTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<DPXTest>(new DPXTest(context));
        }

        void DPXTest::run()
        {
            _enums();
            _io();
        }

        void DPXTest::_enums()
        {
            _enum<dpx::Version>("Version", dpx::getVersionEnums);
            _enum<dpx::Endian>("Endian", dpx::getEndianEnums);
            _enum<dpx::Orient>("Orient", dpx::getOrientEnums);
            _enum<dpx::Transfer>("Transfer", dpx::getTransferEnums);
            _enum<dpx::Components>("Components", dpx::getComponentsEnums);
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
                const Options& options)
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

                // \@bug: tags not equal
                // const auto frameTags = videoData.image->getTags();
                // for (const auto& j : tags)
                // {
                //     const auto k = frameTags.find(j.first);
                //     TLRENDER_ASSERT(k != frameTags.end());
                //     TLRENDER_ASSERT(k->second == j.second);
                // }
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

        void DPXTest::_io()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            auto readPlugin = readSystem->getPlugin<dpx::ReadPlugin>();
            auto writeSystem = _context->getSystem<WriteSystem>();
            auto writePlugin = writeSystem->getPlugin<dpx::WritePlugin>();

            const image::Tags tags = {
                {"Time", "1:00PM 2023/10/01"},
                {"Creator", "Charlie"},
                {"Project", "Nap Time"},
                {"Copyright", "Copyright (c) 2023 Charlie"},
                {"Source Offset", "1 2"},
                {"Source Center", "3 4"},
                {"Source Size", "5 6"},
                {"Source File", "zzz.png"},
                {"Source Time", "1:00PM 2023/10/01"},
                {"Source Input Device", "Lunch"},
                {"Source Input Serial", "0123456789"},
                {"Source Border", "7 8 9 10"},
                {"Source Pixel Aspect", "11 12"},
                {"Source Scan Size", "13 14"},
                {"Keycode", "1:2:3:4:5"},
                {"Film Format", "Vista Vision"},
                {"Film Frame", "20"},
                {"Film Sequence", "21"},
                {"Film Hold", "22"},
                {"Film Frame Rate", "24"},
                {"Film Shutter", "180"},
                {"Film Frame ID", "25"},
                {"Film Slate", "Slate"},
                {"Timecode", "01:00:00:00"},
                {"TV Interlace", "26"},
                {"TV Field", "27"},
                {"TV Video Signal", "28"},
                {"TV Sample Rate", "29 30"},
                {"TV Frame Rate", "30"},
                {"TV Time Offset", "31"},
                {"TV Gamma", "32"},
                {"TV Black Level", "33"},
                {"TV Black Gain", "34"},
                {"TV Breakpoint", "35"},
                {"TV White Level", "36"},
                {"TV Integration Times", "37"},
                {"Timecode", "38"}};
            const std::vector<std::string> fileNames = {"DPXTest", "大平原"};
            const std::vector<bool> memoryIOList = {false, true};
            const std::vector<image::Size> sizes = {
                image::Size(16, 16), image::Size(1, 1), image::Size(0, 0)};
            const std::vector<std::pair<std::string, std::string> > optionsList = {
                {"DPX/Version", "1.0"},
                {"DPX/Version", "2.0"},
                {"DPX/Endian", "Auto"},
                {"DPX/Endian", "MSB"},
                {"DPX/Endian", "LSB"}};

            for (const auto& fileName : fileNames)
            {
                for (const bool memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
                    {
                        for (const auto& pixelType : image::getPixelTypeEnums())
                        {
                            for (const auto& option : optionsList)
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
                                           << pixelType << ".0.dpx";
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

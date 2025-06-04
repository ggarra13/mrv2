// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIOTest/OpenEXRTest.h>

#include <tlIO/OpenEXR.h>
#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/FileIO.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        OpenEXRTest::OpenEXRTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("io_tests::OpenEXRTest", context)
        {
        }

        std::shared_ptr<OpenEXRTest>
        OpenEXRTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<OpenEXRTest>(new OpenEXRTest(context));
        }

        void OpenEXRTest::run()
        {
            _enums();
            _io();
        }

        void OpenEXRTest::_enums()
        {
            _enum<exr::ChannelGrouping>(
                "ChannelGrouping", exr::getChannelGroupingEnums);
            _enum<exr::Compression>("Compression", exr::getCompressionEnums);
        }

        namespace
        {
            void write(
                const std::shared_ptr<io::IPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, const image::Info& imageInfo,
                const image::Tags& tags, const Options& options)
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime = otime::TimeRange(
                    otime::RationalTime(0.0, 24.0),
                    otime::RationalTime(1.0, 24.0));
                info.tags = tags;
                auto write = plugin->write(path, info, options);
                write->writeVideo(otime::RationalTime(0.0, 24.0), image);
            }

            void read(
                const std::shared_ptr<io::IPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path, bool memoryIO, const image::Tags& tags,
                const Options& options)
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
                const auto videoData =
                    read->readVideo(otime::RationalTime(0.0, 24.0)).get();
                TLRENDER_ASSERT(videoData.image);
                TLRENDER_ASSERT(videoData.image->getSize() == image->getSize());
                //! \todo Compare image data.
                // TLRENDER_ASSERT(0 == memcmp(
                //     videoData.image->getData(),
                //     image->getData(),
                //     image->getDataByteCount()));
                const auto frameTags = videoData.image->getTags();
                for (const auto& j : frameTags)
                {
                    const auto k = tags.find(j.first);
                    if (k != tags.end())
                    {
                        TLRENDER_ASSERT(k->second == j.second);
                    }
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
                const auto videoData =
                    read->readVideo(otime::RationalTime(0.0, 24.0)).get();
            }
        } // namespace

        void OpenEXRTest::_io()
        {
            auto system = _context->getSystem<System>();
            auto plugin = system->getPlugin<exr::Plugin>();

            const image::Tags tags = {
                //{ "Name", "Name" },
                //{ "Type", "scanlineimage" },
                //{ "Version", "1" },
                //{ "Chunk Count", "1" },
                //{ "View", "View" },
                //{ "Tile", "1 2 1 1" },
                {"AdoptedNeutral", "0 1"},
                {"Altitude", "1"},
                {"Aperture", "1"},
                {"AscFramingDecisionList", "AscFramingDecisionList"},
                {"CameraCCTSetting", "1"},
                {"CameraColorBalance", "1 2"},
                {"CameraFirmwareVersion", "CameraFirmwareVersion"},
                {"CameraLabel", "CameraLabel"},
                {"CameraMake", "CameraMake"},
                {"CameraModel", "CameraModel"},
                {"CameraSerialNumber", "CameraSerialNumber"},
                {"CameraTintSetting", "1"},
                {"CameraTintSetting", "CameraTintSetting"},
                {"CapDate", "CapDate"},
                {"CaptureRate", "24 1"},
                {"Chromaticities", "0 1 2 3 4 5 6 7"},
                {"Comments", "Comments"},
                {"EffectiveFocalLength", "1"},
                {"EntrancePupilOffset", "1"},
                {"Envmap", "1"},
                {"ExpTime", "1"},
                {"Focus", "1"},
                {"FramesPerSecond", "24 1"},
                {"ImageCounter", "1"},
                {"IsoSpeed", "1"},
                {"KeyCode", "1:2:3:4:5:6:20"},
                {"Latitude", "1"},
                {"LensFirmwareVersion", "LensFirmwareVersion"},
                {"LensMake", "LensMake"},
                {"LensModel", "LensModel"},
                {"LensSerialNumber", "LensSerialNumber"},
                {"Longitude", "1"},
                {"NominalFocalLength", "1"},
                {"OriginalDataWindow", "0 1 2 3"},
                {"Owner", "Owner"},
                {"PinholeFocalLength", "1"},
                {"ReelName", "ReelName"},
                {"SensorAcquisitionRectangle", "0 1 2 3"},
                {"SensorCenterOffset", "0 1"},
                {"SensorPhotositePitch", "1"},
                {"ShutterAngle", "1"},
                {"TStop", "1"},
                {"TimeCode", "01:00:00:00"},
                {"UtcOffset", "1"},
                {"WhiteLuminance", "1"},
                {"WorldToCamera", "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15"},
                {"WorldToNDC", "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15"},
                {"XDensity", "1"},

                {"Wrapmodes", "Wrapmodes"},
                {"MultiView", "5:hello0:5:world"},
                {"DeepImageState", "1"},

                /*{ "X Density", "1" },
                { "Owner", "Owner" },
                { "Comments", "Comments" },
                { "Capture Date", "Capture Date" },
                { "UTC Offset", "1" },
                { "Longitude", "1" },
                { "Latitude", "1" },
                { "Altitude", "1" },
                { "Focus", "1" },
                { "Exposure Time", "1" },
                { "Aperture", "1" },
                { "ISO Speed", "1" },
                { "Environment Map", "1" },
                { "Keycode", "1 2 3 4 5 6 7" },
                { "Timecode", "01:02:03:04" },
                { "Wrap Modes", "Wrap Modes" }*/
            };
            const std::vector<std::string> fileNames = {
                "OpenEXRTest", "大平原"};
            const std::vector<bool> memoryIOList = {false, true};
            const std::vector<image::Size> sizes = {
                image::Size(16, 16), image::Size(1, 1), image::Size(0, 0)};
            const std::vector<std::pair<std::string, std::string> > options = {
                {"OpenEXR/ChannelGrouing", "None"},
                {"OpenEXR/ChannelGrouing", "Known"},
                {"OpenEXR/ChannelGrouing", "All"},
                {"OpenEXR/Compression", "None"},
                {"OpenEXR/Compression", "RLE"},
                {"OpenEXR/Compression", "ZIPS"},
                {"OpenEXR/Compression", "ZIP"},
                {"OpenEXR/Compression", "PIZ"},
                {"OpenEXR/Compression", "PXR24"},
                {"OpenEXR/Compression", "B44"},
                {"OpenEXR/Compression", "B44A"},
                {"OpenEXR/Compression", "DWAA"},
                {"OpenEXR/Compression", "DWAB"},
                {"OpenEXR/DWACompressionLevel", "45"},
                {"OpenEXR/DWACompressionLevel", "100"}};

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
                                           << pixelType << ".0.exr";
                                        _print(ss.str());
                                        path = file::Path(ss.str());
                                    }
                                    auto image =
                                        image::Image::create(imageInfo);
                                    image->zero();
                                    image->setTags(tags);
                                    try
                                    {
                                        write(
                                            plugin, image, path, imageInfo,
                                            tags, options);
                                        read(
                                            plugin, image, path, memoryIO, tags,
                                            options);
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

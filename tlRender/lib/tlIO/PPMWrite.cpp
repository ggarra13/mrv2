// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/PPM.h>

#include <sstream>

namespace tl
{
    namespace ppm
    {
        namespace
        {
            class File
            {
            public:
                File(
                    const std::string& fileName,
                    const std::shared_ptr<image::Image>& image, Data data)
                {
                    const auto& info = image->getInfo();
                    uint8_t ppmType = Data::ASCII == data ? 2 : 5;
                    const uint8_t channelCount =
                        image::getChannelCount(info.pixelType);
                    if (3 == channelCount)
                    {
                        ++ppmType;
                    }
                    char magic[] = "P \n";
                    magic[1] = '0' + ppmType;
                    auto io = file::FileIO::create(fileName, file::Mode::Write);
                    io->write(magic, 3);

                    std::stringstream ss;
                    ss << info.size.w << ' ' << info.size.h;
                    io->write(ss.str());
                    io->writeU8('\n');
                    const uint8_t bitDepth = image::getBitDepth(info.pixelType);
                    const uint16_t maxValue = 8 == bitDepth ? 255 : 65535;
                    ss = std::stringstream();
                    ss << maxValue;
                    io->write(ss.str());
                    io->writeU8('\n');

                    const uint8_t* p = image->getData();
                    switch (data)
                    {
                    case Data::ASCII:
                    {
                        const size_t scanlineByteCount =
                            info.size.w * channelCount * (bitDepth / 8);
                        std::vector<uint8_t> scanline(getFileScanlineByteCount(
                            info.size.w, channelCount, bitDepth));
                        for (uint16_t y = 0; y < info.size.h;
                             ++y, p += scanlineByteCount)
                        {
                            const size_t size = writeASCII(
                                p, reinterpret_cast<char*>(scanline.data()),
                                static_cast<size_t>(info.size.w) * channelCount,
                                bitDepth);
                            io->write(scanline.data(), size);
                        }
                        break;
                    }
                    case Data::Binary:
                        io->write(p, image::getDataByteCount(info));
                        break;
                    default:
                        break;
                    }
                }
            };
        } // namespace

        void Write::_init(
            const file::Path& path, const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            ISequenceWrite::_init(path, info, options, logSystem);

            auto option = options.find("PPM/Data");
            if (option != options.end())
            {
                std::stringstream ss(option->second);
                ss >> _data;
            }
        }

        Write::Write() {}

        Write::~Write() {}

        std::shared_ptr<Write> Write::create(
            const file::Path& path, const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::_writeVideo(
            const std::string& fileName, const otime::RationalTime&,
            const std::shared_ptr<image::Image>& image, const io::Options&)
        {
            const auto f = File(fileName, image, _data);
        }
    } // namespace ppm
} // namespace tl

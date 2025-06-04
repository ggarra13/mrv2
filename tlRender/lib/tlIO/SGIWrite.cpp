// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/SGI.h>

#include <sstream>

namespace tl
{
    namespace sgi
    {
        namespace
        {
            template <typename T>
            void planarDeinterleave(
                const T* in, T* out, size_t w, size_t h, size_t channels)
            {
                switch (channels)
                {
                case 1:
                    memcpy(out, in, w * h * sizeof(T));
                    break;
                case 2:
                {
                    T* out0 = out;
                    T* out1 = out + w * h;
                    for (size_t y = 0; y < h; ++y)
                    {
                        for (size_t x = 0; x < w; ++x, in += 2, ++out0, ++out1)
                        {
                            *out0 = in[0];
                            *out1 = in[1];
                        }
                    }
                    break;
                }
                case 3:
                {
                    T* out0 = out;
                    T* out1 = out + w * h;
                    T* out2 = out + w * h * 2;
                    for (size_t y = 0; y < h; ++y)
                    {
                        for (size_t x = 0; x < w;
                             ++x, in += 3, ++out0, ++out1, ++out2)
                        {
                            *out0 = in[0];
                            *out1 = in[1];
                            *out2 = in[2];
                        }
                    }
                    break;
                }
                case 4:
                {
                    T* out0 = out;
                    T* out1 = out + w * h;
                    T* out2 = out + w * h * 2;
                    T* out3 = out + w * h * 3;
                    for (size_t y = 0; y < h; ++y)
                    {
                        for (size_t x = 0; x < w;
                             ++x, in += 4, ++out0, ++out1, ++out2, ++out3)
                        {
                            *out0 = in[0];
                            *out1 = in[1];
                            *out2 = in[2];
                            *out3 = in[3];
                        }
                    }
                    break;
                }
                default:
                    break;
                }
            }

            class File
            {
            public:
                File(
                    const std::string& fileName,
                    const std::shared_ptr<image::Image>& image)
                {
                    const auto& info = image->getInfo();
                    Header header;
                    header.bytes = image::getBitDepth(info.pixelType) / 8;
                    header.dimension = 3;
                    header.width = info.size.w;
                    header.height = info.size.h;
                    header.channels = image::getChannelCount(info.pixelType);
                    header.pixelMin = 0;
                    header.pixelMax = image::getBitDepth(info.pixelType) == 8
                                          ? image::U8Range.getMax()
                                          : image::U16Range.getMax();

                    auto io = file::FileIO::create(fileName, file::Mode::Write);
                    io->setEndianConversion(
                        memory::getEndian() != memory::Endian::MSB);
                    io->writeU16(header.magic);
                    io->writeU8(header.storage);
                    io->writeU8(header.bytes);
                    io->writeU16(header.dimension);
                    io->writeU16(header.width);
                    io->writeU16(header.height);
                    io->writeU16(header.channels);
                    io->writeU32(header.pixelMin);
                    io->writeU32(header.pixelMax);
                    std::vector<uint8_t> dummy(512 - sizeof(Header), 0);
                    io->write(dummy.data(), dummy.size());
                    io->setEndianConversion(false);

                    auto tmp = image::Image::create(info);
                    planarDeinterleave(
                        image->getData(), tmp->getData(), info.size.w,
                        info.size.h, image::getChannelCount(info.pixelType));
                    io->write(tmp->getData(), tmp->getDataByteCount());
                }
            };
        } // namespace

        void Write::_init(
            const file::Path& path, const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            ISequenceWrite::_init(path, info, options, logSystem);
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
            const auto f = File(fileName, image);
        }
    } // namespace sgi
} // namespace tl

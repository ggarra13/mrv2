// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/SGI.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace sgi
    {
        namespace
        {
            template <typename T> void readRLE(const T* in, T* out, size_t size)
            {
                const T* const outEnd = out + size;
                while (out < outEnd)
                {
                    const size_t count = *in & 0x7f;
                    const bool run = !(*in & 0x80);
                    const size_t length = run ? 1 : count;
                    ++in;
                    if (run)
                    {
                        for (size_t j = 0; j < count; ++j, ++out)
                        {
                            *out = *in;
                        }
                        ++in;
                    }
                    else
                    {
                        for (size_t j = 0; j < length; ++j, ++in, ++out)
                        {
                            *out = *in;
                        }
                    }
                }
            }

            template <typename T>
            void planarInterleave(
                const T* in, T* out, size_t w, size_t h, size_t channels)
            {
                switch (channels)
                {
                case 1:
                    memcpy(out, in, w * h * sizeof(T));
                    break;
                case 2:
                {
                    const T* in0 = in;
                    const T* in1 = in + w * h;
                    for (size_t y = 0; y < h; ++y)
                    {
                        for (size_t x = 0; x < w; ++x, ++in0, ++in1, out += 2)
                        {
                            out[0] = *in0;
                            out[1] = *in1;
                        }
                    }
                    break;
                }
                case 3:
                {
                    const T* in0 = in;
                    const T* in1 = in + w * h;
                    const T* in2 = in + w * h * 2;
                    for (size_t y = 0; y < h; ++y)
                    {
                        for (size_t x = 0; x < w;
                             ++x, ++in0, ++in1, ++in2, out += 3)
                        {
                            out[0] = *in0;
                            out[1] = *in1;
                            out[2] = *in2;
                        }
                    }
                    break;
                }
                case 4:
                {
                    const T* in0 = in;
                    const T* in1 = in + w * h;
                    const T* in2 = in + w * h * 2;
                    const T* in3 = in + w * h * 3;
                    for (size_t y = 0; y < h; ++y)
                    {
                        for (size_t x = 0; x < w;
                             ++x, ++in0, ++in1, ++in2, ++in3, out += 4)
                        {
                            out[0] = *in0;
                            out[1] = *in1;
                            out[2] = *in2;
                            out[3] = *in3;
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
                    const std::string& fileName, const file::MemoryRead* memory)
                {
                    _io =
                        memory
                            ? file::FileIO::create(fileName, *memory)
                            : file::FileIO::create(fileName, file::Mode::Read);
                    _io->setEndianConversion(
                        memory::getEndian() != memory::Endian::MSB);
                    _io->readU16(&_header.magic);
                    if (_header.magic != 474)
                    {
                        throw std::runtime_error(string::Format("{0}: {1}")
                                                     .arg(fileName)
                                                     .arg("Bad magic number"));
                    }
                    _io->readU8(&_header.storage);
                    _io->readU8(&_header.bytes);
                    _io->readU16(&_header.dimension);
                    _io->readU16(&_header.width);
                    _io->readU16(&_header.height);
                    _io->readU16(&_header.channels);
                    _io->readU32(&_header.pixelMin);
                    _io->readU32(&_header.pixelMax);
                    _io->setPos(512);
                    if (_header.storage)
                    {
                        const size_t size = _header.height * _header.channels;
                        _rleOffset.resize(size);
                        _rleSize.resize(size);
                        _io->readU32(_rleOffset.data(), size);
                        _io->readU32(_rleSize.data(), size);
                    }
                    _io->setEndianConversion(false);

                    const size_t ioSize = _io->getSize();
                    const size_t ioPos = _io->getPos();
                    const size_t fileDataByteCount =
                        ioSize >= ioPos ? (ioSize - ioPos) : 0;
                    size_t dataByteCount = 0;
                    if (_header.storage)
                    {
                        for (auto i : _rleSize)
                        {
                            dataByteCount += i;
                        }
                    }
                    else
                    {
                        dataByteCount = image::getDataByteCount(_info);
                    }
                    if (dataByteCount > fileDataByteCount)
                    {
                        throw std::runtime_error(string::Format("{0}: {1}")
                                                     .arg(fileName)
                                                     .arg("Incomplete file"));
                    }

                    _info.size.w = _header.width;
                    _info.size.h = _header.height;
                    _info.pixelType = image::getIntType(
                        _header.channels, 1 == _header.bytes ? 8 : 16);
                    if (image::PixelType::None == _info.pixelType)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: {1}")
                                .arg(fileName)
                                .arg("Unsupported image type"));
                    }
                    _info.layout.endian = memory::Endian::MSB;
                }

                const image::Info& getInfo() const { return _info; }

                io::VideoData read(
                    const std::string& fileName,
                    const otime::RationalTime& time)
                {
                    io::VideoData out;
                    out.time = time;
                    out.image = image::Image::create(_info);

                    const size_t pos = _io->getPos();
                    const size_t size = _io->getSize() - pos;
                    const size_t channels =
                        image::getChannelCount(_info.pixelType);
                    const size_t bytes =
                        image::getBitDepth(_info.pixelType) / 8;
                    const size_t dataByteCount = out.image->getDataByteCount();
                    auto tmp = image::Image::create(_info);
                    if (!_header.storage)
                    {
                        _io->read(tmp->getData(), dataByteCount);
                    }
                    else
                    {
                        std::vector<uint8_t> rleData(size);
                        _io->read(rleData.data(), size);
                        switch (bytes)
                        {
                        case 1:
                        {
                            const uint8_t* inP = rleData.data();
                            uint8_t* outP = tmp->getData();
                            for (size_t c = 0; c < channels; ++c)
                            {
                                for (size_t y = 0; y < _info.size.h;
                                     ++y, outP += _info.size.w)
                                {
                                    readRLE(
                                        inP + _rleOffset[y + _info.size.h * c] -
                                            pos,
                                        outP, _info.size.w);
                                }
                            }
                            break;
                        }
                        case 2:
                        {
                            const uint16_t* inP =
                                reinterpret_cast<const uint16_t*>(
                                    rleData.data());
                            uint16_t* outP =
                                reinterpret_cast<uint16_t*>(tmp->getData());
                            for (size_t c = 0; c < channels; ++c)
                            {
                                for (size_t y = 0; y < _info.size.h;
                                     ++y, outP += _info.size.w)
                                {
                                    readRLE(
                                        inP + _rleOffset[y + _info.size.h * c] -
                                            pos,
                                        outP, _info.size.w);
                                }
                            }
                            break;
                        }
                        default:
                            break;
                        }
                    }

                    switch (bytes)
                    {
                    case 1:
                        planarInterleave<uint8_t>(
                            tmp->getData(), out.image->getData(), _info.size.w,
                            _info.size.h, channels);
                        break;
                    case 2:
                        planarInterleave<uint16_t>(
                            reinterpret_cast<const uint16_t*>(tmp->getData()),
                            reinterpret_cast<uint16_t*>(out.image->getData()),
                            _info.size.w, _info.size.h, channels);
                        break;
                    default:
                        break;
                    }

                    image::Tags tags;
                    tags["otioClipName"] = fileName;
                    {
                        std::stringstream ss;
                        ss << time;
                        tags["otioClipTime"] = ss.str();
                    }
                    out.image->setTags(tags);

                    return out;
                }

            private:
                std::shared_ptr<file::FileIO> _io;
                Header _header;
                image::Info _info;
                std::vector<uint32_t> _rleOffset;
                std::vector<uint32_t> _rleSize;
            };
        } // namespace

        void Read::_init(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, cache, logSystem);
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
            out.video.push_back(File(fileName, memory).getInfo());
            out.videoTime =
                otime::TimeRange::range_from_start_end_time_inclusive(
                    otime::RationalTime(_startFrame, _defaultSpeed),
                    otime::RationalTime(_endFrame, _defaultSpeed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName, const file::MemoryRead* memory,
            const otime::RationalTime& time, const io::Options&)
        {
            return File(fileName, memory).read(fileName, time);
        }
    } // namespace sgi
} // namespace tl

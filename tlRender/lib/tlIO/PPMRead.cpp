// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/PPM.h>

#include <tlCore/StringFormat.h>

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
                    const std::string& fileName, const file::MemoryRead* memory)
                {
                    _io =
                        memory
                            ? file::FileIO::create(fileName, *memory)
                            : file::FileIO::create(fileName, file::Mode::Read);

                    char magic[] = {0, 0, 0};
                    _io->read(magic, 2);
                    if (magic[0] != 'P')
                    {
                        throw std::runtime_error(string::Format("{0}: {1}")
                                                     .arg(fileName)
                                                     .arg("Bad magic number"));
                    }
                    switch (magic[1])
                    {
                    case '2':
                    case '3':
                    case '5':
                    case '6':
                        break;
                    default:
                    {
                        throw std::runtime_error(string::Format("{0}: {1}")
                                                     .arg(fileName)
                                                     .arg("Bad magic number"));
                    }
                    }
                    const int ppmType = magic[1] - '0';
                    _data = (2 == ppmType || 3 == ppmType) ? Data::ASCII
                                                           : Data::Binary;

                    char tmp[string::cBufferSize] = "";
                    file::readWord(_io, tmp, string::cBufferSize);
                    const int w = std::stoi(tmp);
                    file::readWord(_io, tmp, string::cBufferSize);
                    const int h = std::stoi(tmp);
                    _info.size.w = w;
                    _info.size.h = h;

                    file::readWord(_io, tmp, string::cBufferSize);
                    const int maxValue = std::stoi(tmp);
                    size_t channelCount = 0;
                    switch (ppmType)
                    {
                    case 2:
                    case 5:
                        channelCount = 1;
                        break;
                    case 3:
                    case 6:
                        channelCount = 3;
                        break;
                    default:
                        break;
                    }
                    const size_t bitDepth = maxValue < 256 ? 8 : 16;
                    _info.pixelType = image::getIntType(channelCount, bitDepth);
                    if (image::PixelType::kNone == _info.pixelType)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: {1}")
                                .arg(fileName)
                                .arg("Unsupported image type"));
                    }

                    const size_t ioSize = _io->getSize();
                    const size_t ioPos = _io->getPos();
                    const size_t fileDataByteCount =
                        ioSize >= ioPos ? (ioSize - ioPos) : 0;
                    const size_t dataByteCount = image::getDataByteCount(_info);
                    if (Data::Binary == _data &&
                        dataByteCount > fileDataByteCount)
                    {
                        throw std::runtime_error(string::Format("{0}: {1}")
                                                     .arg(fileName)
                                                     .arg("Incomplete file"));
                    }

                    _info.layout.endian = _data != Data::ASCII
                                              ? memory::Endian::MSB
                                              : memory::getEndian();
                }

                Data getData() const { return _data; }

                const image::Info& getInfo() const { return _info; }

                io::VideoData read(
                    const std::string& fileName,
                    const otime::RationalTime& time)
                {
                    io::VideoData out;
                    out.time = time;
                    out.image = image::Image::create(_info);

                    uint8_t* p = out.image->getData();
                    switch (_data)
                    {
                    case Data::ASCII:
                    {
                        const size_t channelCount =
                            image::getChannelCount(_info.pixelType);
                        const size_t bitDepth =
                            image::getBitDepth(_info.pixelType);
                        const std::size_t scanlineByteCount =
                            _info.size.w * channelCount * (bitDepth / 8);
                        for (uint16_t y = 0; y < _info.size.h;
                             ++y, p += scanlineByteCount)
                        {
                            readASCII(
                                _io, p, _info.size.w * channelCount, bitDepth);
                        }
                        break;
                    }
                    case Data::Binary:
                    {
                        _io->read(p, out.image->getDataByteCount());
                        break;
                    }
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
                Data _data = Data::First;
                image::Info _info;
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
    } // namespace ppm
} // namespace tl

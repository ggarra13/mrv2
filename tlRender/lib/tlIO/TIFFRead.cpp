// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/TIFF.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <tiffio.h>

#include <sstream>

namespace tl
{
    namespace tiff
    {
        namespace
        {
            struct Memory
            {
                const uint8_t* p = nullptr;
                const uint8_t* start = nullptr;
                const uint8_t* end = nullptr;
            };

            tmsize_t
            tiffMemoryRead(thandle_t clientData, void* data, tmsize_t size)
            {
                Memory* memory = static_cast<Memory*>(clientData);
                if (size > (memory->end - memory->p))
                {
                    return 0;
                }
                memcpy(data, memory->p, size);
                memory->p += size;
                return size;
            }

            tmsize_t
            tiffMemoryWrite(thandle_t clientData, void* data, tmsize_t size)
            {
                return 0;
            }

            toff_t
            tiffMemorySeek(thandle_t clientData, toff_t offset, int whence)
            {
                Memory* memory = static_cast<Memory*>(clientData);
                switch (whence)
                {
                case SEEK_SET:
                    memory->p = memory->start + offset;
                    break;
                case SEEK_CUR:
                    if (memory->p + offset < memory->end)
                    {
                        memory->p += offset;
                    }
                    break;
                case SEEK_END:
                    memory->p = memory->end - 1;
                    break;
                }
                return memory->p - memory->start;
            }

            int tiffMemoryClose(thandle_t clientData)
            {
                return 0;
            }

            toff_t tiffMemorySize(thandle_t clientData)
            {
                Memory* memory = static_cast<Memory*>(clientData);
                return memory->end - memory->start;
            }

            class File
            {
            public:
                File(
                    const std::string& fileName, const file::MemoryRead* memory)
                {
                    if (memory)
                    {
                        _memory.p = memory->p;
                        _memory.start = memory->p;
                        _memory.end = memory->p + memory->size;
                        _tiff.p = TIFFClientOpen(
                            fileName.c_str(), "r", &_memory, tiffMemoryRead,
                            tiffMemoryWrite, tiffMemorySeek, tiffMemoryClose,
                            tiffMemorySize, nullptr, nullptr);
                    }
                    else
                    {
#if defined(_WINDOWS)
                        _tiff.p =
                            TIFFOpenW(string::toWide(fileName).c_str(), "r");
#else  // _WINDOWS
                        _tiff.p = TIFFOpen(fileName.c_str(), "r");
#endif // _WINDOWS
                    }
                    if (!_tiff.p)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: Cannot open").arg(fileName));
                    }

                    uint32_t tiffWidth = 0;
                    uint32_t tiffHeight = 0;
                    uint16_t tiffPhotometric = 0;
                    uint16_t tiffSamples = 0;
                    uint16_t tiffSampleDepth = 0;
                    uint16_t tiffSampleFormat = 0;
                    uint16_t* tiffExtraSamples = nullptr;
                    uint16_t tiffExtraSamplesSize = 0;
                    uint16_t tiffOrient = 0;
                    uint16_t tiffCompression = 0;
                    uint16_t tiffPlanarConfig = 0;
                    TIFFGetFieldDefaulted(
                        _tiff.p, TIFFTAG_IMAGEWIDTH, &tiffWidth);
                    TIFFGetFieldDefaulted(
                        _tiff.p, TIFFTAG_IMAGELENGTH, &tiffHeight);
                    TIFFGetFieldDefaulted(
                        _tiff.p, TIFFTAG_PHOTOMETRIC, &tiffPhotometric);
                    TIFFGetFieldDefaulted(
                        _tiff.p, TIFFTAG_SAMPLESPERPIXEL, &tiffSamples);
                    TIFFGetFieldDefaulted(
                        _tiff.p, TIFFTAG_BITSPERSAMPLE, &tiffSampleDepth);
                    TIFFGetFieldDefaulted(
                        _tiff.p, TIFFTAG_SAMPLEFORMAT, &tiffSampleFormat);
                    TIFFGetFieldDefaulted(
                        _tiff.p, TIFFTAG_EXTRASAMPLES, &tiffExtraSamplesSize,
                        &tiffExtraSamples);
                    TIFFGetFieldDefaulted(
                        _tiff.p, TIFFTAG_ORIENTATION, &tiffOrient);
                    TIFFGetFieldDefaulted(
                        _tiff.p, TIFFTAG_COMPRESSION, &tiffCompression);
                    TIFFGetFieldDefaulted(
                        _tiff.p, TIFFTAG_PLANARCONFIG, &tiffPlanarConfig);
                    _planar = PLANARCONFIG_SEPARATE == tiffPlanarConfig;
                    _samples = tiffSamples;
                    _sampleDepth = tiffSampleDepth;
                    _scanlineSize =
                        tiffWidth * tiffSamples * tiffSampleDepth / 8;

                    image::PixelType pixelType = image::PixelType::kNone;
                    switch (tiffPhotometric)
                    {
                    case PHOTOMETRIC_MINISWHITE:
                    case PHOTOMETRIC_MINISBLACK:
                    case PHOTOMETRIC_RGB:
                        if (32 == tiffSampleDepth &&
                            tiffSampleFormat != SAMPLEFORMAT_IEEEFP)
                            break;
                        if (SAMPLEFORMAT_IEEEFP == tiffSampleFormat)
                        {
                            pixelType = image::getFloatType(
                                tiffSamples, tiffSampleDepth);
                        }
                        else
                        {
                            pixelType =
                                image::getIntType(tiffSamples, tiffSampleDepth);
                        }
                        break;
                    }
                    if (image::PixelType::kNone == pixelType)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: Cannot open").arg(fileName));
                    }

                    image::Info imageInfo(tiffWidth, tiffHeight, pixelType);
                    imageInfo.layout.mirror.y = true;
                    _info.video.push_back(imageInfo);

                    char* tag = 0;
                    if (TIFFGetField(_tiff.p, TIFFTAG_ARTIST, &tag))
                    {
                        if (tag)
                        {
                            _info.tags["Creator"] = tag;
                        }
                    }
                    if (TIFFGetField(_tiff.p, TIFFTAG_IMAGEDESCRIPTION, &tag))
                    {
                        if (tag)
                        {
                            _info.tags["Description"] = tag;
                        }
                    }
                    if (TIFFGetField(_tiff.p, TIFFTAG_COPYRIGHT, &tag))
                    {
                        if (tag)
                        {
                            _info.tags["Copyright"] = tag;
                        }
                    }
                    if (TIFFGetField(_tiff.p, TIFFTAG_DATETIME, &tag))
                    {
                        if (tag)
                        {
                            _info.tags["Time"] = tag;
                        }
                    }
                }

                const io::Info& getInfo() const { return _info; }

                io::VideoData read(
                    const std::string& fileName,
                    const otime::RationalTime& time)
                {
                    io::VideoData out;
                    out.time = time;
                    const auto& info = _info.video[0];
                    out.image = image::Image::create(info);

                    _info.tags["otioClipName"] = fileName;
                    {
                        std::stringstream ss;
                        ss << time;
                        _info.tags["otioClipTime"] = ss.str();
                    }
                    out.image->setTags(_info.tags);

                    if (_planar)
                    {
                        std::vector<uint8_t> scanline;
                        scanline.resize(info.size.w * _sampleDepth / 8);
                        for (size_t sample = 0; sample < _samples; ++sample)
                        {
                            uint8_t* p = out.image->getData();
                            for (uint16_t y = 0; y < info.size.h;
                                 ++y, p += _scanlineSize)
                            {
                                if (TIFFReadScanline(
                                        _tiff.p, (tdata_t*)scanline.data(), y,
                                        sample) == -1)
                                {
                                    break;
                                }
                                switch (_sampleDepth)
                                {
                                case 8:
                                {
                                    const uint8_t* inP = scanline.data();
                                    uint8_t* outP = p + sample;
                                    for (uint16_t x = 0; x < info.size.w;
                                         ++x, ++inP, outP += _samples)
                                    {
                                        *outP = *inP;
                                    }
                                    break;
                                }
                                case 16:
                                {
                                    const uint16_t* inP =
                                        reinterpret_cast<uint16_t*>(
                                            scanline.data());
                                    uint16_t* outP =
                                        reinterpret_cast<uint16_t*>(p) + sample;
                                    for (uint16_t x = 0; x < info.size.w;
                                         ++x, ++inP, outP += _samples)
                                    {
                                        *outP = *inP;
                                    }
                                    break;
                                }
                                case 32:
                                {
                                    const float* inP = reinterpret_cast<float*>(
                                        scanline.data());
                                    float* outP =
                                        reinterpret_cast<float*>(p) + sample;
                                    for (uint16_t x = 0; x < info.size.w;
                                         ++x, ++inP, outP += _samples)
                                    {
                                        *outP = *inP;
                                    }
                                    break;
                                }
                                default:
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        uint8_t* p = out.image->getData();
                        for (uint16_t y = 0; y < info.size.h;
                             ++y, p += _scanlineSize)
                        {
                            if (TIFFReadScanline(_tiff.p, (tdata_t*)p, y) == -1)
                            {
                                break;
                            }
                        }
                    }

                    return out;
                }

            private:
                struct TIFFData
                {
                    ~TIFFData()
                    {
                        if (p)
                        {
                            TIFFClose(p);
                        }
                    }
                    TIFF* p = nullptr;
                };

                TIFFData _tiff;
                Memory _memory;
                bool _planar = false;
                size_t _samples = 0;
                size_t _sampleDepth = 0;
                size_t _scanlineSize = 0;
                io::Info _info;
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
            io::Info out = File(fileName, memory).getInfo();
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
    } // namespace tiff
} // namespace tl

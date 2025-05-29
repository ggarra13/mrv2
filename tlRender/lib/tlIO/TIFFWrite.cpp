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
            class File
            {
            public:
                File(
                    const std::string& fileName,
                    const std::shared_ptr<image::Image>& image)
                {
#if defined(_WINDOWS)
                    _tiff.p = TIFFOpenW(string::toWide(fileName).c_str(), "w");
#else  // _WINDOWS
                    _tiff.p = TIFFOpen(fileName.c_str(), "w");
#endif // _WINDOWS
                    if (!_tiff.p)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: Cannot open").arg(fileName));
                    }

                    uint16_t tiffPhotometric = 0;
                    uint16_t tiffSamples = 0;
                    uint16_t tiffSampleDepth = 0;
                    uint16_t tiffSampleFormat = 0;
                    uint16_t tiffExtraSamples[] = {EXTRASAMPLE_ASSOCALPHA};
                    uint16_t tiffExtraSamplesSize = 0;
                    uint16_t tiffCompression = 0;
                    const auto& info = image->getInfo();
                    switch (image::getChannelCount(info.pixelType))
                    {
                    case 1:
                        tiffPhotometric = PHOTOMETRIC_MINISBLACK;
                        tiffSamples = 1;
                        break;
                    case 2:
                        tiffPhotometric = PHOTOMETRIC_MINISBLACK;
                        tiffSamples = 2;
                        tiffExtraSamplesSize = 1;
                        break;
                    case 3:
                        tiffPhotometric = PHOTOMETRIC_RGB;
                        tiffSamples = 3;
                        break;
                    case 4:
                        tiffPhotometric = PHOTOMETRIC_RGB;
                        tiffSamples = 4;
                        tiffExtraSamplesSize = 1;
                        break;
                    default:
                        break;
                    }
                    switch (info.pixelType)
                    {
                    case image::PixelType::L_U8:
                    case image::PixelType::LA_U8:
                    case image::PixelType::RGB_U8:
                    case image::PixelType::RGBA_U8:
                        tiffSampleDepth = 8;
                        tiffSampleFormat = SAMPLEFORMAT_UINT;
                        break;
                    case image::PixelType::L_U16:
                    case image::PixelType::LA_U16:
                    case image::PixelType::RGB_U16:
                    case image::PixelType::RGBA_U16:
                        tiffSampleDepth = 16;
                        tiffSampleFormat = SAMPLEFORMAT_UINT;
                        break;
                    case image::PixelType::L_F32:
                    case image::PixelType::LA_F32:
                    case image::PixelType::RGB_F32:
                    case image::PixelType::RGBA_F32:
                        tiffSampleDepth = 32;
                        tiffSampleFormat = SAMPLEFORMAT_IEEEFP;
                        break;
                    default:
                        break;
                    }
                    if (!tiffSamples || !tiffSampleDepth)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: Cannot open").arg(fileName));
                    }

                    tiffCompression = COMPRESSION_NONE;
                    /*switch (_p->options.compression)
                    {
                    case Compression::kNone:
                        compression = COMPRESSION_NONE;
                        break;
                    case Compression::RLE:
                        compression = COMPRESSION_PACKBITS;
                        break;
                    case Compression::LZW:
                        compression = COMPRESSION_LZW;
                        break;
                    default: break;
                    }*/
                    TIFFSetField(_tiff.p, TIFFTAG_IMAGEWIDTH, info.size.w);
                    TIFFSetField(_tiff.p, TIFFTAG_IMAGELENGTH, info.size.h);
                    TIFFSetField(_tiff.p, TIFFTAG_PHOTOMETRIC, tiffPhotometric);
                    TIFFSetField(_tiff.p, TIFFTAG_SAMPLESPERPIXEL, tiffSamples);
                    TIFFSetField(
                        _tiff.p, TIFFTAG_BITSPERSAMPLE, tiffSampleDepth);
                    TIFFSetField(
                        _tiff.p, TIFFTAG_SAMPLEFORMAT, tiffSampleFormat);
                    TIFFSetField(
                        _tiff.p, TIFFTAG_EXTRASAMPLES, tiffExtraSamplesSize,
                        tiffExtraSamples);
                    TIFFSetField(
                        _tiff.p, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
                    TIFFSetField(_tiff.p, TIFFTAG_COMPRESSION, tiffCompression);
                    TIFFSetField(
                        _tiff.p, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

                    const auto& tags = image->getTags();
                    auto i = tags.find("Creator");
                    if (i != tags.end())
                    {
                        TIFFSetField(
                            _tiff.p, TIFFTAG_ARTIST, i->second.c_str());
                    }
                    i = tags.find("Copyright");
                    if (i != tags.end())
                    {
                        TIFFSetField(
                            _tiff.p, TIFFTAG_COPYRIGHT, i->second.c_str());
                    }
                    i = tags.find("Time");
                    if (i != tags.end())
                    {
                        TIFFSetField(
                            _tiff.p, TIFFTAG_DATETIME, i->second.c_str());
                    }
                    i = tags.find("Description");
                    if (i != tags.end())
                    {
                        TIFFSetField(
                            _tiff.p, TIFFTAG_IMAGEDESCRIPTION,
                            i->second.c_str());
                    }

                    const size_t scanlineByteCount = image::getAlignedByteCount(
                        info.size.w * tiffSamples * tiffSampleDepth / 8,
                        info.layout.alignment);
                    const uint8_t* p = image->getData() +
                                       (info.size.h - 1) * scanlineByteCount;
                    for (uint16_t y = 0; y < info.size.h;
                         ++y, p -= scanlineByteCount)
                    {
                        if (TIFFWriteScanline(_tiff.p, (tdata_t*)p, y) == -1)
                        {
                            throw std::runtime_error(
                                string::Format(
                                    "{0}: Cannot write scanline: {1}")
                                    .arg(fileName)
                                    .arg(y));
                        }
                    }
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
    } // namespace tiff
} // namespace tl

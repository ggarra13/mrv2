// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/PNG.h>

#include <tlCore/Error.h>
#include <tlCore/Memory.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace png
    {
        namespace
        {
            bool pngOpen(
                FILE* f, png_structp png, png_infop* pngInfo,
                const image::Info& info)
            {
                if (setjmp(png_jmpbuf(png)))
                {
                    return false;
                }
                *pngInfo = png_create_info_struct(png);
                if (!*pngInfo)
                {
                    return false;
                }
                png_init_io(png, f);

                int colorType = 0;
                switch (info.pixelType)
                {
                case image::PixelType::L_U8:
                case image::PixelType::L_U16:
                    colorType = PNG_COLOR_TYPE_GRAY;
                    break;
                case image::PixelType::LA_U8:
                case image::PixelType::LA_U16:
                    colorType = PNG_COLOR_TYPE_GRAY_ALPHA;
                    break;
                case image::PixelType::RGB_U8:
                case image::PixelType::RGB_U16:
                    colorType = PNG_COLOR_TYPE_RGB;
                    break;
                case image::PixelType::RGBA_U8:
                case image::PixelType::RGBA_U16:
                    colorType = PNG_COLOR_TYPE_RGB_ALPHA;
                    break;
                default:
                    break;
                }

                const int bitDepth = image::getBitDepth(info.pixelType);
                png_set_IHDR(
                    png, *pngInfo, info.size.w, info.size.h, bitDepth,
                    colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                    PNG_FILTER_TYPE_DEFAULT);
                png_write_info(png, *pngInfo);

                if (bitDepth > 8 && memory::Endian::LSB == memory::getEndian())
                {
                    png_set_swap(png);
                }

                return true;
            }

            bool pngScanline(png_structp png, const uint8_t* in)
            {
                if (setjmp(png_jmpbuf(png)))
                    return false;
                png_write_row(png, reinterpret_cast<const png_byte*>(in));
                return true;
            }

            bool pngEnd(png_structp png, png_infop pngInfo)
            {
                if (setjmp(png_jmpbuf(png)))
                    return false;
                png_write_end(png, pngInfo);
                return true;
            }

            class File
            {
            public:
                File(
                    const std::string& fileName,
                    const std::shared_ptr<image::Image>& image)
                {
                    _png.p = png_create_write_struct(
                        PNG_LIBPNG_VER_STRING, &_error, errorFunc, warningFunc);
                    if (!_png.p)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: Cannot open").arg(fileName));
                    }

#if defined(_WINDOWS)
                    if (_wfopen_s(
                            &_f.p, string::toWide(fileName).c_str(), L"wb") !=
                        0)
                    {
                        std::cout << error::getLastError() << std::endl;
                        _f.p = nullptr;
                    }
#else  // _WINDOWS
                    _f.p = fopen(fileName.c_str(), "wb");
#endif // _WINDOWS
                    if (!_f.p)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: Cannot open").arg(fileName));
                    }

                    const auto& info = image->getInfo();
                    if (!pngOpen(_f.p, _png.p, &_png.info, info))
                    {
                        throw std::runtime_error(
                            string::Format("{0}: Cannot open").arg(fileName));
                    }

                    size_t scanlineByteCount = 0;
                    switch (info.pixelType)
                    {
                    case image::PixelType::L_U8:
                        scanlineByteCount = info.size.w;
                        break;
                    case image::PixelType::L_U16:
                        scanlineByteCount =
                            static_cast<size_t>(info.size.w) * 2;
                        break;
                    case image::PixelType::LA_U8:
                        scanlineByteCount =
                            static_cast<size_t>(info.size.w) * 2;
                        break;
                    case image::PixelType::LA_U16:
                        scanlineByteCount =
                            static_cast<size_t>(info.size.w) * 2 * 2;
                        break;
                    case image::PixelType::RGB_U8:
                        scanlineByteCount =
                            static_cast<size_t>(info.size.w) * 3;
                        break;
                    case image::PixelType::RGB_U16:
                        scanlineByteCount =
                            static_cast<size_t>(info.size.w) * 3 * 2;
                        break;
                    case image::PixelType::RGBA_U8:
                        scanlineByteCount =
                            static_cast<size_t>(info.size.w) * 4;
                        break;
                    case image::PixelType::RGBA_U16:
                        scanlineByteCount =
                            static_cast<size_t>(info.size.w) * 4 * 2;
                        break;
                    default:
                        break;
                    }
                    scanlineByteCount = image::getAlignedByteCount(
                        scanlineByteCount, info.layout.alignment);
                    const uint8_t* p = image->getData() +
                                       (info.size.h - 1) * scanlineByteCount;
                    for (uint16_t y = 0; y < info.size.h;
                         ++y, p -= scanlineByteCount)
                    {
                        if (!pngScanline(_png.p, p))
                        {
                            throw std::runtime_error(
                                string::Format(
                                    "{0}: Cannot write scanline: {1}")
                                    .arg(fileName)
                                    .arg(y));
                        }
                    }
                    if (!pngEnd(_png.p, _png.info))
                    {
                        throw std::runtime_error(
                            string::Format("{0}: Cannot close").arg(fileName));
                    }
                }

            private:
                struct PNGData
                {
                    ~PNGData()
                    {
                        if (p || info)
                        {
                            png_destroy_write_struct(
                                p ? &p : nullptr, info ? &info : nullptr);
                        }
                    }
                    png_structp p = nullptr;
                    png_infop info = nullptr;
                };

                struct FilePointer
                {
                    ~FilePointer()
                    {
                        if (p)
                        {
                            fclose(p);
                        }
                    }
                    FILE* p = nullptr;
                };

                PNGData _png;
                FilePointer _f;
                ErrorStruct _error;
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
    } // namespace png
} // namespace tl

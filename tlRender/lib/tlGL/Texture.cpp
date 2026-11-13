// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGL/Texture.h>

#include <tlGL/GL.h>
#include <tlGL/Init.h>

#include <tlCore/Assert.h>

#include <array>
#include <atomic>
#include <iostream>
#include <tuple>

namespace tl
{
    namespace gl
    {

        namespace
        {
            constexpr std::size_t pixelTypeCount =
                static_cast<std::size_t>(image::PixelType::Count);

            //! Build a lookup table indexed by image::PixelType. The size is
            //! checked against image::PixelType::Count at compile time so a
            //! table can never silently fall out of sync with the enum.
            template <typename... Args>
            constexpr std::array<GLenum, sizeof...(Args)> makeTable(Args... args)
            {
                return {{static_cast<GLenum>(args)...}};
            }

        } // namespace

        unsigned int getTextureFormat(image::PixelType value)
        {
            static const auto gl = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_RED, GL_RED, GL_RED, GL_RED, GL_RED,
                // LA_*
                GL_RG, GL_RG, GL_RG, GL_RG, GL_RG,
                // RGB_U8, RGB_U10 (packed, needs RGBA), RGB_U16, RGB_U32, RGB_F16, RGB_F32
                GL_RGB, GL_RGBA, GL_RGB, GL_RGB, GL_RGB, GL_RGB,
                // RGBA_*
                GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA,
                // YUV planar (uploaded as one L_* texture per plane)
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar (uploaded as L_* + LA_* textures)
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_BGRA,
                // RGB_F16_P (one single channel texture per plane)
                GL_RED);
            static const auto gles = makeTable(
                // kNone
                GL_NONE,
                // L_U8, L_U16, L_U32, L_F16, L_F32
                GL_RED, GL_NONE, GL_NONE, GL_RED, GL_RED,
                // LA_*
                GL_RG, GL_NONE, GL_NONE, GL_RG, GL_RG,
                // RGB_U8, RGB_U10, RGB_U16, RGB_U32, RGB_F16, RGB_F32
                GL_RGB, GL_RGBA, GL_NONE, GL_NONE, GL_RGB, GL_RGB,
                // RGBA_*
                GL_RGBA, GL_NONE, GL_NONE, GL_RGBA, GL_RGBA,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult (no BGRA in GLES)
                GL_NONE,
                // RGB_F16_P
                GL_RED);
            static_assert(std::tuple_size<decltype(gl)>::value == pixelTypeCount);
            static_assert(std::tuple_size<decltype(gles)>::value == pixelTypeCount);
            return (isGLES() ? gles : gl)[static_cast<std::size_t>(value)];
        }

        unsigned int getTextureInternalFormat(image::PixelType type)
        {
            static const auto gl = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_R8, GL_R16, GL_R32I, GL_R16F, GL_R32F,
                // LA_*
                GL_RG8, GL_RG16, GL_RG32I, GL_RG16F, GL_RG32F,
                // RGB_*
                GL_RGB8, GL_RGB10_A2, GL_RGB16, GL_RGB32I, GL_RGB16F, GL_RGB32F,
                // RGBA_*
                GL_RGBA8, GL_RGBA16, GL_RGBA32I, GL_RGBA16F, GL_RGBA32F,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_RGBA,
                // RGB_F16_P
                GL_R16F);
            static const auto gles = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_R8, GL_NONE, GL_NONE, GL_R16F, GL_R32F,
                // LA_*
                GL_RG8, GL_NONE, GL_NONE, GL_RG16F, GL_RG32F,
                // RGB_*
                GL_RGB8, GL_RGB10_A2, GL_NONE, GL_NONE, GL_RGB16F, GL_RGB32F,
                // RGBA_*
                GL_RGBA8, GL_NONE, GL_NONE, GL_RGBA16F, GL_RGBA32F,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_NONE,
                // RGB_F16_P
                GL_R16F);
            static_assert(std::tuple_size<decltype(gl)>::value == pixelTypeCount);
            static_assert(std::tuple_size<decltype(gles)>::value == pixelTypeCount);
            return (isGLES() ? gles : gl)[static_cast<std::size_t>(type)];
        }

        unsigned int getTextureType(image::PixelType value)
        {
            static const auto gl = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // LA_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // RGB_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_INT_10_10_10_2, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // RGBA_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_UNSIGNED_SHORT_4_4_4_4_REV,
                // RGB_F16_P
                GL_HALF_FLOAT);
            static const auto gles = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_HALF_FLOAT, GL_FLOAT,
                // LA_*
                GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_HALF_FLOAT, GL_FLOAT,
                // RGB_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_INT_2_10_10_10_REV, GL_NONE, GL_NONE, GL_HALF_FLOAT, GL_FLOAT,
                // RGBA_*
                GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_HALF_FLOAT, GL_FLOAT,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE,
                GL_NONE, GL_NONE,
                GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_NONE,
                // RGB_F16_P
                GL_HALF_FLOAT);
            static_assert(std::tuple_size<decltype(gl)>::value == pixelTypeCount);
            static_assert(std::tuple_size<decltype(gles)>::value == pixelTypeCount);
            return (isGLES() ? gles : gl)[static_cast<std::size_t>(value)];
        }

        bool TextureOptions::operator==(const TextureOptions& other) const
        {
            return filters == other.filters && pbo == other.pbo;
        }

        bool TextureOptions::operator!=(const TextureOptions& other) const
        {
            return !(*this == other);
        }

        unsigned int getTextureFilter(timeline::ImageFilter value)
        {
            const std::array<
                GLenum, static_cast<std::size_t>(timeline::ImageFilter::Count)>
                data = {GL_NEAREST, GL_LINEAR};
            return data[static_cast<std::size_t>(value)];
        }

        namespace
        {
            std::atomic<size_t> objectCount = 0;
            std::atomic<size_t> totalByteCount = 0;
        }

        size_t Texture::getTotalByteCount()
        {
            return totalByteCount;
        }

        size_t Texture::getObjectCount()
        {
            return objectCount;
        }

        struct Texture::Private
        {
            image::Info info;
            GLuint pbo = 0;
            GLuint id = 0;
            //! Whether three channel images are widened to four as they are
            //! copied in, and the buffer that holds them where there is no
            //! pixel buffer.
            bool padRgbToRgba = false;
            std::vector<uint8_t> wide;
        };

        namespace
        {
            //! The type a three channel image is uploaded as: four channels.
            //! The GPU keeps three as four anyway, and a driver given three
            //! widens them itself, on the CPU, in passes of its own on every
            //! upload -- on macOS a 6K RGB sequence played at 15 frames a
            //! second where the same frames as RGBA played at 24. Widening
            //! in the copy made anyway costs nothing more.
            image::PixelType getUploadType(image::PixelType value)
            {
                image::PixelType out = value;
                switch (value)
                {
                case image::PixelType::RGB_U8:
                    out = image::PixelType::RGBA_U8; break;
                case image::PixelType::RGB_U16:
                    out = image::PixelType::RGBA_U16; break;
                case image::PixelType::RGB_F16:
                    out = image::PixelType::RGBA_F16; break;
                case image::PixelType::RGB_F32:
                    out = image::PixelType::RGBA_F32; break;
                default:
                    break;
                }
                return out;
            }
        }

        void
        Texture::_init(const image::Info& info, const TextureOptions& options)
        {
            TLRENDER_P();
            p.info = info;
#if defined(__APPLE__)
            p.info.pixelType = getUploadType(info.pixelType);
#endif
            p.padRgbToRgba = p.info.pixelType != info.pixelType;

            if (!p.info.isValid())
            {
                throw std::runtime_error("Invalid texture");
            }
#if defined(TLRENDER_API_GL_4_1)
            if (options.pbo && 1 == p.info.layout.alignment &&
                memory::getEndian() == p.info.layout.endian)
            {
                glGenBuffers(1, &p.pbo);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, p.pbo);
                glBufferData(
                    GL_PIXEL_UNPACK_BUFFER, image::getDataByteCount(p.info),
                    NULL, GL_STREAM_DRAW);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
#endif // TLRENDER_API_GL_4_1
            glGenTextures(1, &p.id);
            glBindTexture(GL_TEXTURE_2D, p.id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                getTextureFilter(options.filters.minify));
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                getTextureFilter(options.filters.magnify));
            glTexImage2D(
                GL_TEXTURE_2D, 0, getTextureInternalFormat(p.info.pixelType),
                p.info.size.w, p.info.size.h, 0,
                getTextureFormat(p.info.pixelType),
                getTextureType(p.info.pixelType), NULL);

            ++objectCount;
            totalByteCount += image::getDataByteCount(p.info);
        }

        Texture::Texture() :
            _p(new Private)
        {
        }

        Texture::~Texture()
        {
            TLRENDER_P();
            if (p.pbo)
            {
                glDeleteBuffers(1, &p.pbo);
                p.pbo = 0;
            }
            if (p.id)
            {
                glDeleteTextures(1, &p.id);
                p.id = 0;
            }

            --objectCount;
            totalByteCount -= image::getDataByteCount(p.info);
        }

        std::shared_ptr<Texture>
        Texture::create(const image::Info& info, const TextureOptions& options)
        {
            auto out = std::shared_ptr<Texture>(new Texture);
            out->_init(info, options);
            return out;
        }

        const image::Info& Texture::getInfo() const
        {
            return _p->info;
        }

        const image::Size& Texture::getSize() const
        {
            return _p->info.size;
        }

        int Texture::getWidth() const
        {
            return _p->info.size.w;
        }

        int Texture::getHeight() const
        {
            return _p->info.size.h;
        }

        image::PixelType Texture::getPixelType() const
        {
            return _p->info.pixelType;
        }

        unsigned int Texture::getID() const
        {
            return _p->id;
        }

        void Texture::copy(const std::shared_ptr<image::Image>& data)
        {
            _copy(data->getData(), data->getInfo(), 0, 0);
        }

        void Texture::copy(const std::shared_ptr<image::Image>& data,
                           int x, int y)
        {
            _copy(data->getData(), data->getInfo(), x, y);
        }

        void Texture::copy(const uint8_t* data, const image::Info& info)
        {
            _copy(data, info, 0, 0);
        }

        namespace
        {
            //! Widen a row of three channels to four, the fourth opaque.
            void widenRow(image::PixelType type, const uint8_t* src,
                          uint8_t* dst, int w)
            {
                switch (type)
                {
                case image::PixelType::RGB_U8:
                {
                    // Whole pixels at a time: a 32 bit write per pixel is
                    // what lets this run at the speed of a copy.
                    uint32_t* d = reinterpret_cast<uint32_t*>(dst);
                    for (int i = 0; i < w; ++i, src += 3)
                    {
                        d[i] =
                            static_cast<uint32_t>(src[0]) |
                            static_cast<uint32_t>(src[1]) << 8 |
                            static_cast<uint32_t>(src[2]) << 16 |
                            0xFF000000u;
                    }
                    break;
                }
                case image::PixelType::RGB_U16:
                case image::PixelType::RGB_F16:
                {
                    const uint16_t* s = reinterpret_cast<const uint16_t*>(src);
                    uint16_t* d = reinterpret_cast<uint16_t*>(dst);
                    const uint16_t one = image::PixelType::RGB_U16 == type ? 0xFFFF : 0x3C00;
                    for (int i = 0; i < w; ++i, s += 3, d += 4)
                    {
                        d[0] = s[0];
                        d[1] = s[1];
                        d[2] = s[2];
                        d[3] = one;
                    }
                    break;
                }
                case image::PixelType::RGB_F32:
                {
                    const float* s = reinterpret_cast<const float*>(src);
                    float* d = reinterpret_cast<float*>(dst);
                    for (int i = 0; i < w; ++i, s += 3, d += 4)
                    {
                        d[0] = s[0];
                        d[1] = s[1];
                        d[2] = s[2];
                        d[3] = 1.F;
                    }
                    break;
                }
                default: break;
                }
            }
        }

        void
        Texture::_copy(const uint8_t* data,
                       const image::Info& info, int x, int y)
        {
            TLRENDER_P();

            if (p.padRgbToRgba)
            {
                // Three channels made four on the way into the pixel buffer,
                // or into a buffer of our own where there is none: one pass
                // over the image, in the copy that was made anyway.
                const int w = info.size.w;
                const int h = info.size.h;
                const size_t srcRow = h > 0 ? info.getByteCount() / h : 0;
                size_t channelBytes = 1;
                switch (info.pixelType)
                {
                case image::PixelType::RGB_U16:
                case image::PixelType::RGB_F16: channelBytes = 2; break;
                case image::PixelType::RGB_F32: channelBytes = 4; break;
                default: break;
                }
                const size_t wideRow = static_cast<size_t>(w) * 4 * channelBytes;
                const size_t wideBytes = wideRow * h;
                uint8_t* dst = nullptr;
                if (p.pbo)
                {
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, p.pbo);
                    // A new allocation before mapping, as below.
                    glBufferData(GL_PIXEL_UNPACK_BUFFER, wideBytes, NULL, GL_STREAM_DRAW);
                    dst = static_cast<uint8_t*>(glMapBufferRange(
                        GL_PIXEL_UNPACK_BUFFER,
                        0,
                        wideBytes,
                        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
                }
                else
                {
                    p.wide.resize(wideBytes);
                    dst = p.wide.data();
                }
                if (dst)
                {
                    for (int row = 0; row < h; ++row)
                    {
                        widenRow(info.pixelType, data + row * srcRow,
                                 dst + row * wideRow, w);
                    }
                    if (p.pbo)
                    {
                        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                    }
                    glBindTexture(GL_TEXTURE_2D, p.id);
                    // The widened rows are packed.
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    if (!isGLES())
                    {
                        glPixelStorei(GL_UNPACK_SWAP_BYTES, info.layout.endian != memory::getEndian());
                    }
                    glTexSubImage2D(
                        GL_TEXTURE_2D,
                        0,
                        x,
                        y,
                        w,
                        h,
                        getTextureFormat(p.info.pixelType),
                        getTextureType(p.info.pixelType),
                        p.pbo ? NULL : dst);
                }
                if (p.pbo)
                {
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                }
                return;
            }

#if defined(TLRENDER_API_GL_4_1)
            if (p.pbo)
            {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, p.pbo);

                // Orphan the buffer to prevent GPU stalls
                glBufferData(GL_PIXEL_UNPACK_BUFFER,
                             image::getDataByteCount(p.info),
                             NULL, GL_STREAM_DRAW);
                if (void* buffer =
                        glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
                {
                    memcpy(buffer, data, image::getDataByteCount(p.info));
                    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                    glBindTexture(GL_TEXTURE_2D, p.id);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
                    glPixelStorei(
                        GL_UNPACK_SWAP_BYTES,
                        info.layout.endian != memory::getEndian());
                    glTexSubImage2D(
                        GL_TEXTURE_2D, 0, x, y, info.size.w, info.size.h,
                        getTextureFormat(info.pixelType),
                        getTextureType(info.pixelType), NULL);
                }
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
            else
#endif // TLRENDER_API_GL_4_1
            {
                glBindTexture(GL_TEXTURE_2D, p.id);
                glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
#if defined(TLRENDER_API_GL_4_1)
                glPixelStorei(
                    GL_UNPACK_SWAP_BYTES,
                    info.layout.endian != memory::getEndian());
#endif // TLRENDER_API_GL_4_1
                glTexSubImage2D(
                    GL_TEXTURE_2D, 0, x, y, info.size.w, info.size.h,
                    getTextureFormat(info.pixelType),
                    getTextureType(info.pixelType), data);
            }
        }

        void Texture::copy(const uint8_t* data, const image::Info& info,
                           const int rowStride)
        {
            TLRENDER_P();
            const int channelCount = image::getChannelCount(info.pixelType);
            const int bytesPerPixel =
                channelCount * (image::getBitDepth(info.pixelType) / 8);
            const int unpackRowLength =
                bytesPerPixel > 0 ? (rowStride / bytesPerPixel) : 0;


#if defined(TLRENDER_API_GL_4_1)
            if (p.pbo)
            {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, p.pbo);
                if (void* buffer =
                        glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
                {
                    if (rowStride == 0)
                    {
                        memcpy(buffer, data, image::getDataByteCount(info));
                    }
                    else
                    {
                        for (int y = 0; y < info.size.h; ++y)
                        {
                            memcpy(
                                static_cast<uint8_t*>(buffer) + y * info.size.w * bytesPerPixel,
                                data + y * rowStride,
                                info.size.w * bytesPerPixel);
                        }
                    }
                    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                    glBindTexture(GL_TEXTURE_2D, p.id);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
                    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
                    glPixelStorei(
                        GL_UNPACK_SWAP_BYTES,
                        info.layout.endian != memory::getEndian());
                    glTexSubImage2D(
                        GL_TEXTURE_2D, 0, 0, 0, info.size.w, info.size.h,
                        getTextureFormat(info.pixelType),
                        getTextureType(info.pixelType), NULL);
                }
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
            else
#endif // TLRENDER_API_GL_4_1
            {
                glBindTexture(GL_TEXTURE_2D, p.id);
                glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, unpackRowLength);
#if defined(TLRENDER_API_GL_4_1)
                glPixelStorei(
                    GL_UNPACK_SWAP_BYTES,
                    info.layout.endian != memory::getEndian());
#endif // TLRENDER_API_GL_4_1
                glTexSubImage2D(
                    GL_TEXTURE_2D, 0, 0, 0, info.size.w, info.size.h,
                    getTextureFormat(info.pixelType),
                    getTextureType(info.pixelType), data);
            }

            // Always reset GL state to avoid side effects
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }

        void Texture::bind()
        {
            glBindTexture(GL_TEXTURE_2D, _p->id);
        }

    } // namespace gl
} // namespace tl

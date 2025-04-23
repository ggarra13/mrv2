// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlVk/Texture.h>

#include <tlVk/Vk.h>

#include <tlCore/Assert.h>

#include <array>
#include <iostream>

namespace tl
{
    namespace vk
    {
        unsigned int getTextureFormat(image::PixelType value)
        {
            return 0;

            //             const std::array<
            //                 GLenum,
            //                 static_cast<std::size_t>(image::PixelType::Count)>
            //                 data = {
            //                     GL_NONE,

            // #if defined(TLRENDER_API_GL_4_1)
            //                     GL_RED,  GL_RED,  GL_RED,  GL_RED,  GL_RED,

            //                     GL_RG,   GL_RG,   GL_RG,   GL_RG,   GL_RG,

            //                     GL_RGB,  GL_RGBA, GL_RGB,  GL_RGB,  GL_RGB,
            //                     GL_RGB,

            //                     GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA,

            //                     GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
            //                     GL_NONE,

            //                     GL_BGRA
            // #elif defined(TLRENDER_API_GLES_2)
            //                     GL_LUMINANCE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,

            //                     GL_LUMINANCE_ALPHA,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,

            //                     GL_RGB,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,

            //                     GL_RGBA,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,

            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,

            //                     GL_NONE
            // #endif // TLRENDER_API_GL_4_1
            //                 };
            //             return data[static_cast<std::size_t>(value)];
        }

        VkFormat getTextureInternalFormat(image::PixelType type)
        {
            const std::array<
                VkFormat, static_cast<std::size_t>(image::PixelType::Count)>
                data = {
                    VK_FORMAT_UNDEFINED,

                    VK_FORMAT_R8_UNORM,
                    VK_FORMAT_R16_UNORM,
                    VK_FORMAT_R32_UINT,
                    VK_FORMAT_R16_SFLOAT,
                    VK_FORMAT_R32_SFLOAT,

                    VK_FORMAT_R8G8_UNORM,
                    VK_FORMAT_R16G16_UNORM,
                    VK_FORMAT_R32G32_UINT,
                    VK_FORMAT_R16G16_SFLOAT,
                    VK_FORMAT_R32G32_SFLOAT,

                    VK_FORMAT_R8G8B8_UNORM,
                    VK_FORMAT_A2R10G10B10_UNORM_PACK32,
                    VK_FORMAT_R16G16B16_UNORM,
                    VK_FORMAT_R32G32B32_UINT,
                    VK_FORMAT_R16G16B16_SFLOAT,
                    VK_FORMAT_R32G32B32_SFLOAT,

                    VK_FORMAT_R8G8B8A8_UNORM,
                    VK_FORMAT_R16G16B16A16_UNORM,
                    VK_FORMAT_R32G32B32A32_UINT,
                    VK_FORMAT_R16G16B16A16_SFLOAT,
                    VK_FORMAT_R32G32B32A32_SFLOAT,

                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G8_B8R8_2PLANE_422_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM

                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16R16_2PLANE_420_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16R16_2PLANE_422_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM

                    VK_FORMAT_R8G8B8A8_UNORM};
            return data[static_cast<std::size_t>(type)];
        }

        unsigned int getTextureType(image::PixelType value)
        {
            return 0;
            //             const std::array<
            //                 GLenum,
            //                 static_cast<std::size_t>(image::PixelType::Count)>
            //                 data = {
            //                     GL_NONE,

            // #if defined(TLRENDER_API_GL_4_1)
            //                     GL_UNSIGNED_BYTE,
            //                     GL_UNSIGNED_SHORT,
            //                     GL_UNSIGNED_INT,
            //                     GL_HALF_FLOAT,
            //                     GL_FLOAT,

            //                     GL_UNSIGNED_BYTE,
            //                     GL_UNSIGNED_SHORT,
            //                     GL_UNSIGNED_INT,
            //                     GL_HALF_FLOAT,
            //                     GL_FLOAT,

            //                     GL_UNSIGNED_BYTE,
            //                     GL_UNSIGNED_INT_10_10_10_2,
            //                     GL_UNSIGNED_SHORT,
            //                     GL_UNSIGNED_INT,
            //                     GL_HALF_FLOAT,
            //                     GL_FLOAT,

            //                     GL_UNSIGNED_BYTE,
            //                     GL_UNSIGNED_SHORT,
            //                     GL_UNSIGNED_INT,
            //                     GL_HALF_FLOAT,
            //                     GL_FLOAT,

            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,
            //                     GL_NONE,

            //                     GL_UNSIGNED_SHORT_4_4_4_4_REV
            // #elif defined(TLRENDER_API_GLES_2)
            //                     GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_NONE,
            //                     GL_NONE,

            //                     GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_NONE,
            //                     GL_NONE,

            //                     GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_NONE,
            //                     GL_NONE, GL_NONE,

            //                     GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_NONE,
            //                     GL_NONE,

            //                     GL_NONE,          GL_NONE, GL_NONE, GL_NONE,
            //                     GL_NONE, GL_NONE,

            //                     GL_NONE
            // #endif // TLRENDER_API_GL_4_1
            //                 };
            //             return data[static_cast<std::size_t>(value)];
        }

        bool TextureOptions::operator==(const TextureOptions& other) const
        {
            return filters == other.filters && pbo == other.pbo;
        }

        bool TextureOptions::operator!=(const TextureOptions& other) const
        {
            return !(*this == other);
        }

        VkFilter getTextureFilter(timeline::ImageFilter value)
        {
            const std::array<
                VkFilter,
                static_cast<std::size_t>(timeline::ImageFilter::Count)>
                data = {VK_FILTER_NEAREST, VK_FILTER_LINEAR};
            return data[static_cast<std::size_t>(value)];
        }

        struct Texture::Private
        {
            image::Info info;
            uint32_t pbo = 0;
            uint32_t id = 0;
        };

        void
        Texture::_init(const image::Info& info, const TextureOptions& options)
        {
            TLRENDER_P();
            p.info = info;
            if (!p.info.isValid())
            {
                throw std::runtime_error("Invalid texture");
            }
            // #if defined(TLRENDER_API_GL_4_1)
            //             if (options.pbo && 1 == p.info.layout.alignment &&
            //                 memory::getEndian() == p.info.layout.endian)
            //             {
            //                 glGenBuffers(1, &p.pbo);
            //                 glBindBuffer(GL_PIXEL_UNPACK_BUFFER, p.pbo);
            //                 glBufferData(
            //                     GL_PIXEL_UNPACK_BUFFER,
            //                     image::getDataByteCount(p.info), NULL,
            //                     GL_STREAM_DRAW);
            //                 glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            //             }
            // #endif // TLRENDER_API_GL_4_1
            //             glGenTextures(1, &p.id);
            //             glBindTexture(GL_TEXTURE_2D, p.id);
            //             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
            //             GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_2D,
            //             GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            //             glTexParameteri(
            //                 GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            //                 getTextureFilter(options.filters.minify));
            //             glTexParameteri(
            //                 GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
            //                 getTextureFilter(options.filters.magnify));
            //             glTexImage2D(
            //                 GL_TEXTURE_2D, 0,
            //                 getTextureInternalFormat(p.info.pixelType),
            //                 p.info.size.w, p.info.size.h, 0,
            //                 getTextureFormat(p.info.pixelType),
            //                 getTextureType(p.info.pixelType), NULL);
        }

        Texture::Texture(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
        }

        Texture::~Texture()
        {
            TLRENDER_P();
            if (p.pbo)
            {
                // glDeleteBuffers(1, &p.pbo);
                p.pbo = 0;
            }
            if (p.id)
            {
                // glDeleteTextures(1, &p.id);
                p.id = 0;
            }
        }

        std::shared_ptr<Texture> Texture::create(
            Fl_Vk_Context& ctx, const image::Info& info,
            const TextureOptions& options)
        {
            auto out = std::shared_ptr<Texture>(new Texture(ctx));
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
            TLRENDER_P();
#if defined(TLRENDER_API_GL_4_1)
            if (p.pbo)
            {
                // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, p.pbo);
                // if (void* buffer =
                //         glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
                // {
                //     memcpy(buffer, data->getData(),
                //     data->getDataByteCount());
                //     glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                //     const auto& info = data->getInfo();
                //     glBindTexture(GL_TEXTURE_2D, p.id);
                //     glPixelStorei(GL_UNPACK_ALIGNMENT,
                //     info.layout.alignment); glPixelStorei(
                //         GL_UNPACK_SWAP_BYTES,
                //         info.layout.endian != memory::getEndian());
                //     glTexSubImage2D(
                //         GL_TEXTURE_2D, 0, 0, 0, info.size.w, info.size.h,
                //         getTextureFormat(info.pixelType),
                //         getTextureType(info.pixelType), NULL);
                // }
                // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
            else
#endif // TLRENDER_API_GL_4_1
            {
                //                 const auto& info = data->getInfo();
                //                 glBindTexture(GL_TEXTURE_2D, p.id);
                //                 glPixelStorei(GL_UNPACK_ALIGNMENT,
                //                 info.layout.alignment);
                // #if defined(TLRENDER_API_GL_4_1)
                //                 glPixelStorei(
                //                     GL_UNPACK_SWAP_BYTES,
                //                     info.layout.endian !=
                //                     memory::getEndian());
                // #endif // TLRENDER_API_GL_4_1
                //                 glTexSubImage2D(
                //                     GL_TEXTURE_2D, 0, 0, 0, info.size.w,
                //                     info.size.h,
                //                     getTextureFormat(info.pixelType),
                //                     getTextureType(info.pixelType),
                //                     data->getData());
            }
        }

        void
        Texture::copy(const std::shared_ptr<image::Image>& data, int x, int y)
        {
            TLRENDER_P();
#if defined(TLRENDER_API_GL_4_1)
            if (p.pbo)
            {
                // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, p.pbo);
                // if (void* buffer =
                //         glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
                // {
                //     memcpy(buffer, data->getData(),
                //     data->getDataByteCount());
                //     glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                //     const auto& info = data->getInfo();
                //     glBindTexture(GL_TEXTURE_2D, p.id);
                //     glPixelStorei(GL_UNPACK_ALIGNMENT,
                //     info.layout.alignment); glPixelStorei(
                //         GL_UNPACK_SWAP_BYTES,
                //         info.layout.endian != memory::getEndian());
                //     glTexSubImage2D(
                //         GL_TEXTURE_2D, 0, x, y, info.size.w, info.size.h,
                //         getTextureFormat(info.pixelType),
                //         getTextureType(info.pixelType), NULL);
                // }
                // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
            else
#endif // TLRENDER_API_GL_4_1
            {
                const auto& info = data->getInfo();
                //                 glBindTexture(GL_TEXTURE_2D, p.id);
                //                 glPixelStorei(GL_UNPACK_ALIGNMENT,
                //                 info.layout.alignment);
                // #if defined(TLRENDER_API_GL_4_1)
                //                 glPixelStorei(
                //                     GL_UNPACK_SWAP_BYTES,
                //                     info.layout.endian !=
                //                     memory::getEndian());
                // #endif // TLRENDER_API_GL_4_1
                //                 glTexSubImage2D(
                //                     GL_TEXTURE_2D, 0, x, y, info.size.w,
                //                     info.size.h,
                //                     getTextureFormat(info.pixelType),
                //                     getTextureType(info.pixelType),
                //                     data->getData());
            }
        }

        void Texture::copy(const uint8_t* data, const image::Info& info)
        {
            TLRENDER_P();
#if defined(TLRENDER_API_GL_4_1)
            if (p.pbo)
            {
                // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, p.pbo);
                // if (void* buffer =
                //         glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
                // {
                //     memcpy(buffer, data, image::getDataByteCount(info));
                //     glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                //     glBindTexture(GL_TEXTURE_2D, p.id);
                //     glPixelStorei(GL_UNPACK_ALIGNMENT,
                //     info.layout.alignment); glPixelStorei(
                //         GL_UNPACK_SWAP_BYTES,
                //         info.layout.endian != memory::getEndian());
                //     glTexSubImage2D(
                //         GL_TEXTURE_2D, 0, 0, 0, info.size.w, info.size.h,
                //         getTextureFormat(info.pixelType),
                //         getTextureType(info.pixelType), NULL);
                // }
                // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
            else
#endif // TLRENDER_API_GL_4_1
            {
                //                 glBindTexture(GL_TEXTURE_2D, p.id);
                //                 glPixelStorei(GL_UNPACK_ALIGNMENT,
                //                 info.layout.alignment);
                // #if defined(TLRENDER_API_GL_4_1)
                //                 glPixelStorei(
                //                     GL_UNPACK_SWAP_BYTES,
                //                     info.layout.endian !=
                //                     memory::getEndian());
                // #endif // TLRENDER_API_GL_4_1
                //                 glTexSubImage2D(
                //                     GL_TEXTURE_2D, 0, 0, 0, info.size.w,
                //                     info.size.h,
                //                     getTextureFormat(info.pixelType),
                //                     getTextureType(info.pixelType), data);
            }
        }

        void Texture::bind()
        {
            // glBindTexture(GL_TEXTURE_2D, _p->id);
        }
    } // namespace vk
} // namespace tl

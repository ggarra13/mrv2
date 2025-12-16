// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (C) 2025-Present Gonzalo Garramuño.
// All rights reserved.

#include "FL/Fl_Vk_Utils.H"
#include "FL/vk_enum_string_helper.h"

#include <tlTimelineVk/RenderPrivate.h>
#include <tlTimelineVk/RenderStructs.h>

#include <tlVk/Vk.h>
#include <tlVk/Mesh.h>
#include <tlVk/Util.h>

#include <tlCore/Assert.h>
#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#if defined(TLRENDER_LIBPLACEBO)
extern "C"
{
#    include <libplacebo/gpu.h>
#    include <libplacebo/shaders/colorspace.h>
#    include <libplacebo/shaders.h>
}
#endif

#include <array>
#include <cstdint>
#include <list>
#include <regex>

#define _USE_MATH_DEFINES
#include <math.h>

namespace
{

    inline uint32_t byteSwap32(uint32_t x)
    {
        return ((x >> 24) & 0x000000FF) |
            ((x >> 8)  & 0x0000FF00) |
            ((x << 8)  & 0x00FF0000) |
            ((x << 24) & 0xFF000000);
    }
        
    // Convert from R10G10B10A2 to Vulkan A2R10G10B10 (ChatGPT)
    void convert_R10G10B10A2_to_A2R10G10B10(
        // DPX buffer (R10G10B10A2) - big endian
        const uint32_t* src,
        // Vulkan-compatible buffer (A2R10G10B10)
        uint32_t* dst,
        // number of pixels
        const size_t num_pixels,
        // endian of image
        const tl::memory::Endian endian
        )
    {
        for (std::size_t i = 0; i < num_pixels; ++i)
        {
            uint32_t pixel = src[i];

            if (tl::memory::getEndian() != endian)
                pixel = byteSwap32(pixel);
                    
            // Correct extraction for A2R10G10B10 layout
            const uint32_t R = (pixel >> 22) & 0x3FF;
            const uint32_t G = (pixel >> 12) & 0x3FF;
            const uint32_t B = (pixel >>  2) & 0x3FF;
            const uint32_t A = 0x3FF;
                    
            // Vulkan expects: A (bits 30-31), B (20-29), G
            // (10-19), R (0-9)
            pixel = (A << 30) | (R << 20) | (G << 10) | (B << 0);
                        
            dst[i] = pixel;
        }
    }

    std::string
    replaceUniformSampler(const std::string& input, unsigned& bindingIndex)
    {
        std::string output = input;
        std::string target = "uniform sampler";
        size_t pos = 0;

        while ((pos = output.find(target, pos)) != std::string::npos)
        {
            std::string replacement =
                "layout(binding=" + std::to_string(bindingIndex++) + ") " +
                target;
            output.replace(pos, target.length(), replacement);
            pos += replacement.length(); // move past the inserted text
        }

        return output;
    }

#if defined(TLRENDER_LIBPLACEBO)
    VkFormat to_vk_format(pl_fmt fmt)
    {
        int size = fmt->internal_size / fmt->num_components;

        switch (fmt->num_components)
        {
        case 1:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16_SFLOAT : VK_FORMAT_R32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32_UINT
                                 : (size == 2 ? VK_FORMAT_R16_UINT
                                              : VK_FORMAT_R8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32_SINT
                                 : (size == 2 ? VK_FORMAT_R16_SINT
                                              : VK_FORMAT_R8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16_UNORM : VK_FORMAT_R8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16_SNORM : VK_FORMAT_R8_SNORM;
            }
            break;
        case 2:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16G16_SFLOAT
                                 : VK_FORMAT_R32G32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32G32_UINT
                                 : (size == 2 ? VK_FORMAT_R16G16_UINT
                                              : VK_FORMAT_R8G8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32G32_SINT
                                 : (size == 2 ? VK_FORMAT_R16G16_SINT
                                              : VK_FORMAT_R8G8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16_UNORM
                                 : VK_FORMAT_R8G8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16_SNORM
                                 : VK_FORMAT_R8G8_SNORM;
            }
            break;
        case 3:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16G16B16_SFLOAT
                                 : VK_FORMAT_R32G32B32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32_UINT
                                 : (size == 2 ? VK_FORMAT_R16G16B16_UINT
                                              : VK_FORMAT_R8G8B8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32_SINT
                                 : (size == 2 ? VK_FORMAT_R16G16B16_SINT
                                              : VK_FORMAT_R8G8B8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16_UNORM
                                 : VK_FORMAT_R8G8B8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16_SNORM
                                 : VK_FORMAT_R8G8B8_SNORM;
            }
            break;
        case 4:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16G16B16A16_SFLOAT
                                 : VK_FORMAT_R32G32B32A32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32A32_UINT
                                 : (size == 2 ? VK_FORMAT_R16G16B16A16_UINT
                                              : VK_FORMAT_R8G8B8A8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32A32_SINT
                                 : (size == 2 ? VK_FORMAT_R16G16B16A16_SINT
                                              : VK_FORMAT_R8G8B8A8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16A16_UNORM
                                 : VK_FORMAT_R8G8B8A8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16A16_SNORM
                                 : VK_FORMAT_R8G8B8A8_SNORM;
            }
            break;
        }

        return VK_FORMAT_UNDEFINED;
    }
#endif
    
} // namespace

namespace tl
{
    namespace timeline_vlk
    {
        namespace
        {
            const int pboSizeMin = 1024;
        }

        std::vector<std::shared_ptr<vlk::Texture> > getTextures(
            Fl_Vk_Context& ctx, const image::Info& info,
            const timeline::ImageFilters& imageFilters, size_t offset)
        {
            std::vector<std::shared_ptr<vlk::Texture> > out;
            vlk::TextureOptions options;
            options.filters = imageFilters;
            options.pbo =
                info.size.w >= pboSizeMin || info.size.h >= pboSizeMin;
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h / 2),
                    image::PixelType::L_U8);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                break;
            }
            case image::PixelType::YUV_422P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h),
                    image::PixelType::L_U8);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                break;
            }
            case image::PixelType::YUV_444P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                break;
            }
            case image::PixelType::YUV_420P_U10:
            case image::PixelType::YUV_420P_U12:
            case image::PixelType::YUV_420P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h / 2),
                    image::PixelType::L_U16);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                break;
            }
            case image::PixelType::YUV_422P_U10:
            case image::PixelType::YUV_422P_U12:
            case image::PixelType::YUV_422P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h),
                    image::PixelType::L_U16);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                break;
            }
            case image::PixelType::YUV_444P_U10:
            case image::PixelType::YUV_444P_U12:
            case image::PixelType::YUV_444P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                out.push_back(vlk::Texture::create(ctx, infoTmp, options));
                break;
            }
            default:
            {
                auto texture = vlk::Texture::create(ctx, info, options);
                out.push_back(texture);
                break;
            }
            }
            return out;
        }

        void copyTextures(
            const std::shared_ptr<image::Image>& image,
            const std::vector<std::shared_ptr<vlk::Texture> >& textures,
            size_t offset)
        {
            const auto& info = image->getInfo();
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
            {
                if (image->getPlaneCount() == 1)
                {
                    textures[0]->copy(image->getData(),
                                      textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    textures[1]->copy(
                        image->getData() + (w * h),
                        textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) + (w2 * h2),
                        textures[2]->getInfo());
                }
                else
                {
                    textures[0]->copy(image->getPlaneData(0),
                                      textures[0]->getInfo(),
                                      image->getLineSize(0));
                    textures[1]->copy(image->getPlaneData(1),
                                      textures[1]->getInfo(),
                                      image->getLineSize(1));
                    textures[2]->copy(image->getPlaneData(2),
                                      textures[2]->getInfo(),
                                      image->getLineSize(2));
                }
                break;
            }
            case image::PixelType::YUV_422P_U8:
            {
                if (image->getPlaneCount() == 1)
                {
                    textures[0]->copy(image->getData(),
                                      textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    textures[1]->copy(
                        image->getData() + (w * h),
                        textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) + (w2 * h),
                        textures[2]->getInfo());
                }
                else
                {
                    textures[0]->copy(image->getPlaneData(0),
                                      textures[0]->getInfo(),
                                      image->getLineSize(0));
                    textures[1]->copy(image->getPlaneData(1),
                                      textures[1]->getInfo(),
                                      image->getLineSize(1));
                    textures[2]->copy(image->getPlaneData(2),
                                      textures[2]->getInfo(),
                                      image->getLineSize(2));
                }
                break;
            }
            case image::PixelType::YUV_444P_U8:
            {
                if (3 == textures.size())
                {
                    if (image->getPlaneCount() == 1)
                    {
                        textures[0]->copy(image->getData(),
                                          textures[0]->getInfo());
                        const std::size_t w = info.size.w;
                        const std::size_t h = info.size.h;
                        textures[1]->copy(
                            image->getData() + (w * h), textures[1]->getInfo());
                        textures[2]->copy(
                            image->getData() + (w * h) + (w * h),
                            textures[2]->getInfo());
                    }
                    else
                    {
                        textures[0]->copy(image->getPlaneData(0),
                                          textures[0]->getInfo(),
                                          image->getLineSize(0));
                        textures[1]->copy(image->getPlaneData(1),
                                          textures[1]->getInfo(),
                                          image->getLineSize(1));
                        textures[2]->copy(image->getPlaneData(2),
                                          textures[2]->getInfo(),
                                          image->getLineSize(2));
                    }
                }
                break;
            }
            case image::PixelType::YUV_422P_U10:
            case image::PixelType::YUV_444P_U10:
            case image::PixelType::YUV_420P_U10:
            case image::PixelType::YUV_420P_U12:
            case image::PixelType::YUV_422P_U12:
            case image::PixelType::YUV_444P_U12:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getPlaneData(0),
                                      textures[0]->getInfo(),
                                      image->getLineSize(0));
                    textures[1]->copy(image->getPlaneData(1),
                                      textures[1]->getInfo(),
                                      image->getLineSize(1));
                    textures[2]->copy(image->getPlaneData(2),
                                      textures[2]->getInfo(),
                                      image->getLineSize(2));
                }
                break;
            }
            case image::PixelType::YUV_420P_U16:
            {
                if (3 == textures.size())
                {
                    if (image->getPlaneCount() == 1)
                    {
                        textures[0]->copy(image->getData(),
                                          textures[0]->getInfo());
                        const std::size_t w = info.size.w;
                        const std::size_t h = info.size.h;
                        const std::size_t w2 = w / 2;
                        const std::size_t h2 = h / 2;
                        textures[1]->copy(
                            image->getData() + (w * h) * 2,
                            textures[1]->getInfo());
                        textures[2]->copy(
                            image->getData() + (w * h) * 2 + (w2 * h2) * 2,
                            textures[2]->getInfo());
                    }
                    else
                    {
                        textures[0]->copy(image->getPlaneData(0),
                                          textures[0]->getInfo(),
                                          image->getLineSize(0));
                        textures[1]->copy(image->getPlaneData(1),
                                          textures[1]->getInfo(),
                                          image->getLineSize(1));
                        textures[2]->copy(image->getPlaneData(2),
                                          textures[2]->getInfo(),
                                          image->getLineSize(2));
                    }
                }
                break;
            }
            case image::PixelType::YUV_422P_U16:
            {
                if (3 == textures.size())
                {
                    if (image->getPlaneCount() == 1)
                    {
                        textures[0]->copy(image->getData(),
                                          textures[0]->getInfo());
                        const std::size_t w = info.size.w;
                        const std::size_t h = info.size.h;
                        const std::size_t w2 = w / 2;
                        textures[1]->copy(
                            image->getData() + (w * h) * 2,
                            textures[1]->getInfo());
                        textures[2]->copy(
                            image->getData() + (w * h) * 2 + (w2 * h) * 2,
                            textures[2]->getInfo());
                    }
                    else
                    {
                        textures[0]->copy(image->getPlaneData(0),
                                          textures[0]->getInfo(),
                                          image->getLineSize(0));
                        textures[1]->copy(image->getPlaneData(1),
                                          textures[1]->getInfo(),
                                          image->getLineSize(1));
                        textures[2]->copy(image->getPlaneData(2),
                                          textures[2]->getInfo(),
                                          image->getLineSize(2));
                    }
                }
                break;
            }
            case image::PixelType::YUV_444P_U16:
            {
                if (3 == textures.size())
                {
                    if (image->getPlaneCount() == 1)
                    {
                        textures[0]->copy(image->getData(), textures[0]->getInfo());
                        const std::size_t w = info.size.w;
                        const std::size_t h = info.size.h;
                        textures[1]->copy(
                            image->getData() + (w * h) * 2, textures[1]->getInfo());
                        textures[2]->copy(
                            image->getData() + (w * h) * 2 + (w * h) * 2,
                            textures[2]->getInfo());
                    }
                    else
                    {
                        textures[0]->copy(image->getPlaneData(0),
                                          textures[0]->getInfo(),
                                          image->getLineSize(0));
                        textures[1]->copy(image->getPlaneData(1),
                                          textures[1]->getInfo(),
                                          image->getLineSize(1));
                        textures[2]->copy(image->getPlaneData(2),
                                          textures[2]->getInfo(),
                                          image->getLineSize(2));
                    }
                }
                break;
            }
            case image::PixelType::RGB_U8:
            {
                if (textures[0]->getInternalFormat() ==
                    VK_FORMAT_R8G8B8A8_UNORM)
                {
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const uint8_t* src = image->getData();
                    std::vector<uint8_t> dst(w * h * 4);
                    uint8_t* out = dst.data();
                    for (std::size_t y = 0; y < h; ++y)
                    {
                        for (std::size_t x = 0; x < w; ++x)
                        {
                            *out++ = *src++;
                            *out++ = *src++;
                            *out++ = *src++;

                            *out++ = 255;
                        }
                    }
                    textures[0]->copy(dst.data(), dst.size());
                }
                else
                {
                    textures[0]->copy(image);
                }
                break;
            }
            case image::PixelType::RGB_U16:
            {
                if (textures[0]->getInternalFormat() ==
                    VK_FORMAT_R16G16B16A16_UNORM)
                {
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const uint16_t* src =
                        reinterpret_cast<uint16_t*>(image->getData());
                    std::vector<uint16_t> dst(w * h * 4);
                    uint16_t* out = dst.data();
                    for (std::size_t y = 0; y < h; ++y)
                    {
                        for (std::size_t x = 0; x < w; ++x)
                        {
                            *out++ = *src++;
                            *out++ = *src++;
                            *out++ = *src++;

                            *out++ = 65535;
                        }
                    }
                    textures[0]->copy(
                        reinterpret_cast<uint8_t*>(dst.data()),
                        dst.size() * sizeof(uint16_t));
                }
                else
                {
                    textures[0]->copy(image);
                }
                break;
            }
            case image::PixelType::RGB_F16:
            {
                if (textures[0]->getInternalFormat() ==
                    VK_FORMAT_R16G16B16A16_SFLOAT)
                {
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const uint16_t* src =
                        reinterpret_cast<uint16_t*>(image->getData());
                    std::vector<uint16_t> dst(w * h * 4);
                    uint16_t* out = dst.data();
                    for (std::size_t y = 0; y < h; ++y)
                    {
                        for (std::size_t x = 0; x < w; ++x)
                        {
                            *out++ = *src++;
                            *out++ = *src++;
                            *out++ = *src++;

                            *out++ = 0x3C00; // 1.0 in IEEE 754
                        }
                    }
                    textures[0]->copy(
                        reinterpret_cast<uint8_t*>(dst.data()),
                        dst.size() * sizeof(uint16_t));
                }
                else
                {
                    textures[0]->copy(image);
                }
                break;
            }
            case image::PixelType::RGB_U10:
            {
                const std::size_t w = info.size.w;
                const std::size_t h = info.size.h;
                const uint32_t* src =
                    reinterpret_cast<uint32_t*>(image->getData());
                std::vector<uint32_t> dst(w * h);
                uint32_t* out = dst.data();
                convert_R10G10B10A2_to_A2R10G10B10(src, out, w*h,
                                                   info.layout.endian);
                textures[0]->copy(
                    reinterpret_cast<uint8_t*>(dst.data()),
                    dst.size() * sizeof(uint32_t));
            }
            break;
            default:
                if (1 == textures.size())
                {
                    textures[0]->copy(image);
                }
                break;
            }
        }

#if defined(TLRENDER_OCIO)
        OCIOData::~OCIOData()
        {
        }

        OCIOLUTData::~OCIOLUTData()
        {
        }
#endif // TLRENDER_OCIO

#if defined(TLRENDER_LIBPLACEBO)
        LibPlaceboData::LibPlaceboData()
        {
            log = pl_log_create(PL_API_VER, NULL);
            if (!log)
            {
                throw std::runtime_error("log creation failed");
            }

            struct pl_gpu_dummy_params gpu_dummy;
            memset(&gpu_dummy, 0, sizeof(pl_gpu_dummy_params));

            gpu_dummy.glsl.version = 410;
            gpu_dummy.glsl.gles = false;
            gpu_dummy.glsl.vulkan = false;
            gpu_dummy.glsl.compute = false;

            gpu_dummy.limits.callbacks = false;
            gpu_dummy.limits.thread_safe = true;
            /* pl_buf */
            gpu_dummy.limits.max_buf_size = SIZE_MAX;
            gpu_dummy.limits.max_ubo_size = SIZE_MAX;
            gpu_dummy.limits.max_ssbo_size = SIZE_MAX;
            gpu_dummy.limits.max_vbo_size = SIZE_MAX;
            gpu_dummy.limits.max_mapped_size = SIZE_MAX;
            gpu_dummy.limits.max_buffer_texels = UINT64_MAX;
            /* pl_tex */
            gpu_dummy.limits.max_tex_1d_dim = UINT32_MAX;
            gpu_dummy.limits.max_tex_2d_dim = UINT32_MAX;
            gpu_dummy.limits.max_tex_3d_dim = UINT32_MAX;
            gpu_dummy.limits.buf_transfer = true;
            gpu_dummy.limits.align_tex_xfer_pitch = 1;
            gpu_dummy.limits.align_tex_xfer_offset = 1;

            /* pl_pass */
            gpu_dummy.limits.max_variable_comps = SIZE_MAX;
            gpu_dummy.limits.max_constants = SIZE_MAX;
            gpu_dummy.limits.max_pushc_size = SIZE_MAX;
            gpu_dummy.limits.max_dispatch[0] = UINT32_MAX;
            gpu_dummy.limits.max_dispatch[1] = UINT32_MAX;
            gpu_dummy.limits.max_dispatch[2] = UINT32_MAX;
            gpu_dummy.limits.fragment_queues = 0;
            gpu_dummy.limits.compute_queues = 0;

            gpu = pl_gpu_dummy_create(log, &gpu_dummy);
            if (!gpu)
            {
                throw std::runtime_error("pl_gpu_dummy_create failed!");
            }

            pl_shader_params shader_params;
            memset(&shader_params, 0, sizeof(pl_shader_params));
                
            shader_params.id = 1;
            shader_params.gpu = gpu;
            shader_params.dynamic_constants = false;

            shader = pl_shader_alloc(log, &shader_params);
            if (!shader)
            {
                throw std::runtime_error("pl_shader_alloc failed!");
            }
                
            state = nullptr;
            res = nullptr;
        }

        LibPlaceboData::~LibPlaceboData()
        {
            pl_shader_free(&shader);
            res = nullptr;
            if (state) {
                pl_shader_obj_destroy(&state);
                state = nullptr;
            }
            pl_gpu_dummy_destroy(&gpu);
            pl_log_destroy(&log);
        }
#endif // TLRENDER_LIBPLACEBO

        void Render::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<TextureCache>& textureCache)
        {
            IRender::_init(context);
            TLRENDER_P();

            p.textureCache = textureCache;
            if (!p.textureCache)
            {
                p.textureCache = std::make_shared<TextureCache>();
            }

            p.glyphTextureAtlas = vlk::TextureAtlas::create(
                ctx, 1, 4096, image::PixelType::L_U8,
                timeline::ImageFilter::Linear);

            p.logTimer = std::chrono::steady_clock::now();
        }

        Render::Render(Fl_Vk_Context& context) :
            ctx(context),
            _p(new Private)
        {
            TLRENDER_P();
            
            bool valid_colorspace = false;
            
            switch (ctx.colorSpace)
            {
            case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
            case VK_COLOR_SPACE_HDR10_ST2084_EXT:
            case VK_COLOR_SPACE_HDR10_HLG_EXT:
            case VK_COLOR_SPACE_DOLBYVISION_EXT:
                valid_colorspace = true;
                break;
            default:
                break;
            }

            if (valid_colorspace)
            {
                p.hdrMonitorFound = true;
            }

            for (int i = 0; i < vlk::MAX_FRAMES_IN_FLIGHT; ++i)
            {
                p.garbage[i].pipelines.reserve(20);
                p.garbage[i].bindingSets.reserve(30);
            }
        }

        void Render::wait_queue()
        {
            VkQueue queue = ctx.queue();
            
            std::lock_guard<std::mutex> lock(ctx.queue_mutex());
            vkQueueWaitIdle(queue);
        }

        void Render::wait_device()
        {
            VkDevice device = ctx.device;
            vkDeviceWaitIdle(device);
        }
        
        Render::~Render()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;
            
            for (auto& [_, pipeline] : p.pipelines)
            {
                vkDestroyPipeline(device, pipeline.second, nullptr);
            }

            for (auto& [_, pipelineLayout] : p.pipelineLayouts)
            {
                vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            }
            for (auto& g : p.garbage)
            {
                for (auto& pipeline : g.pipelines)
                {
                    vkDestroyPipeline(device, pipeline, nullptr);
                }
                // Destroy old pipelineLayouts that are no longer used.
                for (auto& pipelineLayout : g.pipelineLayouts)
                {
                    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
                }
            }
        }

        std::shared_ptr<Render> Render::create(
            Fl_Vk_Context& vulkanContext,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<TextureCache>& textureCache)
        {
            auto out = std::shared_ptr<Render>(new Render(vulkanContext));
            out->_init(context, textureCache);
            return out;
        }

        const std::shared_ptr<TextureCache>& Render::getTextureCache() const
        {
            return _p->textureCache;
        }

        void Render::begin(
            VkCommandBuffer& cmd, std::shared_ptr<vlk::OffscreenBuffer> fbo,
            const uint32_t frameIndex, const math::Size2i& renderSize,
            const timeline::RenderOptions& renderOptions)
        {
            TLRENDER_P();

            p.cmd = cmd;
            p.fbo = fbo;
            p.renderPass = fbo->getClearRenderPass();
            p.frameIndex = frameIndex;

#if USE_DYNAMIC_RGBA_WRITE_MASKS
            const VkColorComponentFlags allMask[] =
                { VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
            ctx.vkCmdSetColorWriteMaskEXT(cmd, 0, 1, allMask);
#endif

#if USE_DYNAMIC_STENCILS
            ctx.vkCmdSetStencilTestEnableEXT(cmd, VK_FALSE);
            ctx.vkCmdSetStencilOpEXT(cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                     VK_STENCIL_OP_KEEP,
                                     VK_STENCIL_OP_KEEP,
                                     VK_STENCIL_OP_KEEP,
                                     VK_COMPARE_OP_ALWAYS);
            
            vkCmdSetStencilCompareMask(cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                       0xFFFFFFFF);
            vkCmdSetStencilWriteMask(cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                     0xFFFFFFFF);
#endif
            
            begin(renderSize, renderOptions);
        }

        void Render::begin(
            const math::Size2i& renderSize,
            const timeline::RenderOptions& renderOptions)
        {
            TLRENDER_P();

            p.timer = std::chrono::steady_clock::now();

            p.renderSize = renderSize;
            p.renderOptions = renderOptions;
            p.textureCache->setMax(renderOptions.textureCacheByteCount);

            VkDevice device = ctx.device;

            // Destroy old pipelines that are no longer used.
            auto& g = p.garbage[p.frameIndex];
            for (auto& pipeline : g.pipelines)
            {
                vkDestroyPipeline(device, pipeline, nullptr);
            }
            // Destroy old pipelineLayouts that are no longer used.
            for (auto& pipelineLayout : g.pipelineLayouts)
            {
                vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            }
            g.pipelines.clear();
            g.pipelineLayouts.clear();
            g.bindingSets.clear();
            
            const math::Matrix4x4f transform;
            const image::Color4f color(1.F, 1.F, 1.F);

            // Shader used to draw a 2D mesh with a texture * color
            if (!p.shaders["rect"])
            {
#if USE_PRECOMPILED_SHADERS
                p.shaders["rect"] = vlk::Shader::create(
                    ctx,
                    Vertex2NoUVs_spv,
                    Vertex2NoUVs_spv_len,
                    meshFragment_spv,
                    meshFragment_spv_len, "rect");
#else
                p.shaders["rect"] = vlk::Shader::create(
                    ctx, vertex2NoUVsSource(), meshFragmentSource(), "rect");
#endif
                p.shaders["rect"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["rect"]->addPush(
                    "color", color, vlk::kShaderFragment);
                _createBindingSet(p.shaders["rect"]);
            }

            // Shader used to draw a 3d mesh with a texture * push color
            if (!p.shaders["mesh"])
            {
#if USE_PRECOMPILED_SHADERS
                p.shaders["mesh"] = vlk::Shader::create(
                    ctx,
                    Vertex3_spv,
                    Vertex3_spv_len,
                    meshFragment_spv,
                    meshFragment_spv_len, "mesh");
#else
                p.shaders["mesh"] = vlk::Shader::create(
                    ctx, vertexSource(), meshFragmentSource(), "mesh");
#endif
                p.shaders["mesh"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["mesh"]->addPush(
                      "color", color, vlk::kShaderFragment);
                _createBindingSet(p.shaders["mesh"]);
              }

              // Shader used to create a 2D mesh with colors
              if (!p.shaders["colorMesh"])
              {
#if USE_PRECOMPILED_SHADERS
                p.shaders["colorMesh"] = vlk::Shader::create(
                    ctx,
                    colorMeshVertex_spv,
                    colorMeshVertex_spv_len,
                    colorMeshFragment_spv,
                    colorMeshFragment_spv_len, "colorMesh");
#else
                p.shaders["colorMesh"] = vlk::Shader::create(
                    ctx, colorMeshVertexSource(), colorMeshFragmentSource(),
                    "colorMesh");
#endif
                p.shaders["colorMesh"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["colorMesh"]->addPush("color", color, vlk::kShaderFragment);
                _createBindingSet(p.shaders["colorMesh"]);
            }

            // Shader to creates a quad with a texture for text drawing 
            if (!p.shaders["text"])
            {
#if USE_PRECOMPILED_SHADERS
                p.shaders["text"] = vlk::Shader::create(
                    ctx,
                    Vertex2_spv,
                    Vertex2_spv_len,
                    textFragment_spv,
                    textFragment_spv_len, "text");
#else
                p.shaders["text"] = vlk::Shader::create(
                    ctx, vertex2Source(), textFragmentSource(), "text");
#endif
                p.shaders["text"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["text"]->addTexture("textureSampler");
                p.shaders["text"]->addPush(
                    "color", color, vlk::kShaderFragment);
                
                _createBindingSet(p.shaders["text"]);
            }
            
            // Shader to read one mesh with 3 vertex and uvs and a simple textue.
            if (!p.shaders["texture"])
            {
#if USE_PRECOMPILED_SHADERS
                p.shaders["texture"] = vlk::Shader::create(
                    ctx,
                    Vertex3_spv,
                    Vertex3_spv_len,
                    textureFragment_spv,
                    textureFragment_spv_len, "texture");
#else
                p.shaders["texture"] = vlk::Shader::create(
                    ctx, vertexSource(), textureFragmentSource(), "texture");
#endif
                p.shaders["texture"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["texture"]->addTexture("textureSampler");
                p.shaders["texture"]->addPush(
                    "color", color, vlk::kShaderFragment);
                
                _createBindingSet(p.shaders["texture"]);
            }

            // Shader used to read an RGB or YUV image
            if (!p.shaders["image"])
            {
#if USE_PRECOMPILED_SHADERS
                p.shaders["image"] = vlk::Shader::create(
                    ctx,
                    Vertex3_spv,
                    Vertex3_spv_len,
                    imageFragment_spv,
                    imageFragment_spv_len, "image");
#else
                p.shaders["image"] = vlk::Shader::create(
                    ctx, vertexSource(), imageFragmentSource(), "image");
#endif
                p.shaders["image"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);

                UBOTexture ubo;
                p.shaders["image"]->createUniform("ubo", ubo);

                p.shaders["image"]->addTexture("textureSampler0");
                p.shaders["image"]->addTexture("textureSampler1");
                p.shaders["image"]->addTexture("textureSampler2");

                _createBindingSet(p.shaders["image"]);
            }
            if (!p.shaders["overlay"])
            {
#if USE_PRECOMPILED_SHADERS
                p.shaders["overlay"] = vlk::Shader::create(
                    ctx,
                    Vertex3_spv,
                    Vertex3_spv_len,
                    textureFragment_spv,
                    textureFragment_spv_len, "overlay");
#else
                p.shaders["overlay"] = vlk::Shader::create(
                    ctx, vertexSource(), textureFragmentSource(), "overlay");
#endif

                p.shaders["overlay"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["overlay"]->addFBO("textureSampler");
                p.shaders["overlay"]->addPush("color", color, vlk::kShaderFragment);
                
                _createBindingSet(p.shaders["overlay"]);
            }
            if (!p.shaders["difference"])
            {
#if USE_PRECOMPILED_SHADERS
                p.shaders["difference"] = vlk::Shader::create(
                    ctx,
                    Vertex3_spv,
                    Vertex3_spv_len,
                    differenceFragment_spv,
                    differenceFragment_spv_len, "difference");
#else
                p.shaders["difference"] = vlk::Shader::create(
                    ctx, vertexSource(), differenceFragmentSource(),
                    "difference");
#endif

                p.shaders["difference"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["difference"]->addFBO("textureSampler");
                p.shaders["difference"]->addFBO("textureSamplerB");

                _createBindingSet(p.shaders["difference"]);
            }
            if (!p.shaders["dissolve"])
            {
#if USE_PRECOMPILED_SHADERS
                p.shaders["dissolve"] = vlk::Shader::create(
                    ctx,
                    Vertex3_spv,
                    Vertex3_spv_len,
                    textureFragment_spv,
                    textureFragment_spv_len, "dissolve");
#else
                p.shaders["dissolve"] = vlk::Shader::create(
                    ctx, vertexSource(), textureFragmentSource(), "dissolve");
#endif
                p.shaders["dissolve"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["dissolve"]->addFBO("textureSampler");
                p.shaders["dissolve"]->addPush(
                    "color", color, vlk::kShaderFragment);
                _createBindingSet(p.shaders["dissolve"]);
            }
            if (!p.shaders["hard"])
            {
#if USE_PRECOMPILED_SHADERS
                p.shaders["hard"] = vlk::Shader::create(
                    ctx,
                    Vertex2_spv,
                    Vertex2_spv_len,
                    hardFragment_spv,
                    hardFragment_spv_len, "hard");
#else
                p.shaders["hard"] = vlk::Shader::create(
                    ctx, vertex2Source(), softFragmentSource(), "hard");
#endif
                p.shaders["hard"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["hard"]->addPush("color", color);
                _createBindingSet(p.shaders["hard"]);
            }
            if (!p.shaders["soft"])
            {
#if USE_PRECOMPILED_SHADERS
                p.shaders["soft"] = vlk::Shader::create(
                    ctx,
                    Vertex2_spv,
                    Vertex2_spv_len,
                    softFragment_spv,
                    softFragment_spv_len, "soft");
#else
                p.shaders["soft"] = vlk::Shader::create(
                    ctx, vertex2Source(), softFragmentSource(), "soft");
#endif
                p.shaders["soft"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["soft"]->addPush("color", color);
                _createBindingSet(p.shaders["soft"]);
            }
            if (!p.shaders["wipe"])
            {
#if USE_PRECOMPILED_SHADERS
                p.shaders["wipe"] = vlk::Shader::create(
                    ctx,
                    Vertex2NoUVs_spv,
                    Vertex2NoUVs_spv_len,
                    meshFragment_spv,
                    meshFragment_spv_len, "wipe");
#else
                p.shaders["wipe"] = vlk::Shader::create(
                    ctx, vertex2NoUVsSource(), meshFragmentSource(), "wipe");
#endif
                p.shaders["wipe"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["wipe"]->addPush("color", color, vlk::kShaderFragment);
                _createBindingSet(p.shaders["wipe"]);
            }
            _displayShader();


            //
            // Meshes
            //
            if (!p.vbos["image"] || p.vbos["image"]->getSize() != 6)
            {
                p.vbos["image"] =
                    vlk::VBO::create(2 * 3, vlk::VBOType::Pos2_F32_UV_U16);
                p.vaos["image"] = vlk::VAO::create(ctx);
            }
            if (!p.vbos["rect"] || p.vbos["rect"]->getSize() != 6)
            {
                p.vbos["rect"] =
                    vlk::VBO::create(2 * 3, vlk::VBOType::Pos2_F32);
                p.vaos["rect"] = vlk::VAO::create(ctx);
            }
            if (!p.vbos["text"])
            {
                p.vbos["text"] = vlk::VBO::create(2 * 3,
                                                  vlk::VBOType::Pos2_F32_UV_U16);
                p.vaos["text"] = vlk::VAO::create(ctx);
                
                {
                    const std::string pipelineName = "text";
                    const std::string pipelineLayoutName = "text";
                    const std::string shaderName = "text";
                    const std::string meshName = "text";
                    const bool enableBlending = true;
                    createPipeline(p.fbo, pipelineName, pipelineLayoutName,
                                   shaderName, meshName, enableBlending);
                }
            }
            if (!p.vbos["texture"] || p.vbos["texture"]->getSize() != 6)
            {
                p.vbos["texture"] =
                    vlk::VBO::create(2 * 3, vlk::VBOType::Pos2_F32_UV_U16);
                p.vaos["texture"] = vlk::VAO::create(ctx);
            }
            if (!p.vbos["wipe"] || p.vbos["wipe"]->getSize() != 3)
            {
                p.vbos["wipe"] =
                    vlk::VBO::create(1 * 3, vlk::VBOType::Pos2_F32);
                p.vaos["wipe"] = vlk::VAO::create(ctx);
            }
            if (!p.vbos["video"] || p.vbos["video"]->getSize() != 6)
            {
                p.vbos["video"] =
                    vlk::VBO::create(2 * 3, vlk::VBOType::Pos2_F32_UV_U16);
                p.vaos["video"] = vlk::VAO::create(ctx);
            }

            if (renderOptions.clear)
            {
                clearViewport(renderOptions.clearColor);
            }

            setTransform(
                math::ortho(
                    0.F, static_cast<float>(renderSize.w), 0.F,
                    static_cast<float>(renderSize.h), -1.F, 1.F));
            // applyTransforms();
        }

        void Render::end()
        {
            TLRENDER_P();

            p.fbo->transitionToShaderRead(p.cmd);

            const auto now = std::chrono::steady_clock::now();
            const auto diff =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - p.timer);
            p.currentStats.time = diff.count();
            p.stats.push_back(p.currentStats);
            p.currentStats = Private::Stats();
            while (p.stats.size() > 60)
            {
                p.stats.pop_front();
            }
            
            const std::chrono::duration<float> logDiff = now - p.logTimer;
            if (logDiff.count() > 10.F)
            {
                p.logTimer = now;
                if (auto context = _context.lock())
                {
                    Private::Stats average;
                    const size_t size = p.stats.size();
                    if (size > 0)
                    {
                        for (const auto& i : p.stats)
                        {
                            average.time += i.time;
                            average.rects += i.rects;
                            average.meshes += i.meshes;
                            average.meshTriangles += i.meshTriangles;
                            average.text += i.text;
                            average.textTriangles += i.textTriangles;
                            average.textures += i.textures;
                            average.images += i.images;
                            average.pipelineChanges += i.pipelineChanges;
                        }
                        average.time /= p.stats.size();
                        average.rects /= p.stats.size();
                        average.meshes /= p.stats.size();
                        average.meshTriangles /= p.stats.size();
                        average.text /= p.stats.size();
                        average.textTriangles /= p.stats.size();
                        average.textures /= p.stats.size();
                        average.images /= p.stats.size();
                        average.pipelineChanges /= p.stats.size();
                    }

                    context->log(
                        string::Format("tl::timeline_vlk::Render {0}").arg(this),
                        string::Format(
                            "\n"
                            "    Average render time: {0}ms\n"
                            "    Average rectangle count: {1}\n"
                            "    Average mesh count: {2}\n"
                            "    Average mesh triangles: {3}\n"
                            "    Average text count: {4}\n"
                            "    Average text triangles: {5}\n"
                            "    Average texture count: {6}\n"
                            "    Average image count: {7}\n"
                            "    Average pipeline changes: {8}\n"
                            "    Glyph texture atlas: {9}%\n"
                            "    Glyph IDs: {10}")
                            .arg(average.time)
                            .arg(average.rects)
                            .arg(average.meshes)
                            .arg(average.meshTriangles)
                            .arg(average.text)
                            .arg(average.textTriangles)
                            .arg(average.textures)
                            .arg(average.images)
                            .arg(average.pipelineChanges)
                            .arg(p.glyphTextureAtlas->getPercentageUsed())
                            .arg(p.glyphIDs.size()));
                }
            }
        }

        VkCommandBuffer Render::getCommandBuffer() const
        {
            return _p->cmd;
        }

        uint32_t Render::getFrameIndex() const
        {
            return _p->frameIndex;
        }

        Fl_Vk_Context& Render::getContext() const
        {
            return ctx;
        }
        
        math::Size2i Render::getRenderSize() const
        {
            return _p->renderSize;
        }

        void Render::setRenderSize(const math::Size2i& value)
        {
            _p->renderSize = value;
        }

        math::Box2i Render::getViewport() const
        {
            return _p->viewport;
        }

        void Render::setViewport(const math::Box2i& value)
        {
            _p->viewport = value;
        }
        
        void Render::createBindingSet(const std::string& shaderName)
        {
            _createBindingSet(_p->shaders[shaderName]);
        }

        void Render::beginLoadRenderPass()
        {
            TLRENDER_P();

            p.fbo->transitionToColorAttachment(p.cmd);
            p.fbo->beginLoadRenderPass(p.cmd);
        }
        
        void Render::beginRenderPass()
        {
            TLRENDER_P();

            p.fbo->transitionToColorAttachment(p.cmd);
            p.fbo->beginClearRenderPass(p.cmd);
        }

        void Render::endRenderPass()
        {
            TLRENDER_P();
            
            p.fbo->endRenderPass(p.cmd);
        }
        
        void Render::setupViewportAndScissor()
        {
            TLRENDER_P();
            
            p.fbo->setupViewportAndScissor();
        }
        
        void Render::clearViewport(const image::Color4f& value)
        {
            TLRENDER_P();

            VkClearValue clearValues[2];
            clearValues[0].color = {value.r, value.g, value.b, value.a};
            clearValues[1].depthStencil = {1.F, 0};

            VkRenderPassBeginInfo rpBegin{};
            rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rpBegin.renderPass = p.fbo->getClearRenderPass();
            rpBegin.framebuffer = p.fbo->getFramebuffer();
            rpBegin.renderArea.offset = {0, 0};
            rpBegin.renderArea.extent = p.fbo->getExtent(); // Use FBO extent
            rpBegin.clearValueCount =
                1 +
                static_cast<uint16_t>(p.fbo->hasDepth() || p.fbo->hasStencil());
            rpBegin.pClearValues = clearValues;

            // Begin the first render pass instance within the single command
            // buffer
            vkCmdBeginRenderPass(p.cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdEndRenderPass(p.cmd);
        }

        bool Render::getClipRectEnabled() const
        {
            return _p->clipRectEnabled;
        }

        void Render::setClipRectEnabled(bool value)
        {
            _p->clipRectEnabled = value;
        }

        math::Box2i Render::getClipRect() const
        {
            return _p->clipRect;
        }

        std::shared_ptr<vlk::OffscreenBuffer> Render::getFBO() const
        {
            return _p->fbo;
        }

        void Render::setClipRect(const math::Box2i& value)
        {
            TLRENDER_P();
            p.clipRect = value;
            if (p.clipRectEnabled && p.clipRect.w() > 0 && p.clipRect.h() > 0)
            {
                VkRect2D scissorRect = {
                    p.clipRect.x(),
                    p.clipRect.y(),
                    static_cast<uint32_t>(p.clipRect.w()),
                    static_cast<uint32_t>(p.clipRect.h())
                };
                vkCmdSetScissor(p.cmd, 0, 1, &scissorRect);
            }
        }

        math::Matrix4x4f Render::getTransform() const
        {
            return _p->transform;
        }

        void Render::setTransform(const math::Matrix4x4f& value)
        {
            _p->transform = value;
        }
        
        void Render::applyTransforms()
        {
            TLRENDER_P();
            
            for (auto& i : p.shaders)
            {
                if (i.second)
                {
                    i.second->bind(p.frameIndex);
                    i.second->setUniform("transform.mvp", p.transform,
                                         vlk::kShaderVertex);
                }
            }
        }

#if defined(TLRENDER_OCIO)

        void Render::_addTextures(
            std::vector<std::shared_ptr<vlk::Texture> >& textures,
            const OCIO::GpuShaderDescRcPtr& shaderDesc)
        {
            TLRENDER_P();

            // Create 3D textures.
            const unsigned num3DTextures = shaderDesc->getNum3DTextures();
            for (unsigned i = 0; i < num3DTextures; ++i)
            {
                const char* textureName = nullptr;
                const char* samplerName = nullptr;
                unsigned edgelen = 0;
                OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                shaderDesc->get3DTexture(
                    i, textureName, samplerName, edgelen, interpolation);
                if (!textureName || !*textureName || !samplerName ||
                    !*samplerName || 0 == edgelen)
                {
                    throw std::runtime_error(
                        "The OCIO texture data is corrupted");
                }

                const float* values = nullptr;
                shaderDesc->get3DTextureValues(i, values);
                if (!values)
                {
                    throw std::runtime_error(
                        "The OCIO texture values are missing");
                }

                VkFormat imageFormat = VK_FORMAT_R32G32B32_SFLOAT;
                const uint32_t width = edgelen;
                const uint32_t height = edgelen;
                const uint32_t depth = edgelen;
                const uint16_t channels = 3;

                auto texture = vlk::Texture::create(
                    ctx, VK_IMAGE_TYPE_3D, width, height, depth, imageFormat,
                    samplerName);
                texture->copy(
                    reinterpret_cast<const uint8_t*>(values),
                    width * height * depth * channels * sizeof(float));
                texture->transitionToShaderRead(p.cmd);
                textures.push_back(texture);
            }

            // Create 1D textures.
            const unsigned numTextures = shaderDesc->getNumTextures();
            for (unsigned i = 0; i < numTextures; ++i)
            {
                unsigned width = 0;
                unsigned height = 0;
                const char* textureName = nullptr;
                const char* samplerName = nullptr;
                OCIO::GpuShaderDesc::TextureType channel =
                    OCIO::GpuShaderDesc::TEXTURE_RGB_CHANNEL;
                OCIO::GpuShaderCreator::TextureDimensions dimensions =
                    OCIO::GpuShaderDesc::TEXTURE_1D;
                OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                shaderDesc->getTexture(
                    i, textureName, samplerName, width, height, channel,
                    dimensions, interpolation);
                if (!textureName || !*textureName || !samplerName ||
                    !*samplerName || width == 0)
                {
                    throw std::runtime_error(
                        "The OCIO texture data is corrupted");
                }

                const float* values = nullptr;
                shaderDesc->getTextureValues(i, values);
                if (!values)
                {
                    throw std::runtime_error(
                        "The OCIO texture values are missing");
                }

                uint16_t channels = 3;
                VkFormat imageFormat = VK_FORMAT_R32G32B32_SFLOAT;
                if (OCIO::GpuShaderCreator::TEXTURE_RED_CHANNEL == channel)
                {
                    channels = 1;
                    imageFormat = VK_FORMAT_R32_SFLOAT;
                }

                VkImageType imageType;
                switch (dimensions)
                {
                case OCIO::GpuShaderDesc::TEXTURE_1D:
                    imageType = VK_IMAGE_TYPE_1D;
                    if (height == 0)
                        height = 1;
                    break;
                case OCIO::GpuShaderDesc::TEXTURE_2D:
                    imageType = VK_IMAGE_TYPE_2D;
                    break;
                default:
                    throw std::runtime_error("Unknown OCIO image type");
                }

                auto texture = vlk::Texture::create(
                    ctx, imageType, width, height, 0, imageFormat, samplerName);
                texture->copy(
                    reinterpret_cast<const uint8_t*>(values),
                    width * height * channels * sizeof(float));
                texture->transitionToShaderRead(p.cmd);
                textures.push_back(texture);
            }
        }
#endif // TLRENDER_OCIO

#if defined(TLRENDER_LIBPLACEBO)
        void Render::_addTextures(
            std::vector<std::shared_ptr<vlk::Texture> >& textures,
            const pl_shader_res* res)
        {
            TLRENDER_P();

            // Create 3D textures.
            for (unsigned i = 0; i < res->num_descriptors; ++i)
            {
                const pl_shader_desc* sd = &res->descriptors[i];
                switch (sd->desc.type)
                {
                case PL_DESC_STORAGE_IMG:
                    throw std::runtime_error("Unimplemented storage image");
                case PL_DESC_SAMPLED_TEX:
                {
                    pl_tex tex = reinterpret_cast<pl_tex>(sd->binding.object);
                    pl_fmt fmt = tex->params.format;

                    int dims = pl_tex_params_dimension(tex->params);
                    assert(dims >= 1 && dims <= 3);

                    const char* samplerName = sd->desc.name;
                    int pixel_fmt_size = fmt->internal_size / fmt->num_components;
                    int channels = fmt->num_components;

                    // Map libplacebo format to Vulkan format
                    VkFormat imageFormat = to_vk_format(fmt);

                    // Texture dimensions
                    uint32_t width = tex->params.w;
                    uint32_t height = (dims >= 2) ? tex->params.h : 1;
                    uint32_t depth = (dims == 3) ? tex->params.d : 1;

                    assert(width > 0);
                    if (dims >= 2)
                        assert(height > 0);
                    if (dims == 3)
                        assert(depth > 0);

                    // Get texture data from libplacebo
                    const void* values = pl_tex_dummy_data(tex);
                    if (!values)
                    {
                        throw std::runtime_error(
                            "Could not read pl_tex_dummy_data");
                    }

                    // Determine image type
                    VkImageType imageType = (dims == 1)   ? VK_IMAGE_TYPE_1D
                                            : (dims == 2) ? VK_IMAGE_TYPE_2D
                                                          : VK_IMAGE_TYPE_3D;
                    if (imageType == VK_IMAGE_TYPE_1D)
                        height = depth = 1;
                    else if (imageType == VK_IMAGE_TYPE_2D)
                        depth = 1;
                    vlk::TextureOptions options;
                    options.tiling = VK_IMAGE_TILING_OPTIMAL;
                    auto texture = vlk::Texture::create(
                        ctx, imageType, width, height, depth, imageFormat,
                        samplerName);
                    texture->copy(
                        reinterpret_cast<const uint8_t*>(values),
                        width * height * depth * fmt->internal_size);
                    texture->transitionToShaderRead(p.cmd);
                    textures.push_back(texture);
                    break;
                }
                case PL_DESC_BUF_UNIFORM:
                    break;
                case PL_DESC_BUF_STORAGE:
                    break;
                case PL_DESC_BUF_TEXEL_UNIFORM:
                    break;
                case PL_DESC_BUF_TEXEL_STORAGE:
                    break;
                case PL_DESC_INVALID:
                case PL_DESC_TYPE_COUNT:
                    break;
                }
            }
        }
#endif

        void Render::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            TLRENDER_P();
            if (value == p.ocioOptions)
                return;

#if defined(TLRENDER_OCIO)
            p.ocioData.reset();
#endif // TLRENDER_OCIO

            p.ocioOptions = value;

#if defined(TLRENDER_OCIO)
            
            if (p.ocioOptions.enabled)
            {
                p.ocioData.reset(new OCIOData);

                if (!p.ocioOptions.fileName.empty())
                {
                    p.ocioData->config = OCIO::Config::CreateFromFile(
                        p.ocioOptions.fileName.c_str());
                }
                else
                {
                    p.ocioData->config = OCIO::GetCurrentConfig();
                }
                if (!p.ocioData->config)
                {
                    throw std::runtime_error("Cannot get OCIO configuration");
                }
                p.ocioData->transform = OCIO::DisplayViewTransform::Create();
                if (!p.ocioData->transform)
                {
                    p.ocioData.reset();
                    throw std::runtime_error("Cannot create OCIO transform");
                }
                if (!p.ocioOptions.input.empty())
                {
                    p.ocioData->icsDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
                    if (!p.ocioData->icsDesc)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error("Cannot create OCIO transform");
                    }
                
                    OCIO::ConstColorSpaceRcPtr srcCS =
                        p.ocioData->config->getColorSpace(
                            p.ocioOptions.input.c_str());
                    OCIO::ConstColorSpaceRcPtr dstCS =
                        p.ocioData->config->getColorSpace(
                            OCIO::ROLE_SCENE_LINEAR);
                    p.ocioData->processor = p.ocioData->config->getProcessor(
                        p.ocioData->config->getCurrentContext(), srcCS, dstCS);
                    if (!p.ocioData->processor)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error("Cannot get OCIO processor");
                    }

                    p.ocioData->gpuProcessor =
                        p.ocioData->processor->getOptimizedGPUProcessor(
                            OCIO::OPTIMIZATION_DEFAULT);
                    if (!p.ocioData->gpuProcessor)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error(
                            "Cannot get OCIO GPU processor for ICS");
                    }
                    p.ocioData->icsDesc =
                        OCIO::GpuShaderDesc::CreateShaderDesc();
                    if (!p.ocioData->icsDesc)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error(
                            "Cannot create OCIO ICS shader description");
                    }

                    p.ocioData->icsDesc->setLanguage(
                        OCIO::GPU_LANGUAGE_GLSL_4_0);
                    p.ocioData->icsDesc->setFunctionName("ocioICSFunc");
                    p.ocioData->icsDesc->setResourcePrefix("ocioICS"); // ocio?
                    p.ocioData->gpuProcessor->extractGpuShaderInfo(
                        p.ocioData->icsDesc);
                    try
                    {
                        _addTextures(p.ocioData->textures, p.ocioData->icsDesc);
                    }
                    catch (const std::exception& e)
                    {
                        p.ocioData.reset();
                        throw e;
                    }
                }
                if (!p.ocioOptions.display.empty() &&
                    !p.ocioOptions.view.empty())
                {
                    p.ocioData->transform->setSrc(OCIO::ROLE_SCENE_LINEAR);
                    p.ocioData->transform->setDisplay(
                        p.ocioOptions.display.c_str());
                    p.ocioData->transform->setView(p.ocioOptions.view.c_str());

                    p.ocioData->lvp = OCIO::LegacyViewingPipeline::Create();
                    if (!p.ocioData->lvp)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error(
                            "Cannot create OCIO viewing pipeline");
                    }
                    p.ocioData->lvp->setDisplayViewTransform(
                        p.ocioData->transform);
                    p.ocioData->lvp->setLooksOverrideEnabled(true);
                    p.ocioData->lvp->setLooksOverride(
                        p.ocioOptions.look.c_str());

                    p.ocioData->processor = p.ocioData->lvp->getProcessor(
                        p.ocioData->config,
                        p.ocioData->config->getCurrentContext());
                    if (!p.ocioData->processor)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error("Cannot get OCIO processor");
                    }
                    p.ocioData->gpuProcessor =
                        p.ocioData->processor->getOptimizedGPUProcessor(
                            OCIO::OPTIMIZATION_DEFAULT);
                    if (!p.ocioData->gpuProcessor)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error(
                            "Cannot get OCIO GPU processor");
                    }
                    p.ocioData->shaderDesc =
                        OCIO::GpuShaderDesc::CreateShaderDesc();
                    if (!p.ocioData->shaderDesc)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error(
                            "Cannot create OCIO shader description");
                    }
                    p.ocioData->shaderDesc->setLanguage(
                        OCIO::GPU_LANGUAGE_GLSL_4_0);
                    p.ocioData->shaderDesc->setFunctionName("ocioDisplayFunc");
                    p.ocioData->shaderDesc->setResourcePrefix("ocio");
                    p.ocioData->gpuProcessor->extractGpuShaderInfo(
                        p.ocioData->shaderDesc);
                    try
                    {
                        _addTextures(
                            p.ocioData->textures, p.ocioData->shaderDesc);
                    }
                    catch (const std::exception& e)
                    {
                        p.ocioData.reset();
                        throw e;
                    }
                }
            }
#endif // TLRENDER_OCIO

            p.shaders["display"].reset();
            _displayShader();
        }

        void Render::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;

#if defined(TLRENDER_OCIO)
            //wait_device();
            p.lutData.reset();
#endif // TLRENDER_OCIO

            p.lutOptions = value;

#if defined(TLRENDER_OCIO)
            if (p.lutOptions.enabled && !p.lutOptions.fileName.empty())
            {
                p.lutData.reset(new OCIOLUTData);

                p.lutData->config = OCIO::Config::CreateRaw();
                if (!p.lutData->config)
                {
                    throw std::runtime_error(
                        "Cannot create OCIO configuration");
                }

                p.lutData->transform = OCIO::FileTransform::Create();
                if (!p.lutData->transform)
                {
                    p.lutData.reset();
                    throw std::runtime_error("Cannot create OCIO transform");
                }
                p.lutData->transform->setSrc(p.lutOptions.fileName.c_str());
                p.lutData->transform->validate();

                p.lutData->processor =
                    p.lutData->config->getProcessor(p.lutData->transform);
                if (!p.lutData->processor)
                {
                    p.lutData.reset();
                    throw std::runtime_error("Cannot get OCIO processor");
                }
                p.lutData->gpuProcessor =
                    p.lutData->processor->getDefaultGPUProcessor();
                if (!p.lutData->gpuProcessor)
                {
                    p.lutData.reset();
                    throw std::runtime_error("Cannot get OCIO GPU processor");
                }
                p.lutData->shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
                if (!p.lutData->shaderDesc)
                {
                    p.lutData.reset();
                    throw std::runtime_error(
                        "Cannot create OCIO shader description");
                }
                p.lutData->shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_4_0);
                p.lutData->shaderDesc->setFunctionName("lutFunc");
                p.lutData->shaderDesc->setResourcePrefix("lut");
                p.lutData->gpuProcessor->extractGpuShaderInfo(
                    p.lutData->shaderDesc);
                try
                {
                    _addTextures(p.lutData->textures, p.lutData->shaderDesc);
                }
                catch (const std::exception& e)
                {
                    p.ocioData.reset();
                    throw e;
                }
            }
#endif // TLRENDER_OCIO

            p.shaders["display"].reset();
            _displayShader();
        }

        void Render::setHDROptions(const timeline::HDROptions& value)
        {
            TLRENDER_P();
            if (value == p.hdrOptions &&
                value.passthru == p.hdrOptions.passthru)
                return;

            p.hdrOptions = value;
            p.hdrOptions.passthru = value.passthru;

#if defined(TLRENDER_LIBPLACEBO)
            if (p.hdrOptions.passthru || p.hdrOptions.tonemap)
            {
                p.placeboData.reset(new LibPlaceboData);
            }
            else
            {
                p.placeboData.reset();
            }
#endif // TLRENDER_LIBPLACEBO
            
            p.shaders["display"].reset();
            _displayShader();
        }

        void Render::_displayShader()
        {
            TLRENDER_P();

            if (!p.shaders["display"])
            {
                if (p.pipelines.count("display") != 0)
                {
                    auto pair = p.pipelines["display"];
                    p.garbage[p.frameIndex].pipelines.push_back(pair.second);
                

                    vlk::PipelineCreationState pipelineState;
                    pair = std::make_pair(pipelineState, VK_NULL_HANDLE);
                    p.pipelines["display"] = pair;
                }
                if (p.pipelineLayouts["display"])
                {
                    p.garbage[p.frameIndex].pipelineLayouts.push_back(p.pipelineLayouts["display"]);
                    p.pipelineLayouts["display"] = VK_NULL_HANDLE;
                }

                std::string toneMapDef;
                std::string ocioICSDef;
                std::string ocioICS;
                std::string ocioDef;
                std::string ocio;
                std::string lutDef;
                std::string lut;
                std::string toneMap;

                // Start of binding index
                p.bindingIndex = 6;
                std::size_t pushOffset = 0;
#if defined(TLRENDER_LIBPLACEBO)
                if (p.placeboData)
                {
                    pl_shader_params shader_params;
                    memset(&shader_params, 0, sizeof(pl_shader_params));
                
                    shader_params.id = 1;
                    shader_params.gpu = p.placeboData->gpu;
                    shader_params.dynamic_constants = false;
            
                    pl_shader_reset(p.placeboData->shader, &shader_params);

                    pl_color_map_params cmap = pl_color_map_high_quality_params;

                    // defaults, generates LUTs if state is set.
                    cmap.gamut_mapping = &pl_gamut_map_perceptual;
                    switch (p.hdrOptions.gamutMapping)
                    {
                    case timeline::HDRGamutMapping::Clip:
                        cmap.gamut_mapping = &pl_gamut_map_clip;
                        break;
                    case timeline::HDRGamutMapping::Perceptual:
                        cmap.gamut_mapping = &pl_gamut_map_perceptual;
                        break;
                    case timeline::HDRGamutMapping::Relative:
                        cmap.gamut_mapping = &pl_gamut_map_relative;
                        break;
                    case timeline::HDRGamutMapping::Saturation:
                        cmap.gamut_mapping = &pl_gamut_map_saturation;
                        break;
                    case timeline::HDRGamutMapping::Absolute:
                        cmap.gamut_mapping = &pl_gamut_map_absolute;
                        break;
                    case timeline::HDRGamutMapping::Desaturate:
                        cmap.gamut_mapping = &pl_gamut_map_desaturate;
                        break;
                    case timeline::HDRGamutMapping::Darken:
                        cmap.gamut_mapping = &pl_gamut_map_darken;
                        break;
                    case timeline::HDRGamutMapping::Highlight:
                        cmap.gamut_mapping = &pl_gamut_map_highlight;
                        break;
                    case timeline::HDRGamutMapping::Linear:
                        cmap.gamut_mapping = &pl_gamut_map_linear;
                        break;
                    default:
                        break;
                    }

                    cmap.tone_mapping_function = nullptr;

                    const image::HDRData& data = p.hdrOptions.hdrData;

                    pl_color_space src_colorspace;
                    memset(&src_colorspace, 0, sizeof(pl_color_space));

                    bool hasHDR = false;
                    switch (data.eotf)
                    {
                    case image::EOTFType::EOTF_BT2100_PQ: // PQ (HDR10)
                    case image::EOTFType::EOTF_BT2020:    // PQ (HDR10)
                        src_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
                        src_colorspace.transfer = PL_COLOR_TRC_PQ;
                        hasHDR = true;
                        break;
                    case image::EOTFType::EOTF_BT2100_HLG: // HLG
                        src_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
                        src_colorspace.transfer = PL_COLOR_TRC_HLG;
                        hasHDR = true;
                        break;
                    case image::EOTFType::EOTF_BT709:
                        src_colorspace.primaries = PL_COLOR_PRIM_BT_709;
                        src_colorspace.transfer = PL_COLOR_TRC_BT_1886;
                        break;
                    case image::EOTFType::EOTF_BT601:
                    default:
                        src_colorspace.primaries = PL_COLOR_PRIM_BT_709;
                        src_colorspace.transfer = PL_COLOR_TRC_SRGB;
                        break;
                    }

                    if (hasHDR)
                    {
                        cmap.metadata = PL_HDR_METADATA_ANY;

                        pl_hdr_metadata& hdr = src_colorspace.hdr;
                        hdr.min_luma = data.displayMasteringLuminance.getMin();
                        hdr.max_luma = data.displayMasteringLuminance.getMax();
                        hdr.prim.red.x =
                            data.primaries[image::HDRPrimaries::Red][0];
                        hdr.prim.red.y =
                            data.primaries[image::HDRPrimaries::Red][1];
                        hdr.prim.green.x =
                            data.primaries[image::HDRPrimaries::Green][0];
                        hdr.prim.green.y =
                            data.primaries[image::HDRPrimaries::Green][1];
                        hdr.prim.blue.x =
                            data.primaries[image::HDRPrimaries::Blue][0];
                        hdr.prim.blue.y =
                            data.primaries[image::HDRPrimaries::Blue][1];
                        hdr.prim.white.x =
                            data.primaries[image::HDRPrimaries::White][0];
                        hdr.prim.white.y =
                            data.primaries[image::HDRPrimaries::White][1];
                        hdr.max_cll = data.maxCLL;
                        hdr.max_fall = data.maxFALL;
                        hdr.scene_max[0] = data.sceneMax[0];
                        hdr.scene_max[1] = data.sceneMax[1];
                        hdr.scene_max[2] = data.sceneMax[2];
                        hdr.scene_avg = data.sceneAvg;
                        hdr.ootf.target_luma = data.ootf.targetLuma;
                        hdr.ootf.knee_x = data.ootf.kneeX;
                        hdr.ootf.knee_y = data.ootf.kneeY;
                        hdr.ootf.num_anchors = data.ootf.numAnchors;
                        for (int i = 0; i < hdr.ootf.num_anchors; i++)
                            hdr.ootf.anchors[i] = data.ootf.anchors[i];
                    }
                    else
                    {
                        src_colorspace.primaries = PL_COLOR_PRIM_BT_709;

                        // SDR uses sRGB-like gamma
                        src_colorspace.transfer = PL_COLOR_TRC_SRGB; // right?
                    }

                    pl_color_space_infer(&src_colorspace);

                    pl_color_space dst_colorspace;
                    memset(&dst_colorspace, 0, sizeof(pl_color_space));

                    if (p.hdrMonitorFound)
                    {
                        dst_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
                        dst_colorspace.transfer = PL_COLOR_TRC_PQ;
                        if (ctx.colorSpace ==
                            VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT)
                        {
                            dst_colorspace.primaries = PL_COLOR_PRIM_DISPLAY_P3;
                            dst_colorspace.transfer = PL_COLOR_TRC_BT_1886;
                        }
                        else if (ctx.colorSpace == VK_COLOR_SPACE_HDR10_HLG_EXT)
                        {
                            dst_colorspace.transfer = PL_COLOR_TRC_HLG;
                        }
                        else if (
                            ctx.colorSpace == VK_COLOR_SPACE_DOLBYVISION_EXT)
                        {
                            // \@todo:  How to handle this?
                            // PL_COLOR_TRC_DOLBYVISION does not exist.
                            // According to libplacebo:
                            // "Unlikely to ever be implemented"
                            // dst_colorspace.transfer = ???
                            dst_colorspace.transfer = PL_COLOR_TRC_PQ;
                        }

                        // For SDR content on HDR monitor, enable tone mapping
                        // to fit SDR into HDR
                        if (!hasHDR)
                        {
                            cmap.tone_mapping_function = &pl_tone_map_st2094_40;
                            cmap.metadata = PL_HDR_METADATA_NONE; // Simplify
                        }
                        else
                        {
                            cmap.tone_mapping_function = &pl_tone_map_spline;
                        }
                    }
                    else
                    {
                        dst_colorspace.primaries = PL_COLOR_PRIM_BT_709;
                        dst_colorspace.transfer = PL_COLOR_TRC_BT_1886;
                        dst_colorspace.hdr.min_luma = 0.F;
                        dst_colorspace.hdr.max_luma = 100.0F; // SDR peak in nits

                        if (hasHDR)
                        {
                            switch (p.hdrOptions.algorithm)
                            {
                            case timeline::HDRTonemapAlgorithm::kNone:
                                cmap.tone_mapping_function = nullptr;
                                break;
                            case timeline::HDRTonemapAlgorithm::Clip:
                                break;
                            case timeline::HDRTonemapAlgorithm::ST2094_10:
                                cmap.tone_mapping_function =
                                    &pl_tone_map_st2094_10;
                                break;
                            case timeline::HDRTonemapAlgorithm::BT2390:
                                cmap.tone_mapping_function =
                                    &pl_tone_map_bt2390;
                                break;
                            case timeline::HDRTonemapAlgorithm::BT2446A:
                                cmap.tone_mapping_function =
                                    &pl_tone_map_bt2446a;
                                break;
                            case timeline::HDRTonemapAlgorithm::Spline:
                                cmap.tone_mapping_function =
                                    &pl_tone_map_spline;
                                break;
                            case timeline::HDRTonemapAlgorithm::Reinhard:
                                cmap.tone_mapping_function =
                                    &pl_tone_map_reinhard;
                                break;
                            case timeline::HDRTonemapAlgorithm::Mobius:
                                cmap.tone_mapping_function =
                                    &pl_tone_map_mobius;
                                break;
                            case timeline::HDRTonemapAlgorithm::Hable:
                                cmap.tone_mapping_function = &pl_tone_map_hable;
                                break;
                            case timeline::HDRTonemapAlgorithm::Gamma:
                                cmap.tone_mapping_function = &pl_tone_map_gamma;
                                break;
                            case timeline::HDRTonemapAlgorithm::Linear:
                                cmap.tone_mapping_function =
                                    &pl_tone_map_linear;
                                break;
                            case timeline::HDRTonemapAlgorithm::LinearLight:
                                cmap.tone_mapping_function =
                                    &pl_tone_map_linear_light;
                                break;
                            case timeline::HDRTonemapAlgorithm::ST2094_40:
                                cmap.tone_mapping_function =
                                    &pl_tone_map_st2094_40;
                                break;
                            default:
                                break;
                            }
                        }
                        else
                        {
                            cmap.tone_mapping_function = &pl_tone_map_hable;
                        }
                    }

                    pl_color_space_infer(&dst_colorspace);

                    pl_color_map_args color_map_args;
                    memset(&color_map_args, 0, sizeof(pl_color_map_args));

                    color_map_args.src = src_colorspace;
                    color_map_args.dst = dst_colorspace;
                    color_map_args.prelinearized = false;
                    
                    if (p.placeboData->state) {
                        pl_shader_obj_destroy(&p.placeboData->state);
                        p.placeboData->state = NULL;
                    }
                    color_map_args.state = &(p.placeboData->state);

                    pl_shader_color_map_ex(p.placeboData->shader, &cmap,
                                           &color_map_args);
                    
                    const pl_shader_res* res =
                        pl_shader_finalize(p.placeboData->shader);
                    p.placeboData->res = res;
                    if (!res)
                    {
                        p.placeboData.reset();
                        throw std::runtime_error("pl_shader_finalize failed!");
                    }

                    std::stringstream s;

                    // std::cerr << "num_vertex_attribs=" <<
                    // res->num_vertex_attribs
                    //           << std::endl
                    //           << "num_descriptors="
                    //           << res->num_descriptors << std::endl
                    //           << "num_variables=" << res->num_variables
                    //           << std::endl
                    //           << "num_constants="
                    //           << res->num_constants << std::endl;
                    for (int i = 0; i < res->num_descriptors; i++)
                    {
                        const pl_shader_desc* sd = &res->descriptors[i];
                        const pl_desc* desc = &sd->desc;
                        switch (desc->type)
                        {
                        case PL_DESC_SAMPLED_TEX:
                        case PL_DESC_STORAGE_IMG:
                        {
                            static const char* types[] = {
                                "sampler1D",
                                "sampler2D",
                                "sampler3D",
                            };

                            pl_desc_binding binding = sd->binding;
                            pl_tex tex = (pl_tex)binding.object;
                            int dims = pl_tex_params_dimension(tex->params);
                            const char* type = types[dims - 1];

                            char prefix = ' ';
                            switch (tex->params.format->type)
                            {
                            case PL_FMT_UINT:
                                prefix = 'u';
                                break;
                            case PL_FMT_SINT:
                                prefix = 'i';
                                break;
                            case PL_FMT_FLOAT:
                            case PL_FMT_UNORM:
                            case PL_FMT_SNORM:
                            default:
                                break;
                            }

                            s << "layout(binding=" << p.bindingIndex++
                              << ") uniform " << prefix << type << " "
                              << desc->name << ";" << std::endl;
                            break;
                        }
                        case PL_DESC_BUF_UNIFORM:
                            throw "buf uniform";
                            break;
                        case PL_DESC_BUF_STORAGE:
                            throw "buf storage";
                        case PL_DESC_BUF_TEXEL_UNIFORM:
                            throw "buf texel uniform";
                        case PL_DESC_BUF_TEXEL_STORAGE:
                            throw "buf texel storage";
                        case PL_DESC_INVALID:
                        case PL_DESC_TYPE_COUNT:
                            throw "invalid or count";
                            break;
                        }
                    }

                    s << "//" << std::endl
                      << "// Variables"
                      << "//" << std::endl
                      << std::endl;

                    s << "layout(std430, push_constant) uniform PushC {\n";
                    for (int i = 0; i < res->num_variables; ++i)
                    {
                        const struct pl_shader_var shader_var = res->variables[i];
                        const struct pl_var var = shader_var.var;
                        const std::string glsl_type = pl_var_glsl_type_name(var);
                        struct pl_var_layout layout = pl_std430_layout(pushOffset, &var);

                        s << "\tlayout(offset=" << layout.offset << ") " << glsl_type << " " << var.name << ";\n";
                        pushOffset = layout.offset + layout.size;
    
                    }
                    s << "};\n";
                    
                    s << std::endl
                      << "//" << std::endl
                      << "// Constants" << std::endl
                      << "//" << std::endl
                      << std::endl;
                    for (int i = 0; i < res->num_constants; ++i)
                    {
                        // s << "layout(constant_id=" << i << ") ";
                        const struct pl_shader_const constant =
                            res->constants[i];
                        switch (constant.type)
                        {
                        case PL_VAR_SINT:
                            s << "const int " << constant.name << " = "
                              << *(reinterpret_cast<const int*>(constant.data));
                            break;
                        case PL_VAR_UINT:
                            s << "const uint " << constant.name << " = "
                              << *(reinterpret_cast<const unsigned*>(
                                       constant.data));
                            break;
                        case PL_VAR_FLOAT:
                            s << "const float " << constant.name << " = "
                              << *(reinterpret_cast<const float*>(
                                       constant.data));
                            break;
                        default:
                            break;
                        }
                        s << ";" << std::endl;
                    }

                    s << res->glsl << std::endl;
                    toneMapDef = s.str();
                
                    try
                    {
                        _addTextures(p.placeboData->textures, res);
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << e.what() << std::endl;
                        p.placeboData.reset();
                        throw e;
                    }
                    toneMap = "outColor = ";
                    toneMap += res->name;
                    toneMap += "(outColor);\n";
                
                }
#endif
                
#if defined(TLRENDER_OCIO)
                if (p.ocioData && p.ocioData->icsDesc)
                {
                    ocioICSDef = p.ocioData->icsDesc->getShaderText();
                    ocioICSDef =
                        replaceUniformSampler(ocioICSDef, p.bindingIndex);
                    ocioICS = "outColor = ocioICSFunc(outColor);";
                }
                if (p.ocioData && p.ocioData->shaderDesc)
                {
                    ocioDef = p.ocioData->shaderDesc->getShaderText();
                    ocioDef = replaceUniformSampler(ocioDef, p.bindingIndex);
                    ocio = "outColor = ocioDisplayFunc(outColor);";
                }
                if (p.lutData && p.lutData->shaderDesc)
                {
                    lutDef = p.lutData->shaderDesc->getShaderText();
                    lutDef = replaceUniformSampler(lutDef, p.bindingIndex);
                    lut = "outColor = lutFunc(outColor);";
                }
#endif // TLRENDER_OCIO
                
                const std::string source = displayFragmentSource(
                    ocioICSDef, ocioICS, ocioDef, ocio, lutDef, lut,
                    p.lutOptions.order, toneMapDef, toneMap);
                if (auto context = _context.lock())
                {
                    context->log(
                        "tl::vlk::VulkanRender", "Creating display shader");
                }


#if defined(TLRENDER_LIBPLACEBO)
                try
                {
                    if (p.placeboData)
                    {
                        p.placeboData->textures.clear();
                        _addTextures(p.placeboData->textures,
                                     p.placeboData->res);
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                    p.placeboData.reset();
                    throw e;
                }
#endif
                    
#if USE_PRECOMPILED_SHADERS
                p.shaders["display"] =
                    vlk::Shader::create(ctx, Vertex3_spv, Vertex3_spv_len,
                                        source, "display");
#else
                p.shaders["display"] =
                    vlk::Shader::create(ctx, vertexSource(), source, "display");
#endif
                p.shaders["display"]->createUniform(
                    "transform.mvp", p.transform, vlk::kShaderVertex);
                p.shaders["display"]->addFBO("textureSampler");

                UBOLevels uboLevels;
                p.shaders["display"]->createUniform("uboLevels", uboLevels);

                // \@unused in mrv2 (used to keep reference of gain UI)
                // timeline::EXRDisplay exrDisplay;
                // p.shaders["display"]->createUniform(
                //     "uboEXRDisplay", exrDisplay);

                UBONormalize uboNormalize;
                p.shaders["display"]->createUniform(
                    "uboNormalize", uboNormalize);

                UBOColor uboColor;
                p.shaders["display"]->createUniform("uboColor", uboColor);

                UBOOptions ubo;
                p.shaders["display"]->createUniform("ubo", ubo);
#if defined(TLRENDER_LIBPLACEBO)
                if (p.placeboData)
                {
                    for (const auto& texture : p.placeboData->textures)
                    {
                        p.shaders["display"]->addTexture(texture->getName());
                    }
                }
#endif
#if defined(TLRENDER_OCIO)
                if (p.ocioData)
                {
                    for (const auto& texture : p.ocioData->textures)
                    {
                        p.shaders["display"]->addTexture(texture->getName());
                    }
                }
                if (p.lutData)
                {
                    for (const auto& texture : p.lutData->textures)
                    {
                        p.shaders["display"]->addTexture(texture->getName());
                    }
                }
#endif // TLRENDER_OCIO
                p.shaders["display"]->createPush("libplacebo", pushOffset, vlk::kShaderFragment);
                
                _createBindingSet(p.shaders["display"]);
            }
        }
    

            
#if defined(TLRENDER_LIBPLACEBO)
        std::string Render::_debugPLVar(const struct pl_shader_var& shader_var)
        {
            
            std::stringstream s;
            const struct pl_var var = shader_var.var;
            std::string glsl_type = pl_var_glsl_type_name(var);
            s << "const " << glsl_type << " " << var.name;
            if (!shader_var.data)
            {
                s << ";" << std::endl;
            }
            else
            {
                const int dim_v = var.dim_v;
                const int dim_m = var.dim_m;
                switch (var.type)
                {
                case PL_VAR_SINT:
                {
                    int* m = (int*)shader_var.data;
                    s << " = " << m[0] << ";" << std::endl;
                    break;
                }
                case PL_VAR_UINT:
                {
                    unsigned* m = (unsigned*)shader_var.data;
                    s << " = " << m[0] << ";" << std::endl;
                    break;
                }
                case PL_VAR_FLOAT:
                {
                    float* m = (float*)shader_var.data;
                    if (dim_m > 1 && dim_v > 1)
                    {
                        s << " = " << glsl_type << "(";
                        for (int c = 0; c < dim_v; ++c)
                        {
                            for (int r = 0; r < dim_m; ++r)
                            {
                                int index = c * dim_m + r;
                                s << m[index];

                                // Check if it's the last element
                                if (!(r == dim_m - 1 && c == dim_v - 1))
                                {
                                    s << ", ";
                                }
                            }
                        }
                        s << ");" << std::endl;
                    }
                    else if (dim_v > 1)
                    {
                        s << " = " << glsl_type << "(";
                        for (int c = 0; c < dim_v; ++c)
                        {
                            s << m[c];

                            // Check if it's the last element
                            if (!(c == dim_v - 1))
                            {
                                s << ", ";
                            }
                        }
                        s << ");" << std::endl;
                    }
                    else
                    {
                        s << " = " << m[0] << ";" << std::endl;
                    }
                    break;
                }
                default:
                    break;
                }
            }
            
            return s.str();
        }
#endif
        
    } // namespace timeline_vlk
} // namespace tl

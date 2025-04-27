// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "FL/Fl_Vk_Utils.H"
#include "FL/vk_enum_string_helper.h"

#include <tlTimelineVk/RenderPrivate.h>

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
#    include <libplacebo/shaders/colorspace.h>
#    include <libplacebo/shaders.h>
}
#endif

#include <array>
#include <list>

#define _USE_MATH_DEFINES
#include <math.h>

namespace
{
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
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    textures[1]->copy(
                        image->getData() + (w * h), textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) + (w2 * h2),
                        textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_422P_U8:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    textures[1]->copy(
                        image->getData() + (w * h), textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) + (w2 * h),
                        textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_444P_U8:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    textures[1]->copy(
                        image->getData() + (w * h), textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) + (w * h),
                        textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_420P_U16:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    textures[1]->copy(
                        image->getData() + (w * h) * 2, textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) * 2 + (w2 * h2) * 2,
                        textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_422P_U16:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    textures[1]->copy(
                        image->getData() + (w * h) * 2, textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) * 2 + (w2 * h) * 2,
                        textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_444P_U16:
            {
                if (3 == textures.size())
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
                break;
            }
            case image::PixelType::RGB_U8:
            {
                if (textures[0]->getInternalFormat() == VK_FORMAT_R8G8B8A8_UNORM)
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
                if (textures[0]->getInternalFormat() == VK_FORMAT_R16G16B16A16_UNORM)
                {
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const uint16_t* src = reinterpret_cast<uint16_t*>(image->getData());
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
                    textures[0]->copy(reinterpret_cast<uint8_t*>(dst.data()),
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
                if (textures[0]->getInternalFormat() == VK_FORMAT_R16G16B16A16_SFLOAT)
                {
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const uint16_t* src = reinterpret_cast<uint16_t*>(image->getData());
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
                    textures[0]->copy(reinterpret_cast<uint8_t*>(dst.data()),
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
                // \@bug:
                // \@todo: Fix -- Convert from GL_UNSIGNED_INT_10_10_10_2
                if (textures[0]->getInternalFormat() == VK_FORMAT_A2R10G10B10_UNORM_PACK32)
                {
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const uint32_t* src = reinterpret_cast<uint32_t*>(image->getData());
                    std::vector<uint32_t> dst(w * h);
                    uint32_t* out = dst.data();
                    for (std::size_t y = 0; y < h; ++y)
                    {
                        for (std::size_t x = 0; x < w; ++x)
                        {
                            const uint32_t pixel = src[y * w + x];
                            
                            // OpenGL packing: R (bits 0-9), G (10-19), B (20-29), A (30-31)
                            const uint32_t r = (pixel >> 0)  & 0x3FF;
                            const uint32_t g = (pixel >> 10) & 0x3FF;
                            const uint32_t b = (pixel >> 20) & 0x3FF;
                            const uint32_t a = (pixel >> 30) & 0x3;
                            
                            // Vulkan expects: A (bits 30-31), B (20-29), G (10-19), R (0-9)
                            const uint32_t vk_pixel = (a << 30) | (r << 20) | (g << 10) |
                                                      (b << 0);

                            reinterpret_cast<uint32_t*>(out)[y * w + x] = vk_pixel;
                        }
                    }
                    textures[0]->copy(reinterpret_cast<uint8_t*>(dst.data()),
                                      dst.size() * sizeof(uint32_t));
                }
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
            for (size_t i = 0; i < textures.size(); ++i)
            {
                // glDeleteTextures(1, &textures[i].id);
            }
        }

        OCIOLUTData::~OCIOLUTData()
        {
            for (size_t i = 0; i < textures.size(); ++i)
            {
                // glDeleteTextures(1, &textures[i].id);
            }
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
        }

        LibPlaceboData::~LibPlaceboData()
        {
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
            _p(new Private),
            ctx(context)
        {
        }

        Render::~Render()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;
            
            for (auto& [_, pipeline] : p.pipelines)
            {
                vkDestroyPipeline(device, pipeline, nullptr);
            }
            
            for (auto& [_, pipelineLayout] : p.pipelineLayouts)
            {
                vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
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
            VkCommandBuffer& cmd,
            std::shared_ptr<vlk::OffscreenBuffer> fbo,
            const uint32_t frameIndex,
            const math::Size2i& renderSize,
            const timeline::RenderOptions& renderOptions)
        {
            TLRENDER_P();
            
            p.cmd = cmd;
            p.fbo = fbo;
            p.frameIndex = frameIndex;

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

            // glEnable(GL_BLEND);
            // glBlendEquation(GL_FUNC_ADD);

            const math::Matrix4x4f transform;
            const image::Color4f color(0.F, 0.F, 0.F, 0.F);
            if (!p.shaders["rect"])
            {
                p.shaders["rect"] = vlk::Shader::create(
                    ctx, vertex2Source(), meshFragmentSource(), "rect");
                p.shaders["rect"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["rect"]->createUniform("color", color);
                p.shaders["rect"]->createDescriptorSets();
            }
            if (!p.shaders["mesh"])
            {
                p.shaders["mesh"] = vlk::Shader::create(
                    ctx, vertexSource(), meshFragmentSource(), "mesh");
                p.shaders["mesh"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["mesh"]->createUniform("color", color);
                p.shaders["mesh"]->createDescriptorSets();
            }
            if (!p.shaders["colorMesh"])
            {
                p.shaders["colorMesh"] = vlk::Shader::create(
                    ctx, colorMeshVertexSource(), colorMeshFragmentSource(), "colorMesh");
                p.shaders["colorMesh"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["colorMesh"]->createUniform("color", color);
                p.shaders["colorMesh"]->createDescriptorSets();
            }
            if (!p.shaders["text"])
            {
                p.shaders["text"] = vlk::Shader::create(
                    ctx, vertexSource(), textFragmentSource(), "text");
                p.shaders["text"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["text"]->addTexture("textureSampler");
                p.shaders["text"]->createUniform("color", color);
                p.shaders["text"]->createDescriptorSets();
            }
            if (!p.shaders["texture"])
            {
                p.shaders["texture"] = vlk::Shader::create(
                    ctx, vertexSource(), textureFragmentSource(), "texture");
                p.shaders["texture"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["texture"]->addTexture("textureSampler");
                p.shaders["texture"]->createUniform("color", color);
                p.shaders["texture"]->createDescriptorSets();
            }
            if (!p.shaders["image"])
            {
                p.shaders["image"] = vlk::Shader::create(
                    ctx, vertexSource(), imageFragmentSource(), "image");
                p.shaders["image"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                struct UBO
                {
                    image::Color4f color;
                    int pixelType;
                    int videoLevels;
                    math::Vector4f yuvCoefficients;
                    int imageChannels;
                    int mirrorX;
                    int mirrorY;
                };
                UBO ubo;
                p.shaders["image"]->createUniform("ubo", ubo);
                p.shaders["image"]->addTexture("textureSampler0");
                p.shaders["image"]->addTexture("textureSampler1");
                p.shaders["image"]->addTexture("textureSampler2");
                p.shaders["image"]->createDescriptorSets();
            }
            if (!p.shaders["wipe"])
            {
                p.shaders["wipe"] = vlk::Shader::create(
                    ctx, vertexSource(), meshFragmentSource(), "wipe");
                p.shaders["wipe"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["wipe"]->createUniform("color", color);
                p.shaders["wipe"]->createDescriptorSets();
            }
            if (!p.shaders["overlay"])
            {
                p.shaders["overlay"] = vlk::Shader::create(
                    ctx, vertexSource(), textureFragmentSource(), "overlay");
                p.shaders["overlay"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["overlay"]->addTexture("textureSampler");
                p.shaders["overlay"]->createUniform("color", color);
                p.shaders["overlay"]->createDescriptorSets();
            }
            if (!p.shaders["difference"])
            {
                p.shaders["difference"] = vlk::Shader::create(
                    ctx, vertexSource(), differenceFragmentSource(),
                    "difference");
                p.shaders["difference"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["difference"]->addTexture("textureSampler");
                p.shaders["difference"]->addTexture("textureSamplerB");
                p.shaders["difference"]->createDescriptorSets();
            }
            if (!p.shaders["dissolve"])
            {
                p.shaders["dissolve"] = vlk::Shader::create(
                    ctx, vertexSource(), textureFragmentSource(), "dissolve");
                p.shaders["dissolve"]->createUniform(
                    "transform.mvp", transform, vlk::kShaderVertex);
                p.shaders["dissolve"]->addTexture("textureSampler");
                p.shaders["dissolve"]->createUniform(
                    "color", image::Color4f(1.F, 1.F, 1.F));
                p.shaders["dissolve"]->createDescriptorSets();
            }
            _displayShader();

            if (!p.vbos["rect"] || p.vbos["rect"]->getSize() != 6)
            {
                p.vbos["rect"] = vlk::VBO::create(2 * 3, vlk::VBOType::Pos2_F32);
                p.vaos["rect"] = vlk::VAO::create(ctx);
            }
            
            if (!p.vbos["texture"] || p.vbos["texture"]->getSize() != 6)
            {
                p.vbos["texture"] =
                    vlk::VBO::create(2 * 3, vlk::VBOType::Pos2_F32_UV_U16);
                p.vaos["texture"] = vlk::VAO::create(ctx);
            }
            if (!p.vbos["image"] || p.vbos["image"]->getSize() != 6)
            {
                p.vbos["image"] =
                    vlk::VBO::create(2 * 3, vlk::VBOType::Pos2_F32_UV_U16);
                p.vaos["image"] = vlk::VAO::create(ctx);
            }
            if (!p.vbos["wipe"] || p.vbos["wipe"]->getSize() != 3)
            {
                p.vbos["wipe"] = vlk::VBO::create(1 * 3, vlk::VBOType::Pos2_F32);
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
                    0.F, static_cast<float>(renderSize.w),
                    0.F, static_cast<float>(renderSize.h), -1.F, 1.F));
        }

        void Render::end()
        {
            TLRENDER_P();

            //! \bug Should these be reset periodically?
            // p.glyphIDs.clear();
            // p.vbos["mesh"].reset();
            // p.vaos["mesh"].reset();
            // p.vbos["text"].reset();
            // p.vaos["text"].reset();
            
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
                        }
                        average.time /= p.stats.size();
                        average.rects /= p.stats.size();
                        average.meshes /= p.stats.size();
                        average.meshTriangles /= p.stats.size();
                        average.text /= p.stats.size();
                        average.textTriangles /= p.stats.size();
                        average.textures /= p.stats.size();
                        average.images /= p.stats.size();
                    }

                    context->log(
                        string::Format("tl::timeline::GLRender {0}").arg(this),
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
                            "    Glyph texture atlas: {8}%\n"
                            "    Glyph IDs: {9}")
                            .arg(average.time)
                            .arg(average.rects)
                            .arg(average.meshes)
                            .arg(average.meshTriangles)
                            .arg(average.text)
                            .arg(average.textTriangles)
                            .arg(average.textures)
                            .arg(average.images)
                            .arg(p.glyphTextureAtlas->getPercentageUsed())
                            .arg(p.glyphIDs.size()));
                }
            }
        }

        VkCommandBuffer Render::getCommandBuffer() const
        {
            return _p->cmd;
        }
        
        uint32_t        Render::getFrameIndex() const
        {
            return _p->frameIndex;
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
            TLRENDER_P();
            p.viewport = value;
            // glViewport(
            //     value.x(), p.renderSize.h - value.h() - value.y(), value.w(),
            //     value.h());
        }

        void Render::clearViewport(const image::Color4f& value)
        {
            TLRENDER_P();
            
            // Clear yellow (assuming this clear value is for the FBO)
            VkClearValue clearValues[2];
            clearValues[0].color = { value.r, value.g, value.b, value.a }; // Clear color for the FBO
            clearValues[1].depthStencil = { 1.F, 0 };

            VkRenderPassBeginInfo rpBegin{};
            rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rpBegin.renderPass = p.fbo->getRenderPass(); // Use the FBO's render pass
            // Use the FBO's framebuffer for THIS frame if using per-frame FBO attachments,
            // otherwise use the static one if vk.buffer->getFramebuffer() is static.
            rpBegin.framebuffer = p.fbo->getFramebuffer(/*currentFrameIndex*/); // Pass frame index if needed
            rpBegin.renderArea.offset = {0, 0}; // Assuming render area starts at 0,0
            rpBegin.renderArea.extent = p.fbo->getExtent(); // Use FBO extent
            rpBegin.clearValueCount = 1 + static_cast<uint16_t>(p.fbo->hasDepth() || p.fbo->hasStencil());
            rpBegin.pClearValues = clearValues;

            // Begin the first render pass instance within the single command buffer
            vkCmdBeginRenderPass(p.cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);    
            vkCmdEndRenderPass(p.cmd);
        }

        bool Render::getClipRectEnabled() const
        {
            return _p->clipRectEnabled;
        }

        void Render::setClipRectEnabled(bool value)
        {
            TLRENDER_P();
            p.clipRectEnabled = value;
            if (p.clipRectEnabled)
            {
                // glEnable(GL_SCISSOR_TEST);
            }
            else
            {
                // glDisable(GL_SCISSOR_TEST);
            }
        }

        math::Box2i Render::getClipRect() const
        {
            return _p->clipRect;
        }

        void Render::setClipRect(const math::Box2i& value)
        {
            TLRENDER_P();
            p.clipRect = value;
            if (value.w() > 0 && value.h() > 0)
            {
                // glScissor(
                //     value.x(), p.renderSize.h - value.h() - value.y(),
                //     value.w(), value.h());
            }
        }

        math::Matrix4x4f Render::getTransform() const
        {
            return _p->transform;
        }

        void Render::setTransform(const math::Matrix4x4f& value)
        {
            TLRENDER_P();
            p.transform = value;
            for (auto i : p.shaders)
            {
                i.second->bind(p.frameIndex);
                i.second->setUniform(
                    "transform.mvp", value, vlk::kShaderVertex);
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


                
                auto texture = vlk::Texture::create(ctx, VK_IMAGE_TYPE_3D, width, height,
                                                    depth, imageFormat, samplerName);
                texture->copy(reinterpret_cast<const uint8_t*>(values), width * height * depth * channels * sizeof(float));
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
                    if (height == 0) height = 1;
                    break;
                case OCIO::GpuShaderDesc::TEXTURE_2D:
                    imageType = VK_IMAGE_TYPE_2D;
                    break;
                default:
                    throw std::runtime_error("Unknown OCIO image type");
                }

                auto texture = vlk::Texture::create(ctx, imageType, width, height,
                                                    0, imageFormat, samplerName);
                texture->copy(reinterpret_cast<const uint8_t*>(values), width * height * channels * sizeof(float));
                textures.push_back(texture);
            }
        }
#endif // TLRENDER_OCIO

#if defined(TLRENDER_LIBPLACEBO)
        void Render::_addTextures(
            std::vector<std::shared_ptr<vlk::Texture> >& textures, const pl_shader_res* res)
        {
            TLRENDER_P();

            // Create 3D textures.
            // glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            // glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
            for (unsigned i = 0; i < res->num_descriptors; ++i)
            {
                const pl_shader_desc* sd = &res->descriptors[i];
                switch (sd->desc.type)
                {
                case PL_DESC_STORAGE_IMG:
                    throw "Unimplemented img";
                case PL_DESC_SAMPLED_TEX:
                {
                    pl_tex tex = reinterpret_cast<pl_tex>(sd->binding.object);
                    pl_fmt fmt = tex->params.format;

                    int dims = pl_tex_params_dimension(tex->params);
                    assert(dims >= 1 && dims <= 3);

                    const char* textureName = sd->desc.name;
                    const char* samplerName = sd->desc.name;
                    const int channels = fmt->num_components;

                    // Defaults
                    unsigned textureId = 0;
                    int size = fmt->internal_size / fmt->num_components;

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
                    auto texture = vlk::Texture::create(ctx, imageType, width, height, depth,
                                                        imageFormat, samplerName);
                    texture->copy(reinterpret_cast<const uint8_t*>(values), width * height * depth * fmt->internal_size);
                    textures.push_back(texture);
                    break;
                }
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
                p.ocioData->icsDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
                if (!p.ocioData->icsDesc)
                {
                    p.ocioData.reset();
                    throw std::runtime_error("Cannot create OCIO transform");
                }
                if (!p.ocioOptions.input.empty())
                {
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
            if (value == p.hdrOptions)
                return;

            p.hdrOptions = value;

#if defined(TLRENDER_LIBPLACEBO)
            p.placeboData.reset(new LibPlaceboData);
#endif // TLRENDER_LIBPLACEBO

            p.shaders["display"].reset();
            _displayShader();
        }

        void Render::_displayShader()
        {
            TLRENDER_P();

            if (!p.shaders["display"])
            {
                if (p.pipelines["display"])
                    p.pipelines["display"] = VK_NULL_HANDLE;
                if (p.pipelineLayouts["display"])
                    p.pipelineLayouts["display"] = VK_NULL_HANDLE;
                
                std::string ocioICSDef;
                std::string ocioICS;
                std::string ocioDef;
                std::string ocio;
                std::string lutDef;
                std::string lut;
                std::string toneMapDef;
                std::string toneMap;

#if defined(TLRENDER_OCIO)
                if (p.ocioData && p.ocioData->icsDesc)
                {
                    ocioICSDef = p.ocioData->icsDesc->getShaderText();
                    ocioICS = "outColor = ocioICSFunc(outColor);";
                }
                if (p.ocioData && p.ocioData->shaderDesc)
                {
                    ocioDef = p.ocioData->shaderDesc->getShaderText();
                    ocio = "outColor = ocioDisplayFunc(outColor);";
                }
                if (p.lutData && p.lutData->shaderDesc)
                {
                    lutDef = p.lutData->shaderDesc->getShaderText();
                    lut = "outColor = lutFunc(outColor);";
                }
#endif // TLRENDER_OCIO
#if defined(TLRENDER_LIBPLACEBO)
                if (p.placeboData)
                {
                    pl_shader_params shader_params;
                    memset(&shader_params, 0, sizeof(pl_shader_params));

                    shader_params.id = 1;
                    shader_params.gpu = p.placeboData->gpu;
                    shader_params.dynamic_constants = false;

                    pl_shader shader =
                        pl_shader_alloc(p.placeboData->log, &shader_params);
                    if (!shader)
                    {
                        throw std::runtime_error("pl_shader_alloc failed!");
                    }

                    pl_color_map_params cmap;
                    memset(&cmap, 0, sizeof(pl_color_map_params));

                    // defaults, generates LUTs if state is set.
                    cmap.gamut_mapping = &pl_gamut_map_perceptual;

                    // PL_GAMUT_MAP_CONSTANTS is defined in wrong order for C++
                    cmap.gamut_constants = {0};
                    cmap.gamut_constants.perceptual_deadzone = 0.3F;
                    cmap.gamut_constants.perceptual_strength = 0.8F;
                    cmap.gamut_constants.colorimetric_gamma = 1.80f;
                    cmap.gamut_constants.softclip_knee = 0.70f;
                    cmap.gamut_constants.softclip_desat = 0.35f;

                    cmap.tone_mapping_function = nullptr;

                    cmap.tone_constants = {0};
                    cmap.tone_constants.knee_adaptation = 0.4f;
                    cmap.tone_constants.knee_minimum = 0.1f;
                    cmap.tone_constants.knee_maximum = 0.8f;
                    cmap.tone_constants.knee_default = 0.4f;
                    cmap.tone_constants.knee_offset = 1.0f;

                    cmap.tone_constants.slope_tuning = 1.5f;
                    cmap.tone_constants.slope_offset = 0.2f;

                    cmap.tone_constants.spline_contrast = 0.5f;

                    cmap.tone_constants.reinhard_contrast = 0.5f;
                    cmap.tone_constants.linear_knee = 0.3f;

                    cmap.tone_constants.exposure = 1.0f;

                    cmap.contrast_smoothness = 3.5f;

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
                        else if (ctx.colorSpace == VK_COLOR_SPACE_DOLBYVISION_EXT)
                        {
                            // \@todo:  How to handle this?
                            // PL_COLOR_TRC_DOLBYVISION does not exist.
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
                            cmap.tone_mapping_function = &pl_tone_map_auto;
                        }
                    }
                    else
                    {
                        cmap.lut3d_size[0] = 48;
                        cmap.lut3d_size[1] = 32;
                        cmap.lut3d_size[2] = 256;
                        cmap.lut_size = 256;
                        cmap.visualize_rect.x0 = 0;
                        cmap.visualize_rect.y0 = 0;
                        cmap.visualize_rect.x1 = 1;
                        cmap.visualize_rect.y1 = 1;
                        cmap.contrast_smoothness = 3.5f;

                        dst_colorspace.primaries = PL_COLOR_PRIM_BT_709;
                        dst_colorspace.transfer = PL_COLOR_TRC_BT_1886;

                        if (hasHDR)
                        {
                            switch (p.hdrOptions.algorithm)
                            {
                            case timeline::HDRTonemapAlgorithm::Clip:
                                break;
                            case timeline::HDRTonemapAlgorithm::ST2094_10:
                                cmap.tone_mapping_function = &pl_tone_map_st2094_10;
                                break;
                            case timeline::HDRTonemapAlgorithm::BT2390:
                                cmap.tone_mapping_function = &pl_tone_map_bt2390;
                                break;
                            case timeline::HDRTonemapAlgorithm::BT2446A:
                                cmap.tone_mapping_function = &pl_tone_map_bt2446a;
                                break;
                            case timeline::HDRTonemapAlgorithm::Spline:
                                cmap.tone_mapping_function = &pl_tone_map_spline;
                                break;
                            case timeline::HDRTonemapAlgorithm::Reinhard:
                                cmap.tone_mapping_function = &pl_tone_map_reinhard;
                                break;
                            case timeline::HDRTonemapAlgorithm::Mobius:
                                cmap.tone_mapping_function = &pl_tone_map_mobius;
                                break;
                            case timeline::HDRTonemapAlgorithm::Hable:
                                cmap.tone_mapping_function = &pl_tone_map_hable;
                                break;
                            case timeline::HDRTonemapAlgorithm::Gamma:
                                cmap.tone_mapping_function = &pl_tone_map_gamma;
                                break;
                            case timeline::HDRTonemapAlgorithm::Linear:
                                cmap.tone_mapping_function = &pl_tone_map_linear;
                                break;
                            case timeline::HDRTonemapAlgorithm::LinearLight:
                                cmap.tone_mapping_function =
                                    &pl_tone_map_linear_light;
                                break;
                            case timeline::HDRTonemapAlgorithm::ST2094_40:
                            default:
                                cmap.tone_mapping_function = &pl_tone_map_st2094_40;
                                break;
                            }
                        }
                        else
                        {
                            cmap.tone_mapping_function = nullptr;
                        }
                    }

                    pl_color_space_infer(&dst_colorspace);

                    pl_color_map_args color_map_args;
                    memset(&color_map_args, 0, sizeof(pl_color_map_args));

                    color_map_args.src = src_colorspace;
                    color_map_args.dst = dst_colorspace;
                    color_map_args.prelinearized = false;

                    pl_shader_obj state = NULL;
                    color_map_args.state = &state;

                    pl_shader_color_map_ex(shader, &cmap, &color_map_args);

                    const pl_shader_res* res = pl_shader_finalize(shader);
                    if (!res)
                    {
                        pl_shader_free(&shader);
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
                    int i = 0;
                    for (i = 0; i < res->num_descriptors; i++)
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

                            s << "layout(binding=" << (i + 1) << ") uniform "
                              << prefix << type << " " << desc->name << ";"
                              << std::endl;
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
                    s << "layout(set = 0, binding = " << i + 1 << ", std140) uniform Placebo {\n";
                    for (i = 0; i < res->num_variables; ++i)
                    {
                        const struct pl_shader_var shader_var = res->variables[i];
                        const struct pl_var var = shader_var.var;
                        std::string glsl_type = pl_var_glsl_type_name(var);
                        s << "const " << glsl_type << " " << var.name << ";" << std::endl;
                        // else
                        // {
                        //     int dim_v = var.dim_v;
                        //     int dim_m = var.dim_m;
                        //     switch (var.type)
                        //     {
                        //     case PL_VAR_SINT:
                        //     {
                        //         int* m = (int*)shader_var.data;
                        //         s << " = " << m[0] << ";" << std::endl;
                        //         break;
                        //     }
                        //     case PL_VAR_UINT:
                        //     {
                        //         unsigned* m = (unsigned*)shader_var.data;
                        //         s << " = " << m[0] << ";" << std::endl;
                        //         break;
                        //     }
                        //     case PL_VAR_FLOAT:
                        //     {
                        //         float* m = (float*)shader_var.data;
                        //         if (dim_m > 1 && dim_v > 1)
                        //         {
                        //             s << " = " << glsl_type << "(";
                        //             for (int c = 0; c < dim_v; ++c)
                        //             {
                        //                 for (int r = 0; r < dim_m; ++r)
                        //                 {
                        //                     int index = c * dim_m + r;
                        //                     s << m[index];

                        //                     // Check if it's the last element
                        //                     if (!(r == dim_m - 1 && c == dim_v - 1))
                        //                     {
                        //                         s << ", ";
                        //                     }
                        //                 }
                        //             }
                        //             s << ");" << std::endl;
                        //         }
                        //         else if (dim_v > 1)
                        //         {
                        //             s << " = " << glsl_type << "(";
                        //             for (int c = 0; c < dim_v; ++c)
                        //             {
                        //                 s << m[c];

                        //                 // Check if it's the last element
                        //                 if (!(c == dim_v - 1))
                        //                 {
                        //                     s << ", ";
                        //                 }
                        //             }
                        //             s << ");" << std::endl;
                        //         }
                        //         else
                        //         {
                        //             s << " = " << m[0] << ";" << std::endl;
                        //         }
                        //         break;
                        //     }
                        //     default:
                        //         break;
                        //     }
                        // }
                    }
                    s << "} uboPlacebo;\n";

                    s << std::endl
                      << "//" << std::endl
                      << "// Constants" << std::endl
                      << "//" << std::endl
                      << std::endl;
                    for (int i = 0; i < res->num_constants; ++i)
                    {
                        // s << "layout(constant_id=" << i << ") ";
                        const struct pl_shader_const constant = res->constants[i];
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
                              << *(reinterpret_cast<const float*>(constant.data));
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
                        pl_shader_free(&shader);
                        return;
                    }
                    toneMap = "outColor = ";
                    toneMap += res->name;
                    toneMap += "(tmp);\n";

                    pl_shader_free(&shader);
                }
#endif
                const std::string source = displayFragmentSource(
                    ocioICSDef, ocioICS, ocioDef, ocio, lutDef, lut,
                    p.lutOptions.order, toneMapDef, toneMap);
                if (auto context = _context.lock())
                {
                    context->log("tl::vlk::GLRender", "Creating display shader");
                }
                p.shaders["display"] =
                    vlk::Shader::create(ctx, vertexSource(), source, "display");
                p.shaders["display"]->createUniform(
                    "transform.mvp", p.transform, vlk::kShaderVertex);
                p.shaders["display"]->addFBO("textureSampler");
                timeline::DisplayOptions display;
                p.shaders["display"]->createUniform("uboLevels", display.levels);
                p.shaders["display"]->createUniform("uboEXRDisplay", display.exrDisplay);
                p.shaders["display"]->createUniform("uboNormalize", display.normalize);
                p.shaders["display"]->createUniform("uboColor", display.color);
                struct UBO
                {
                    int   channels = 0;
                    int   mirrorX = false;
                    int   mirrorY = false;
                    float softClip = 0.F;
                    int   videoLevels = 0;
                    bool  invalidValues = false;
                };
                UBO ubo;
                p.shaders["display"]->createUniform("ubo", ubo);
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
#if defined(TLRENDER_LIBPLACEBO)
                if (p.placeboData)
                {
                    for (const auto& texture : p.placeboData->textures)
                    {
                        p.shaders["display"]->addTexture(texture->getName());
                    }
                }
#endif
                p.shaders["display"]->createDescriptorSets();
                //p.shaders["display"]->debug();
            }
        }
    } // namespace timeline_vlk
} // namespace tl

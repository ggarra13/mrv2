// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineGL/RenderPrivate.h>

#include <tlGL/GL.h>
#include <tlGL/Mesh.h>
#include <tlGL/Util.h>

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

namespace tl
{
    namespace timeline_gl
    {
        namespace
        {
            const int pboSizeMin = 1024;
        }

        std::vector<std::shared_ptr<gl::Texture> > getTextures(
            const image::Info& info, const timeline::ImageFilters& imageFilters,
            size_t offset)
        {
            std::vector<std::shared_ptr<gl::Texture> > out;
            gl::TextureOptions options;
            options.filters = imageFilters;
            options.pbo =
                info.size.w >= pboSizeMin || info.size.h >= pboSizeMin;
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h / 2),
                    image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            case image::PixelType::YUV_422P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h),
                    image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            case image::PixelType::YUV_444P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            case image::PixelType::YUV_420P_U10:
            case image::PixelType::YUV_420P_U12:
            case image::PixelType::YUV_420P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h / 2),
                    image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            case image::PixelType::YUV_422P_U10:
            case image::PixelType::YUV_422P_U12:
            case image::PixelType::YUV_422P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h),
                    image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            case image::PixelType::YUV_444P_U10:
            case image::PixelType::YUV_444P_U12:
            case image::PixelType::YUV_444P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            default:
            {
                auto texture = gl::Texture::create(info, options);
                out.push_back(texture);
                break;
            }
            }
            return out;
        }

        void copyTextures(
            const std::shared_ptr<image::Image>& image,
            const std::vector<std::shared_ptr<gl::Texture> >& textures,
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
            case image::PixelType::YUV_420P_U10:
            case image::PixelType::YUV_422P_U10:
            case image::PixelType::YUV_444P_U10:
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
            default:
                if (1 == textures.size())
                {
                    textures[0]->copy(image);
                }
                break;
            }
        }

        void setActiveTextures(
            const image::Info& info,
            const std::vector<std::shared_ptr<gl::Texture> >& textures,
            size_t offset)
        {
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_422P_U8:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_444P_U8:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_420P_U10:
            case image::PixelType::YUV_420P_U12:
            case image::PixelType::YUV_420P_U16:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_422P_U10:
            case image::PixelType::YUV_422P_U12:
            case image::PixelType::YUV_422P_U16:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_444P_U10:
            case image::PixelType::YUV_444P_U12:
            case image::PixelType::YUV_444P_U16:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            default:
                if (1 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glActiveTexture(
                        static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
                break;
            }
        }

#if defined(TLRENDER_OCIO) || defined(TLRENDER_LIBPLACEBO)
        OCIOTexture::OCIOTexture(
            unsigned id, std::string name, std::string sampler, unsigned type) :
            id(id),
            name(name),
            sampler(sampler),
            type(type)
        {
        }
#endif

#if defined(TLRENDER_OCIO)
        OCIOData::~OCIOData()
        {
            for (size_t i = 0; i < textures.size(); ++i)
            {
                glDeleteTextures(1, &textures[i].id);
            }
        }

        OCIOLUTData::~OCIOLUTData()
        {
            for (size_t i = 0; i < textures.size(); ++i)
            {
                glDeleteTextures(1, &textures[i].id);
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

            state = nullptr;
        }

        LibPlaceboData::~LibPlaceboData()
        {
            for (size_t i = 0; i < textures.size(); ++i)
            {
                glDeleteTextures(1, &textures[i].id);
            }
            if (state) {
                pl_shader_obj_destroy(&state);
                state = NULL;
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

            p.glyphTextureAtlas = gl::TextureAtlas::create(
                1, 4096, image::PixelType::L_U8, timeline::ImageFilter::Linear);

            p.logTimer = std::chrono::steady_clock::now();
        }

        Render::Render() :
            _p(new Private)
        {
        }

        Render::~Render() {}

        std::shared_ptr<Render> Render::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<TextureCache>& textureCache)
        {
            auto out = std::shared_ptr<Render>(new Render);
            out->_init(context, textureCache);
            return out;
        }

        const std::shared_ptr<TextureCache>& Render::getTextureCache() const
        {
            return _p->textureCache;
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

            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);

            if (!p.shaders["rect"])
            {
                p.shaders["rect"] =
                    gl::Shader::create(vertexSource(), meshFragmentSource());
            }
            if (!p.shaders["mesh"])
            {
                p.shaders["mesh"] =
                    gl::Shader::create(vertexSource(), meshFragmentSource());
            }
            if (!p.shaders["colorMesh"])
            {
                p.shaders["colorMesh"] = gl::Shader::create(
                    colorMeshVertexSource(), colorMeshFragmentSource());
            }
            if (!p.shaders["text"])
            {
                p.shaders["text"] =
                    gl::Shader::create(vertexSource(), textFragmentSource());
            }
            if (!p.shaders["texture"])
            {
                p.shaders["texture"] =
                    gl::Shader::create(vertexSource(), textureFragmentSource());
            }
            if (!p.shaders["image"])
            {
                p.shaders["image"] =
                    gl::Shader::create(vertexSource(), imageFragmentSource());
            }
            if (!p.shaders["wipe"])
            {
                p.shaders["wipe"] =
                    gl::Shader::create(vertexSource(), meshFragmentSource());
            }
            if (!p.shaders["overlay"])
            {
                p.shaders["overlay"] =
                    gl::Shader::create(vertexSource(), textureFragmentSource());
            }
            if (!p.shaders["difference"])
            {
                p.shaders["difference"] = gl::Shader::create(
                    vertexSource(), differenceFragmentSource());
            }
            if (!p.shaders["dissolve"])
            {
                p.shaders["dissolve"] =
                    gl::Shader::create(vertexSource(), textureFragmentSource());
            }
            _displayShader();

            p.vbos["rect"] = gl::VBO::create(2 * 3, gl::VBOType::Pos2_F32);
            p.vaos["rect"] = gl::VAO::create(
                p.vbos["rect"]->getType(), p.vbos["rect"]->getID());
            p.vbos["texture"] =
                gl::VBO::create(2 * 3, gl::VBOType::Pos2_F32_UV_U16);
            p.vaos["texture"] = gl::VAO::create(
                p.vbos["texture"]->getType(), p.vbos["texture"]->getID());
            p.vbos["image"] =
                gl::VBO::create(2 * 3, gl::VBOType::Pos2_F32_UV_U16);
            p.vaos["image"] = gl::VAO::create(
                p.vbos["image"]->getType(), p.vbos["image"]->getID());
            p.vbos["wipe"] = gl::VBO::create(1 * 3, gl::VBOType::Pos2_F32);
            p.vaos["wipe"] = gl::VAO::create(
                p.vbos["wipe"]->getType(), p.vbos["wipe"]->getID());
            p.vbos["video"] =
                gl::VBO::create(2 * 3, gl::VBOType::Pos2_F32_UV_U16);
            p.vaos["video"] = gl::VAO::create(
                p.vbos["video"]->getType(), p.vbos["video"]->getID());

            setViewport(math::Box2i(0, 0, renderSize.w, renderSize.h));
            if (renderOptions.clear)
            {
                clearViewport(renderOptions.clearColor);
            }
            setTransform(math::ortho(
                0.F, static_cast<float>(renderSize.w),
                static_cast<float>(renderSize.h), 0.F, -1.F, 1.F));
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
                        string::Format("\n"
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
        
        void Render::setMonitorMinNits(float value)
        {
            _p->monitorMinNits = value;
        }
        
        void Render::setMonitorMaxNits(float value)
        {
            _p->monitorMaxNits = value;
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
            glViewport(
                value.x(), p.renderSize.h - value.h() - value.y(), value.w(),
                value.h());
        }

        void Render::clearViewport(const image::Color4f& value)
        {
            glClearColor(value.r, value.g, value.b, value.a);
            glClear(GL_COLOR_BUFFER_BIT);
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
                glEnable(GL_SCISSOR_TEST);
            }
            else
            {
                glDisable(GL_SCISSOR_TEST);
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
                glScissor(
                    value.x(), p.renderSize.h - value.h() - value.y(),
                    value.w(), value.h());
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
                i.second->bind();
                i.second->setUniform("transform.mvp", value);
            }
        }

        namespace
        {
            void setTextureParameters(GLenum textureType, GLenum interpolation)
            {
                glTexParameteri(
                    textureType, GL_TEXTURE_MIN_FILTER, interpolation);
                glTexParameteri(
                    textureType, GL_TEXTURE_MAG_FILTER, interpolation);
                glTexParameteri(
                    textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(
                    textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(
                    textureType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            }

#if defined(TLRENDER_OCIO)

            void setTextureParameters(
                GLenum textureType, OCIO::Interpolation interpolation)
            {
                setTextureParameters(
                    textureType, OCIO::INTERP_NEAREST == interpolation
                                     ? GL_NEAREST
                                     : GL_LINEAR);
            }

            void addGPUTextures(
                std::vector<tl::timeline_gl::OCIOTexture>& textures,
                const OCIO::GpuShaderDescRcPtr& shaderDesc)
            {
                // Create 3D textures.
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
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

                    unsigned textureId = 0;
                    glGenTextures(1, &textureId);
                    glBindTexture(GL_TEXTURE_3D, textureId);
                    setTextureParameters(GL_TEXTURE_3D, interpolation);
                    glTexImage3D(
                        GL_TEXTURE_3D, 0, GL_RGB32F, edgelen, edgelen, edgelen,
                        0, GL_RGB, GL_FLOAT, values);
                    textures.push_back(OCIOTexture(
                        textureId, textureName, samplerName, GL_TEXTURE_3D));
                }

                // Create 1D textures.
                const unsigned numTextures = shaderDesc->getNumTextures();
                for (unsigned i = 0; i < numTextures; ++i)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned width = 0;
                    unsigned height = 0;
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

                    unsigned textureId = 0;
                    GLint internalformat = GL_RGB32F;
                    GLenum format = GL_RGB;
                    if (OCIO::GpuShaderCreator::TEXTURE_RED_CHANNEL == channel)
                    {
                        internalformat = GL_R32F;
                        format = GL_RED;
                    }
                    glGenTextures(1, &textureId);
                    switch (dimensions)
                    {
                    case OCIO::GpuShaderDesc::TEXTURE_1D:
                        glBindTexture(GL_TEXTURE_1D, textureId);
                        setTextureParameters(GL_TEXTURE_1D, interpolation);
                        glTexImage1D(
                            GL_TEXTURE_1D, 0, internalformat, width, 0, format,
                            GL_FLOAT, values);
                        break;
                    case OCIO::GpuShaderDesc::TEXTURE_2D:
                        glBindTexture(GL_TEXTURE_2D, textureId);
                        setTextureParameters(GL_TEXTURE_2D, interpolation);
                        glTexImage2D(
                            GL_TEXTURE_2D, 0, internalformat, width, height, 0,
                            format, GL_FLOAT, values);
                        break;
                    }
                    textures.push_back(OCIOTexture(
                        textureId, textureName, samplerName,
                        (height > 1) ? GL_TEXTURE_2D : GL_TEXTURE_1D));
                }
            }
#endif // TLRENDER_OCIO

#if defined(TLRENDER_LIBPLACEBO)
            void addGPUTextures(
                std::vector<tl::timeline_gl::OCIOTexture>& textures,
                const pl_shader_res* res)
            {
                // Create 3D textures.
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
                for (unsigned i = 0; i < res->num_descriptors; ++i)
                {
                    const pl_shader_desc* sd = &res->descriptors[i];
                    switch (sd->desc.type)
                    {
                    case PL_DESC_STORAGE_IMG:
                        throw "Unimplemented img";
                    case PL_DESC_SAMPLED_TEX:
                    {
                        pl_tex tex =
                            reinterpret_cast<pl_tex>(sd->binding.object);
                        pl_fmt fmt = tex->params.format;

                        int dims = pl_tex_params_dimension(tex->params);
                        assert(dims >= 1 && dims <= 3);

                        const char* textureName = sd->desc.name;
                        const char* samplerName = sd->desc.name;

                        // std::cerr << "fmt->type=" << fmt->type << std::endl;
                        // std::cerr << "fmt->num_components=" <<
                        // fmt->component_depth << std::endl; for (int i = 0; i
                        // < fmt->num_components; ++i)
                        // {
                        //     std::cerr << "fmt->component_depth[" << i << "]="
                        //     << fmt->component_depth[i] << std::endl;
                        // }
                        // std::cerr << "fmt->internal_size=" <<
                        // fmt->internal_size << std::endl;

                        // Defaults
                        unsigned textureId = 0;
                        GLint internalFormat = GL_RGB32F;
                        GLint type = GL_FLOAT;
                        GLenum format = GL_RGB;
                        int size = fmt->internal_size / fmt->num_components;

                        switch (fmt->type)
                        {
                        case PL_FMT_FLOAT:
                            if (size == 2)
                                type = GL_HALF_FLOAT;
                            else
                                type = GL_FLOAT;
                            break;
                        case PL_FMT_UINT:
                        case PL_FMT_UNORM:
                            switch (size)
                            {
                            case 4:
                                type = GL_UNSIGNED_INT;
                                break;
                            case 2:
                                type = GL_UNSIGNED_SHORT;
                                break;
                            default:
                                type = GL_UNSIGNED_BYTE;
                            }
                            break;
                        case PL_FMT_SNORM:
                        case PL_FMT_SINT:
                            switch (size)
                            {
                            case 4:
                                type = GL_INT;
                                break;
                            case 2:
                                type = GL_SHORT;
                                break;
                            default:
                                type = GL_BYTE;
                            }
                            break;
                        default:
                            type = GL_BYTE;
                            break;
                        }

                        if (fmt->num_components == 1)
                        {
                            format = GL_RED;

                            switch (fmt->type)
                            {
                            case PL_FMT_FLOAT:
                                if (size == 2)
                                    internalFormat = GL_R16F;
                                else
                                    internalFormat = GL_R32F;
                                break;
                            case PL_FMT_SNORM:
                                switch (size)
                                {
                                case 2:
                                    internalFormat = GL_R16_SNORM;
                                    break;
                                default:
                                    internalFormat = GL_R8_SNORM;
                                    break;
                                }
                                break;
                            case PL_FMT_UINT:
                                switch (size)
                                {
                                case 4:
                                    internalFormat = GL_R32UI;
                                    break;
                                case 2:
                                    internalFormat = GL_R16UI;
                                    break;
                                default:
                                    internalFormat = GL_R8UI;
                                    break;
                                }
                                break;
                            case PL_FMT_SINT:
                                switch (size)
                                {
                                case 4:
                                    internalFormat = GL_R32I;
                                    break;
                                case 2:
                                    internalFormat = GL_R16I;
                                    break;
                                default:
                                    internalFormat = GL_R8I;
                                    break;
                                }
                                break;
                            case PL_FMT_UNORM:
                                switch (size)
                                {
                                case 2:
                                    internalFormat = GL_R16;
                                    break;
                                default:
                                    internalFormat = GL_R8;
                                    break;
                                }
                                break;
                            default:
                                internalFormat = GL_R8;
                                break;
                            }
                        }
                        else if (fmt->num_components == 2)
                        {
                            format = GL_RG;
                            switch (fmt->type)
                            {
                            case PL_FMT_FLOAT:
                                if (size == 2)
                                    internalFormat = GL_RG16F;
                                else
                                    internalFormat = GL_RG32F;
                                break;
                            case PL_FMT_SNORM:
                                switch (size)
                                {
                                case 2:
                                    internalFormat = GL_RG16_SNORM;
                                    break;
                                default:
                                    internalFormat = GL_RG8_SNORM;
                                    break;
                                }
                                break;
                            case PL_FMT_UINT:
                                switch (size)
                                {
                                case 4:
                                    internalFormat = GL_RG32UI;
                                    break;
                                case 2:
                                    internalFormat = GL_RG16UI;
                                    break;
                                default:
                                    internalFormat = GL_RG8UI;
                                    break;
                                }
                                break;
                            case PL_FMT_SINT:
                                switch (size)
                                {
                                case 4:
                                    internalFormat = GL_RG32I;
                                    break;
                                case 2:
                                    internalFormat = GL_RG16I;
                                    break;
                                default:
                                    internalFormat = GL_RG8I;
                                    break;
                                }
                                break;
                            case PL_FMT_UNORM:
                                switch (size)
                                {
                                case 2:
                                    internalFormat = GL_RG16;
                                    break;
                                default:
                                    internalFormat = GL_RG8;
                                    break;
                                }
                                break;
                            default:
                                internalFormat = GL_RG8;
                                break;
                            }
                        }
                        else if (fmt->num_components == 3)
                        {
                            format = GL_RGB;
                            switch (fmt->type)
                            {
                            case PL_FMT_FLOAT:
                                if (size == 2)
                                    internalFormat = GL_RGB16F;
                                else
                                    internalFormat = GL_RGB32F;
                                break;
                            case PL_FMT_SNORM:
                                switch (size)
                                {
                                case 2:
                                    internalFormat = GL_RGB16_SNORM;
                                    break;
                                default:
                                    internalFormat = GL_RGB8_SNORM;
                                    break;
                                }
                                break;
                            case PL_FMT_UINT:
                                switch (size)
                                {
                                case 4:
                                    internalFormat = GL_RGB32UI;
                                    break;
                                case 2:
                                    internalFormat = GL_RGB16UI;
                                    break;
                                default:
                                    internalFormat = GL_RGB8UI;
                                    break;
                                }
                                break;
                            case PL_FMT_SINT:
                                switch (size)
                                {
                                case 4:
                                    internalFormat = GL_RGB32I;
                                    break;
                                case 2:
                                    internalFormat = GL_RGB16I;
                                    break;
                                default:
                                    internalFormat = GL_RGB8I;
                                    break;
                                }
                                break;
                            case PL_FMT_UNORM:
                                if (size == 2)
                                    internalFormat = GL_RGB16;
                                else
                                    internalFormat = GL_RGB8;
                                break;
                            default:
                                internalFormat = GL_RGB8;
                                break;
                            }
                        }
                        else if (fmt->num_components == 4)
                        {
                            format = GL_RGBA;
                            switch (fmt->type)
                            {
                            case PL_FMT_FLOAT:
                                if (size == 2)
                                    internalFormat = GL_RGBA16F;
                                else
                                    internalFormat = GL_RGBA32F;
                                break;
                            case PL_FMT_SNORM:
                                switch (size)
                                {
                                case 2:
                                    internalFormat = GL_RGBA16_SNORM;
                                    break;
                                default:
                                    internalFormat = GL_RGBA8_SNORM;
                                    break;
                                }
                                break;
                            case PL_FMT_UINT:
                                switch (size)
                                {
                                case 4:
                                    internalFormat = GL_RGBA32UI;
                                    break;
                                case 2:
                                    internalFormat = GL_RGBA16UI;
                                    break;
                                default:
                                    internalFormat = GL_RGBA8UI;
                                    break;
                                }
                                break;
                            case PL_FMT_SINT:
                                switch (size)
                                {
                                case 4:
                                    internalFormat = GL_RGBA32I;
                                    break;
                                case 2:
                                    internalFormat = GL_RGBA16I;
                                    break;
                                default:
                                    internalFormat = GL_RGBA8I;
                                    break;
                                }
                                break;
                            case PL_FMT_UNORM:
                                switch (size)
                                {
                                case 2:
                                    internalFormat = GL_RGBA16;
                                    break;
                                default:
                                    internalFormat = GL_RGBA8;
                                    break;
                                }
                                break;
                            default:
                                internalFormat = GL_RGBA8;
                                break;
                            }
                        }

                        GLint width = tex->params.w;
                        glGenTextures(1, &textureId);

                        const uint8_t* values = pl_tex_dummy_data(tex);
                        if (!values)
                        {
                            throw "Could not read pl_tex_dummy_data";
                        }

                        if (dims == 3)
                        {
                            GLint height = tex->params.h;
                            GLint depth = tex->params.d;
                            assert(width > 0);
                            assert(height > 0);
                            assert(depth > 0);

                            glBindTexture(GL_TEXTURE_3D, textureId);
                            setTextureParameters(GL_TEXTURE_3D, GL_LINEAR);
                            glTexImage3D(
                                GL_TEXTURE_3D, 0, internalFormat, width, height,
                                depth, 0, format, type, values);
                            textures.push_back(OCIOTexture(
                                textureId, textureName, samplerName,
                                GL_TEXTURE_3D));
                        }
                        else if (dims == 2)
                        {
                            GLint height = tex->params.h;
                            assert(height > 0);

                            glBindTexture(GL_TEXTURE_2D, textureId);
                            setTextureParameters(GL_TEXTURE_2D, GL_LINEAR);
                            glTexImage2D(
                                GL_TEXTURE_2D, 0, internalFormat, width, height,
                                0, format, type, values);
                            textures.push_back(OCIOTexture(
                                textureId, textureName, samplerName,
                                GL_TEXTURE_2D));
                        }
                        else if (dims == 1)
                        {
                            // This one seems correct
                            assert(width != 0);

                            glBindTexture(GL_TEXTURE_1D, textureId);
                            setTextureParameters(GL_TEXTURE_1D, GL_LINEAR);
                            glTexImage1D(
                                GL_TEXTURE_1D, 0, internalFormat, width, 0,
                                format, type, values);
                            textures.push_back(OCIOTexture(
                                textureId, textureName, samplerName,
                                GL_TEXTURE_1D));
                        }
                        break;
                    }
                    default:
                        throw "Unknown texture type";
                        break;
                    }
                }
            }
#endif
        } // namespace

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
                        addGPUTextures(
                            p.ocioData->textures, p.ocioData->icsDesc);
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
                        addGPUTextures(
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
                    addGPUTextures(p.lutData->textures, p.lutData->shaderDesc);
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
            if (p.hdrOptions.tonemap)
            {
                p.placeboData.reset(new LibPlaceboData);
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
                if (p.hdrOptions.tonemap)
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

                    pl_color_map_params cmap = pl_color_map_high_quality_params;

                    // defaults, generates LUTs if state is set.
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

                    switch (p.hdrOptions.algorithm)
                    {
                    case timeline::HDRTonemapAlgorithm::kNone:
                        cmap.tone_mapping_function = nullptr;
                        break;
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
                        cmap.tone_mapping_function = &pl_tone_map_linear_light;
                        break;
                    case timeline::HDRTonemapAlgorithm::ST2094_40:
                    default:
                        cmap.tone_mapping_function = &pl_tone_map_st2094_40;
                        break;
                    }

                    cmap.metadata = PL_HDR_METADATA_ANY;
                    
                    const image::HDRData& data = p.hdrOptions.hdrData;

                    pl_color_space src_colorspace;
                    memset(&src_colorspace, 0, sizeof(pl_color_space));
                    src_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
                    src_colorspace.transfer = PL_COLOR_TRC_PQ;

                    switch (data.eotf)
                    {
                    case image::EOTFType::EOTF_BT2100_PQ: // PQ (HDR10)
                    case image::EOTFType::EOTF_BT2020:    // PQ (HDR10)
                        src_colorspace.transfer = PL_COLOR_TRC_PQ;
                        break;
                    case image::EOTFType::EOTF_BT2100_HLG: // HLG
                        src_colorspace.transfer = PL_COLOR_TRC_HLG;
                        break;
                    case image::EOTFType::EOTF_BT709:
                        src_colorspace.transfer = PL_COLOR_TRC_BT_1886;
                        break;
                    case image::EOTFType::EOTF_BT601:
                    default:
                        src_colorspace.transfer = PL_COLOR_TRC_SRGB;
                        break;
                    }

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
                    pl_color_space_infer(&src_colorspace);

                    pl_color_space dst_colorspace;
                    memset(&dst_colorspace, 0, sizeof(pl_color_space));
                    
                    dst_colorspace.primaries = PL_COLOR_PRIM_BT_709;
                    dst_colorspace.transfer = PL_COLOR_TRC_BT_1886;
                    
                    dst_colorspace.hdr.min_luma = 0.0F;
                    dst_colorspace.hdr.max_luma = 100.0F;  // SDR peak in nits
                    pl_color_space_infer(&dst_colorspace);

                    pl_color_map_args color_map_args;
                    memset(&color_map_args, 0, sizeof(pl_color_map_args));

                    color_map_args.src = src_colorspace;
                    color_map_args.dst = dst_colorspace;
                    color_map_args.prelinearized = false;

                    color_map_args.state = &(p.placeboData->state);
                    
                    pl_shader_color_map_ex(shader, &cmap, &color_map_args);

                    const pl_shader_res* res = pl_shader_finalize(shader);
                    if (!res)
                    {
                        pl_shader_free(&shader);
                        throw std::runtime_error("pl_shader_finalize failed!");
                    }

                    // std::cout << "num_vertex_attribs=" <<
                    // res->num_vertex_attribs << std::endl
                    //           << "num_variables=" << res->num_variables <<
                    //           std::endl
                    //           << "num_descriptors=" << res->num_descriptors
                    //           << std::endl
                    //           << "num_constants=" << res->num_constants <<
                    //           std::endl;
                    {
                        std::stringstream s;

                        s << "#define textureLod(t, p, b) texture(t, p)"
                          << std::endl
                          << std::endl;

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

                                s << "uniform " << prefix << type << " "
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

                        s << "// Variables" << std::endl << std::endl;
                        for (int i = 0; i < res->num_variables; ++i)
                        {
                            const struct pl_shader_var shader_var =
                                res->variables[i];
                            const struct pl_var var = shader_var.var;
                            std::string glsl_type = pl_var_glsl_type_name(var);
                            s << "uniform " << glsl_type << " " << var.name;
                            if (!shader_var.data)
                            {
                                s << ";" << std::endl;
                            }
                            else
                            {
                                int dim_v = var.dim_v;
                                int dim_m = var.dim_m;
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

                                                // Check if it's the last
                                                // element
                                                if (!(r == dim_m - 1 &&
                                                      c == dim_v - 1))
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
                        }

                        s << std::endl
                          << "//" << std::endl
                          << "// Constants" << std::endl
                          << "//" << std::endl
                          << std::endl;
                        for (int i = 0; i < res->num_constants; ++i)
                        {
                            const struct pl_shader_const constant =
                                res->constants[i];
                            switch (constant.type)
                            {
                            case PL_VAR_SINT:
                                s << "const int " << constant.name << " = "
                                  << *(reinterpret_cast<const int*>(
                                         constant.data));
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
                            addGPUTextures(p.placeboData->textures, res);
                        }
                        catch (const std::exception& e)
                        {
                            throw e;
                        }
                    }

                    std::stringstream s;
                    s << "outColor = " << res->name << "(outColor);"
                      << std::endl;
                    toneMap = s.str();

                    pl_shader_free(&shader);
                }
#endif
                const std::string source = displayFragmentSource(
                    ocioICSDef, ocioICS, ocioDef, ocio, lutDef, lut,
                    p.lutOptions.order, toneMapDef, toneMap);
                if (auto context = _context.lock())
                {
                    context->log("tl::gl::GLRender", "Creating display shader");
                }
                p.shaders["display"] =
                    gl::Shader::create(vertexSource(), source);
            }
            p.shaders["display"]->bind();
            p.shaders["display"]->setUniform("transform.mvp", p.transform);
            size_t texturesOffset = 1;
#if defined(TLRENDER_OCIO)
            if (p.ocioData)
            {
                for (size_t i = 0; i < p.ocioData->textures.size(); ++i)
                {
                    p.shaders["display"]->setUniform(
                        p.ocioData->textures[i].sampler,
                        static_cast<int>(texturesOffset + i));
                }
                texturesOffset += p.ocioData->textures.size();
            }
            if (p.lutData)
            {
                for (size_t i = 0; i < p.lutData->textures.size(); ++i)
                {
                    p.shaders["display"]->setUniform(
                        p.lutData->textures[i].sampler,
                        static_cast<int>(texturesOffset + i));
                }
                texturesOffset += p.lutData->textures.size();
            }
#endif // TLRENDER_OCIO
#if defined(TLRENDER_LIBPLACEBO)
            if (p.hdrOptions.tonemap)
            {
                for (size_t i = 0; i < p.placeboData->textures.size(); ++i)
                {
                    p.shaders["display"]->setUniform(
                        p.placeboData->textures[i].sampler,
                        static_cast<int>(texturesOffset + i));
                }
                texturesOffset += p.placeboData->textures.size();
            }
#endif
        }
    } // namespace timeline_gl
} // namespace tl

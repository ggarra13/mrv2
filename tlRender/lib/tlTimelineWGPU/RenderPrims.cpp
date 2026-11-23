// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlTimelineWGPU/RenderPrivate.h>
#include <tlTimelineWGPU/RenderStructs.h>

#include <tlWGPU/WGPU.h>

#include <cstdint>
#include <stdexcept>

namespace tl
{
    namespace timeline_wgpu
    {
        namespace
        {
            [[maybe_unused]] size_t align_up(size_t x, size_t alignment) {
                return (x + alignment - 1) & ~(alignment - 1);
            }

            //! Color target with the given blend factors (blend operation is
            //! always Add), or with blending disabled.  The target format is
            //! filled in by Render::createPipeline() from the fbo.
            wgpu::ColorTargetStateInfo blendTarget(
                const bool enable, const WGPUBlendFactor srcColor,
                const WGPUBlendFactor dstColor, const WGPUBlendFactor srcAlpha,
                const WGPUBlendFactor dstAlpha)
            {
                wgpu::ColorTargetStateInfo ct;
                if (enable)
                {
                    wgpu::BlendStateInfo blend;
                    blend.color.operation = WGPUBlendOperation_Add;
                    blend.color.srcFactor = srcColor;
                    blend.color.dstFactor = dstColor;
                    blend.alpha.operation = WGPUBlendOperation_Add;
                    blend.alpha.srcFactor = srcAlpha;
                    blend.alpha.dstFactor = dstAlpha;
                    ct.blend = blend;
                }
                return ct;
            }
        }

        void Render::_create2DMesh(
            const std::string& meshName, const geom::TriangleMesh2& mesh)
        {
            TLRENDER_P();

            const size_t size = mesh.triangles.size();
            if (size == 0) return;

            ++(p.currentStats.meshes);
            p.currentStats.meshTriangles += size;

            auto type = wgpu::VBOType::Pos2_F32;
            if (!mesh.t.empty() && !mesh.c.empty())
            {
                throw std::runtime_error("Colored and textured 2D meshes unsupported");
            }
            if (!mesh.t.empty())
            {
                type = wgpu::VBOType::Pos2_F32_UV_U16;
            }
            else if (!mesh.c.empty())
            {
                type = wgpu::VBOType::Pos2_F32_Color_F32;
            }

            if (!p.vbos[meshName] ||
                (p.vbos[meshName] && p.vbos[meshName]->getSize() != size * 3))
            {
                p.vbos[meshName] = wgpu::VBO::create(size * 3, type);
            }
            if (p.vbos[meshName])
            {
                p.vbos[meshName]->copy(convert(mesh, type));
            }

        }

        void Render::_uploadMesh(const std::string& meshName,
                                 const geom::TriangleMesh2& mesh,
                                 size_t triangleCount)
        {
            TLRENDER_P();

            ++(p.currentStats.meshes);
            p.currentStats.meshTriangles += triangleCount;

            if (!p.vbos[meshName] ||
                p.vbos[meshName]->getSize() != triangleCount * 3)
            {
                p.vbos[meshName] = wgpu::VBO::create(
                    triangleCount * 3, wgpu::VBOType::Pos2_F32_UV_U16);
            }
            if (p.vbos[meshName])
                p.vbos[meshName]->copy(convert(mesh, wgpu::VBOType::Pos2_F32_UV_U16));
        }

        void Render::_create3DMesh(const std::string& meshName,
                                   const geom::TriangleMesh3& mesh)
        {
            TLRENDER_P();

            size_t triangleCount = mesh.triangles.size();
            if (triangleCount == 0) return;

            ++(p.currentStats.meshes);
            p.currentStats.meshTriangles += triangleCount;

            wgpu::VBOType type = wgpu::VBOType::Pos3_F32;
            if (!mesh.t.empty() && !mesh.n.empty() && !mesh.c.empty())
            {
                type = wgpu::VBOType::Pos3_F32_UV_U16_Normal_U10_Color_U8;
            }
            else if (!mesh.t.empty() && !mesh.n.empty() && !mesh.c.empty())
            {
                // \todo: How do we this distinguish this type?
                type = wgpu::VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32;
            }
            else if (!mesh.t.empty() && !mesh.n.empty())
            {
                type = wgpu::VBOType::Pos3_F32_UV_U16_Normal_U10;
            }
            else if (!mesh.t.empty() && !mesh.n.empty())
            {
                // \todo: How do we this distinguish this type?
                type = wgpu::VBOType::Pos3_F32_UV_F32_Normal_F32;
            }
            else if (!mesh.t.empty())
            {
                type = wgpu::VBOType::Pos3_F32_UV_U16;
            }
            else if (!mesh.c.empty())
            {
                type = wgpu::VBOType::Pos3_F32_Color_U8;
            }

            if (!p.vbos[meshName] ||
                (p.vbos[meshName] &&
                 p.vbos[meshName]->getSize() != triangleCount * 3))
            {
                p.vbos[meshName] = wgpu::VBO::create(triangleCount * 3, type);
            }
            if (p.vbos[meshName])
                p.vbos[meshName]->copy(convert(mesh, type));
        }

        void Render::_emitMeshDraw(const std::string& pipelineLayoutName,
                                   const std::string& shaderName,
                                   const std::string& meshName,
                                   const math::Matrix4x4f& transform,
                                   const image::Color4f& color)
        {
            TLRENDER_P();
            auto shader = p.shaders[shaderName];
            shader->bind(p.frameIndex);

            // WebGPU has no push constants: the push block is a uniform
            // buffer bound with a dynamic offset, written by bindToPass().
            // Each draw gets its own copy, just like vkCmdPushConstants.
            PushConstants pc;
            pc.mvp = transform;
            pc.color = color;

            _bindDescriptorSets(p.renderPass, shader, &pc);
            _vkDraw(meshName);
        }

        void Render::_setupRectCommon(const math::Box2i& box)
        {
            TLRENDER_P();
            ++(p.currentStats.rects);
            _createBindingSet(p.shaders["rect"]);
            p.shaders["rect"]->bind(p.frameIndex);
            if (p.vbos["rect"])
                p.vbos["rect"]->copy(convert(geom::box(box), p.vbos["rect"]->getType()));
        }

        void Render::drawRect(const math::Box2i& box,
                              const image::Color4f& color,
                              const std::string& /*unused*/)
        {
            TLRENDER_P();
            _setupRectCommon(box);

            const bool enableBlending = (color.a < 0.95F);
            const std::string pipelineName = enableBlending ? "rect_blending" : "rect";
            createPipeline(p.fbo, pipelineName, "rect", "rect", "rect",
                           enableBlending);
            _emitMeshDraw("rect", "rect", "rect", p.transform, color);
        }

        void Render::drawRectViewport(const std::string& pipelineName,
                                      const math::Box2i& box,
                                      const image::Color4f& color,
                                      const bool enableBlending,
                                      const std::string& shaderName)
        {
            TLRENDER_P();
            _setupRectCommon(box);

            wgpu::ColorTargetStateInfo ct;
            if (shaderName != "erase")
            {
                ct = blendTarget(
                    enableBlending, WGPUBlendFactor_SrcAlpha,
                    WGPUBlendFactor_OneMinusSrcAlpha,
                    WGPUBlendFactor_SrcAlpha,
                    WGPUBlendFactor_OneMinusSrcAlpha);
            }
            else
            {
                ct = blendTarget(
                    enableBlending, WGPUBlendFactor_Zero,
                    WGPUBlendFactor_OneMinusSrcAlpha, WGPUBlendFactor_Zero,
                    WGPUBlendFactor_OneMinusSrcAlpha);
            }

            createPipeline(pipelineName, "rect", p.fbo,
                           p.shaders["rect"], p.vbos["rect"], ct);

            _emitMeshDraw("rect", "rect", "rect", p.transform, color);
        }

        void Render::drawMesh(const std::string& pipelineName,
                              const std::string& pipelineLayoutName,
                              const std::string& shaderName,
                              const std::string& meshName,
                              const geom::TriangleMesh2& mesh,
                              const math::Vector2i& position,
                              const image::Color4f& color,
                              const bool enableBlending,
                              const WGPUBlendFactor srcColorBlendFactor,
                              const WGPUBlendFactor dstColorBlendFactor,
                              const WGPUBlendFactor srcAlphaBlendFactor,
                              const WGPUBlendFactor dstAlphaBlendFactor,
                              const WGPUBlendOperation colorBlendOp,
                              const WGPUBlendOperation alphaBlendOp)
        {
            TLRENDER_P();
            const size_t size = mesh.triangles.size();
            if (size == 0) return;

            _createBindingSet(p.shaders[shaderName]);
            _uploadMesh(meshName, mesh, size);

            createPipeline(p.fbo, pipelineName, pipelineLayoutName,
                           shaderName, meshName, enableBlending,
                           srcColorBlendFactor, dstColorBlendFactor,
                           srcAlphaBlendFactor, dstAlphaBlendFactor,
                           colorBlendOp, alphaBlendOp);

            const auto transform = p.transform *
                                   math::translate(math::Vector3f(position.x, position.y, 0.F));
            _emitMeshDraw(pipelineLayoutName, shaderName, meshName, transform, color);
        }

        void Render::drawMeshViewport(const std::string& pipelineName,
                                      const std::string& shaderName,
                                      const std::string& meshName,
                                      const geom::TriangleMesh2& mesh,
                                      const math::Vector2i& position,
                                      const image::Color4f& color,
                                      const bool enableBlending)
        {
            TLRENDER_P();
            const size_t size = mesh.triangles.size();
            if (size == 0) return;

            auto shader = p.shaders[shaderName];
            if (!shader)
                throw std::runtime_error("Unknown shader '" + shaderName + "'.");
            _createBindingSet(shader);
            _uploadMesh(meshName, mesh, size);

            const std::string pipelineLayoutName = shaderName;
            // Standard "over" blending when enabled.
            const wgpu::ColorTargetStateInfo ct = blendTarget(
                enableBlending, WGPUBlendFactor_SrcAlpha,
                WGPUBlendFactor_OneMinusSrcAlpha, WGPUBlendFactor_One,
                WGPUBlendFactor_OneMinusSrcAlpha);

            createPipeline(pipelineName, pipelineLayoutName,
                           p.fbo, shader, p.vbos[meshName], ct);

            const auto transform = p.transform *
                                   math::translate(math::Vector3f(position.x, position.y, 0.F));
            _emitMeshDraw(pipelineLayoutName, shaderName, meshName, transform, color);
        }


        void Render::drawMesh(
            const geom::TriangleMesh2& mesh, const math::Vector2i& position,
            const image::Color4f& color, const std::string& meshName)
        {
            drawMesh(meshName, "mesh", "mesh", meshName,
                     mesh, position, color, false);
        }

        void Render::drawColorMesh(
            const std::string& pipelineLayoutName,
            const geom::TriangleMesh2& mesh,
            const math::Vector2i& position,
            const image::Color4f& color)
        {
            TLRENDER_P();
            const size_t size = mesh.triangles.size();
            if (size == 0)
                return;

            ++(p.currentStats.meshes);
            p.currentStats.meshTriangles += mesh.triangles.size();

            auto shader = p.shaders["colorMesh"];
            _createBindingSet(shader);

            const auto transform =
                p.transform *
                math::translate(
                    math::Vector3f(position.x, position.y, 0.F));

            shader->bind(p.frameIndex);
            PushConstants pc;
            pc.mvp = transform;
            pc.color = color;
            _bindDescriptorSets(p.renderPass, shader, &pc);

            // Uploads the vertex data into the pool and draws immediately.
            _vkDraw("colorMesh");
        }



        void Render::Private::createTextMesh(
            Fl_WebGPU_Context& ctx, const geom::TriangleMesh2& mesh)
        {
            const size_t size = mesh.triangles.size();
            if (size == 0)
                return;
            currentStats.textTriangles += size;
            if (!vbos["text"] ||
                (vbos["text"] && vbos["text"]->getSize() != size * 3))
            {
                vbos["text"] = wgpu::VBO::create(size * 3,
                                                wgpu::VBOType::Pos2_F32_UV_U16);
            }
            if (vbos["text"])
            {
                vbos["text"]->copy(convert(mesh, vbos["text"]->getType()));
            }
        }

        void Render::drawText(
            const timeline::TextInfo& info,
            const math::Vector2i& position,
            const image::Color4f& color)
        {
            TLRENDER_P();

            const geom::TriangleMesh2& mesh = info.mesh;
            const size_t size = mesh.triangles.size();
            if (size == 0) return;

            const auto& textures = p.glyphTextureAtlas->getTextures();
            const unsigned textureIndex = info.textureId;
            const math::Matrix4x4f transform =
                p.transform *
                math::translate(math::Vector3f(position.x, position.y, 0.F));

            _uploadMesh("text", mesh, size);
            _createBindingSet(p.shaders["text"]);

            const wgpu::ColorTargetStateInfo ct = blendTarget(
                true, WGPUBlendFactor_SrcAlpha,
                WGPUBlendFactor_OneMinusSrcAlpha, WGPUBlendFactor_SrcAlpha,
                WGPUBlendFactor_OneMinusSrcAlpha);

            // Default depth/stencil state: no depth test, no depth write,
            // no stencil.
            wgpu::DepthStencilStateInfo ds;

            auto shader = p.shaders["text"];
            createPipeline("text", "text", p.fbo, shader,
                           p.vbos["text"], ct, ds);

            shader->bind(p.frameIndex);
            shader->setTexture("textureSampler",
                               textures[textureIndex]);

            PushConstants pc;
            pc.mvp = transform;
            pc.color = color;
            _bindDescriptorSets(p.renderPass, shader, &pc);

            _vkDraw("text");
        }

        void Render::appendText(
            std::vector<timeline::TextInfo>& textInfos,
            const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
            const math::Vector2i& pos)
        {
            TLRENDER_P();

            uint8_t textureIndex = 0;
            const auto& textures = p.glyphTextureAtlas->getTextures();

            int x = 0;
            int32_t rsbDeltaPrev = 0;

            if (textInfos.empty())
            {
                timeline::TextInfo textInfo(textureIndex);
                textInfos.emplace_back(textInfo);
            }
            else
            {
                timeline::TextInfo& lastTextInfo = textInfos.back();
                if (lastTextInfo.textureId != textureIndex)
                {
                    timeline::TextInfo textInfo(textureIndex);
                    textInfos.emplace_back(textInfo);
                }
            }

            timeline::TextInfo& lastTextInfo = textInfos.back();
            geom::TriangleMesh2& mesh = lastTextInfo.mesh;
            size_t& meshIndex = lastTextInfo.meshIndex;
            for (const auto& glyph : glyphs)
            {
                if (glyph)
                {
                    if (rsbDeltaPrev - glyph->lsbDelta > 32)
                    {
                        x -= 1;
                    }
                    else if (rsbDeltaPrev - glyph->lsbDelta < -31)
                    {
                        x += 1;
                    }
                    rsbDeltaPrev = glyph->rsbDelta;

                    if (glyph->image && glyph->image->isValid())
                    {
                        wgpu::TextureAtlasID id = 0;
                        const auto i = p.glyphIDs.find(glyph->info);
                        if (i != p.glyphIDs.end())
                        {
                            id = i->second;
                        }
                        wgpu::TextureAtlasItem item;
                        if (!p.glyphTextureAtlas->getItem(id, item))
                        {
                            id = p.glyphTextureAtlas->addItem(
                                glyph->image, item);
                            p.glyphIDs[glyph->info] = id;
                        }
                        if (item.textureIndex != textureIndex)
                        {
                            textureIndex = item.textureIndex;

                            const timeline::TextInfo textInfo(textureIndex);
                            textInfos.emplace_back(textInfo);

                            timeline::TextInfo& lastTextInfo = textInfos.back();
                            mesh = lastTextInfo.mesh;
                            meshIndex = lastTextInfo.meshIndex;
                        }

                        const math::Vector2i& offset = glyph->offset;
                        const math::Box2i box(
                            pos.x + x + offset.x, pos.y - offset.y,
                            glyph->image->getWidth(),
                            glyph->image->getHeight());
                        lastTextInfo.box = math::expand(lastTextInfo.box,
                                                        box);
                        const auto& min = box.min;
                        const auto& max = box.max;

                        mesh.v.push_back(math::Vector2f(min.x, min.y));
                        mesh.v.push_back(math::Vector2f(max.x + 1, min.y));
                        mesh.v.push_back(math::Vector2f(max.x + 1, max.y + 1));
                        mesh.v.push_back(math::Vector2f(min.x, max.y + 1));

                        mesh.t.push_back(
                            math::Vector2f(
                                item.textureU.min(),
                                item.textureV.min()));
                        mesh.t.push_back(
                            math::Vector2f(
                                item.textureU.max(),
                                item.textureV.min()));
                        mesh.t.push_back(
                            math::Vector2f(
                                item.textureU.max(),
                                item.textureV.max()));
                        mesh.t.push_back(
                            math::Vector2f(
                                item.textureU.min(),
                                item.textureV.max()));

                        geom::Triangle2 triangle;
                        triangle.v[0].v = meshIndex + 1;
                        triangle.v[1].v = meshIndex + 2;
                        triangle.v[2].v = meshIndex + 3;
                        triangle.v[0].t = meshIndex + 1;
                        triangle.v[1].t = meshIndex + 2;
                        triangle.v[2].t = meshIndex + 3;
                        mesh.triangles.push_back(triangle);
                        triangle.v[0].v = meshIndex + 3;
                        triangle.v[1].v = meshIndex + 4;
                        triangle.v[2].v = meshIndex + 1;
                        triangle.v[0].t = meshIndex + 3;
                        triangle.v[1].t = meshIndex + 4;
                        triangle.v[2].t = meshIndex + 1;
                        mesh.triangles.push_back(triangle);

                        meshIndex += 4;
                    }

                    x += glyph->advance;
                }
            }
        }

        void Render::drawTexture(
            const std::shared_ptr<wgpu::Texture>& texture,
            const math::Box2i& box, const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.textures);

            const std::string pipelineName = "texture";
            const std::string shaderName = "texture";
            const std::string pipelineLayoutName = shaderName;
            const std::string meshName = "texture";

            createPipeline(
                p.fbo, pipelineName, pipelineLayoutName,
                shaderName, meshName);

            auto shader = p.shaders[shaderName];
            shader->bind(p.frameIndex);
            shader->setTexture("textureSampler", texture);

            if (p.vbos["texture"])
            {
                p.vbos["texture"]->copy(
                    convert(geom::box(box), p.vbos["texture"]->getType()));
            }

            PushConstants pc;
            pc.mvp = p.transform;
            pc.color = color;
            _bindDescriptorSets(p.renderPass, shader, &pc);

            _vkDraw("texture");
        }

        void Render::_drawImageCommon(
            const std::shared_ptr<wgpu::OffscreenBuffer>& fbo,
            const std::shared_ptr<image::Image>& image, const math::Box2i& box,
            const image::Color4f& color,
            const timeline::ImageOptions& imageOptions,
            const bool clearRenderPass,
            const bool flip,
            const bool applyClipRect)
        {
            TLRENDER_P();
            ++(p.currentStats.images);

            const auto& info = image->getInfo();
            std::vector<std::shared_ptr<wgpu::Texture> > textures;
            if (!imageOptions.cache)
            {
                textures = getTextures(ctx, info, imageOptions.imageFilters);
                copyTextures(image, textures);
            }
            else if (!p.textureCache->get(image, textures))
            {
                textures = getTextures(ctx, info, imageOptions.imageFilters);
                copyTextures(image, textures);
                p.textureCache->add(image, textures, image->getDataByteCount());
            }

            auto shader = p.shaders["image"];
            _createBindingSet(shader);

            shader->bind(p.frameIndex);

            UBOTexture ubo;
            ubo.color = color;
            ubo.pixelType = static_cast<int>(info.pixelType);
            image::VideoLevels videoLevels = info.videoLevels;
            switch (imageOptions.videoLevels)
            {
            case timeline::InputVideoLevels::FullRange:
                videoLevels = image::VideoLevels::FullRange;
                break;
            case timeline::InputVideoLevels::LegalRange:
                videoLevels = image::VideoLevels::LegalRange;
                break;
            default:
                break;
            }
            ubo.videoLevels = static_cast<int>(videoLevels);
            ubo.yuvCoefficients =
                image::getYUVCoefficients(info.yuvCoefficients);
            ubo.imageChannels = image::getChannelCount(info.pixelType);
            ubo.mirrorX = info.layout.mirror.x;
            ubo.mirrorY = !info.layout.mirror.y;
            shader->setUniform("ubo", ubo);

            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
            case image::PixelType::YUV_422P_U8:
            case image::PixelType::YUV_444P_U8:
            case image::PixelType::YUV_420P_U10:
            case image::PixelType::YUV_422P_U10:
            case image::PixelType::YUV_444P_U10:
            case image::PixelType::YUV_420P_U12:
            case image::PixelType::YUV_422P_U12:
            case image::PixelType::YUV_444P_U12:
            case image::PixelType::YUV_420P_U16:
            case image::PixelType::YUV_422P_U16:
            case image::PixelType::YUV_444P_U16:
                shader->setTexture("textureSampler0", textures[0]);
                shader->setTexture("textureSampler1", textures[1]);
                shader->setTexture("textureSampler2", textures[2]);
                break;
            case image::PixelType::YUV_420SP_U8:
            case image::PixelType::YUV_420SP_U16:
            case image::PixelType::YUV_444SP_U8:
            case image::PixelType::YUV_422SP_U8:
            case image::PixelType::YUV_422SP_U16:
            case image::PixelType::YUV_444SP_U16:
                shader->setTexture("textureSampler0", textures[0]);
                shader->setTexture("textureSampler1", textures[1]);
                shader->setTexture("textureSampler2", textures[1]);
                break;
            default:
                shader->setTexture("textureSampler0", textures[0]);
                shader->setTexture("textureSampler1", textures[0]);
                shader->setTexture("textureSampler2", textures[0]);
                break;
            }
            bool enableBlending = true;
            WGPUBlendFactor srcColorBlendFactor = WGPUBlendFactor_SrcAlpha;
            WGPUBlendFactor dstColorBlendFactor = WGPUBlendFactor_OneMinusSrcAlpha;
            WGPUBlendFactor srcAlphaBlendFactor = WGPUBlendFactor_One;
            WGPUBlendFactor dstAlphaBlendFactor = WGPUBlendFactor_OneMinusSrcAlpha;
            switch (imageOptions.alphaBlend)
            {
            case timeline::AlphaBlend::kNone:
                srcColorBlendFactor = WGPUBlendFactor_One;
                dstColorBlendFactor = WGPUBlendFactor_Zero;
                srcAlphaBlendFactor = WGPUBlendFactor_One;
                dstAlphaBlendFactor = WGPUBlendFactor_Zero;
                break;
            case timeline::AlphaBlend::Straight:
                srcColorBlendFactor = WGPUBlendFactor_SrcAlpha;
                dstColorBlendFactor = WGPUBlendFactor_OneMinusSrcAlpha;
                srcAlphaBlendFactor = WGPUBlendFactor_One;
                dstAlphaBlendFactor = WGPUBlendFactor_OneMinusSrcAlpha;
                break;
            case timeline::AlphaBlend::Premultiplied:
                srcColorBlendFactor = WGPUBlendFactor_One;
                dstColorBlendFactor = WGPUBlendFactor_OneMinusSrcAlpha;
                srcAlphaBlendFactor = WGPUBlendFactor_One;
                dstAlphaBlendFactor = WGPUBlendFactor_OneMinusSrcAlpha;
                break;
            default:
                break;
            }

            // WebGPU: the pass is an encoder object returned by the fbo.  Pass
            // state (bound pipeline etc.) does not carry over from a
            // previous pass.
            p.renderPass = clearRenderPass ? fbo->beginClearRenderPass(p.cmd)
                                   : fbo->beginLoadRenderPass(p.cmd);
            p.currentPipeline.clear();

            if (p.vbos["image"])
            {
                p.vbos["image"]->copy(
                    convert(geom::box(box, flip), p.vbos["image"]->getType()));
            }

            //
            // Create pipeline
            //
            const std::string pipelineName = "image";
            const std::string pipelineLayoutName = "image";
            const std::string shaderName = "image";
            const std::string meshName = "image";
            createPipeline(fbo, pipelineName, pipelineLayoutName,
                           shaderName, meshName, enableBlending,
                           srcColorBlendFactor, dstColorBlendFactor,
                           srcAlphaBlendFactor, dstAlphaBlendFactor);

            PushConstants pc;
            pc.mvp = p.transform;
            pc.color = image::Color4f(1.F, 1.F, 1.F, 1.F);

            _bindDescriptorSets(p.renderPass, p.shaders["image"], &pc);

            // Reset viewport/scissor (createPipeline() may have applied the
            // clip rect); the clip rect is re-applied only if asked for.
            fbo->setupViewportAndScissor(p.renderPass);

            if (applyClipRect && p.clipRectEnabled)
            {
                setClipRect(p.clipRect);
            }

            _vkDraw("image");

            fbo->endRenderPass(p.renderPass); // ends and releases the pass
            p.renderPass = nullptr;
            p.currentPipeline.clear();
        }

        void Render::drawImage(
            const std::shared_ptr<wgpu::OffscreenBuffer>& fbo,
            const std::shared_ptr<image::Image>& image, const math::Box2i& box,
            const image::Color4f& color,
            const timeline::ImageOptions& imageOptions,
            const bool clearRenderPass)
        {
            _drawImageCommon(
                fbo, image, box, color, imageOptions, clearRenderPass,
                /* flip = */ true, /* applyClipRect = */ false);
        }

        void Render::drawImage(
            const std::shared_ptr<image::Image>& image, const math::Box2i& box,
            const image::Color4f& color,
            const timeline::ImageOptions& imageOptions)
        {
            TLRENDER_P();

            // This is used in the timeline, so we need to use a load
            // render pass.
            _drawImageCommon(
                p.fbo, image, box, color, imageOptions,
                /* clearRenderPass = */ false,
                /* flip             = */ false,
                /* applyClipRect    = */ true);
        }


    } // namespace timeline_wgpu
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlTimelineVk/RenderPrivate.h>
#include <tlTimelineVk/RenderStructs.h>

#include <tlVk/Vk.h>

#include <cstdint>

namespace tl
{
    namespace timeline_vlk
    {
        void Render::_create2DMesh(
            const std::string& meshName, const geom::TriangleMesh2& mesh)
        {
            TLRENDER_P();

            const size_t size = mesh.triangles.size();

            auto type = vlk::VBOType::Pos3_F32;
            if (!mesh.t.empty() && !mesh.c.empty())
            {
                throw std::runtime_error("Colored and textured 2D meshes unsupported");
            }
            if (!mesh.t.empty())
            {
                type = vlk::VBOType::Pos2_F32_UV_U16;
            }
            else if (!mesh.c.empty())
            {
                type = vlk::VBOType::Pos2_F32_Color_F32;
            }

            if (!p.vbos[meshName] ||
                (p.vbos[meshName] && p.vbos[meshName]->getSize() != size * 3))
            {
                p.vbos[meshName] = vlk::VBO::create(size * 3, type);
            }
            if (p.vbos[meshName])
            {
                p.vbos[meshName]->copy(convert(mesh, type));
            }

            if (!p.vaos[meshName] && p.vbos[meshName])
            {
                p.vaos[meshName] = vlk::VAO::create(ctx);
                p.vaos[meshName]->bind(p.frameIndex);
            }
        }

        void Render::drawRect(const math::Box2i& box,
                              const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.rects);

            auto shader = p.shaders["rect"];
            _createBindingSet(shader);
            
            shader->bind(p.frameIndex);
            shader->setUniform("transform.mvp", p.transform);

            if (p.vbos["rect"])
            {
                p.vbos["rect"]->copy(
                    convert(geom::box(box), p.vbos["rect"]->getType()));
            }

            bool enableBlending = false;
            if (color.a < 0.95F)
            {
                bool enableBlending = true;
                createPipeline(p.fbo, "rect_blending", "rect", "rect", "rect",
                               enableBlending);
            }
            else
            {
                createPipeline(p.fbo, "rect", "rect", "rect", "rect",
                               enableBlending);
            }

            VkPipelineLayout pipelineLayout = p.pipelineLayouts["rect"];
            vkCmdPushConstants(
                p.cmd, pipelineLayout,
                p.shaders["rect"]->getPushStageFlags(), 0, sizeof(color),
                &color);
            
            _bindDescriptorSets("rect", "rect");

            _vkDraw("rect");
        }
        
        //! This function draws to the viewport
        void Render::drawRect(const std::string& pipelineName,
                              const math::Box2i& box,
                              const image::Color4f& color,
                              const bool enableBlending)
        {
            TLRENDER_P();
            ++(p.currentStats.rects);

            _createBindingSet(p.shaders["rect"]);
            
            p.shaders["rect"]->bind(p.frameIndex);
            p.shaders["rect"]->setUniform("transform.mvp", p.transform);

            if (p.vbos["rect"])
            {
                p.vbos["rect"]->copy(
                    convert(geom::box(box), p.vbos["rect"]->getType()));
            }
            
            vlk::ColorBlendStateInfo cb;
            vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
            colorBlendAttachment.blendEnable = enableBlending ?
                                               VK_TRUE : VK_FALSE;
            
            cb.attachments.push_back(colorBlendAttachment);
            
            createPipeline(pipelineName, "rect",
                           getRenderPass(),
                           p.shaders["rect"],
                           p.vbos["rect"], cb);
                
            VkPipelineLayout pipelineLayout = p.pipelineLayouts["rect"];
            vkCmdPushConstants(
                p.cmd, pipelineLayout,
                p.shaders["rect"]->getPushStageFlags(), 0, sizeof(color),
                &color);
                
            _bindDescriptorSets("rect", "rect");

            _vkDraw("rect");
        }

        
        void Render::drawMesh(const std::string& pipelineName,
                              const std::string& pipelineLayoutName,
                              const std::string& shaderName,
                              const std::string& meshName,
                              const geom::TriangleMesh2& mesh,
                              const math::Vector2i& position,
                              const image::Color4f& color,
                              const bool enableBlending,
                              const VkBlendFactor srcColorBlendFactor,
                              const VkBlendFactor dstColorBlendFactor,
                              const VkBlendFactor srcAlphaBlendFactor,
                              const VkBlendFactor dstAlphaBlendFactor,
                              const VkBlendOp colorBlendOp,
                              const VkBlendOp alphaBlendOp)
        {
            TLRENDER_P();
            const size_t size = mesh.triangles.size();
            if (size == 0)
                return;
            
            ++(p.currentStats.meshes);
            p.currentStats.meshTriangles += mesh.triangles.size();

            auto shader = p.shaders[shaderName];
            _createBindingSet(shader);
            
            const auto transform =
                p.transform *
                math::translate(
                    math::Vector3f(position.x, position.y, 0.F));

            if (!p.vbos[meshName] ||
                (p.vbos[meshName] && p.vbos[meshName]->getSize() != size * 3))
            {
                p.vbos[meshName] = vlk::VBO::create(
                    size * 3, vlk::VBOType::Pos2_F32_UV_U16);
            }
            if (p.vbos[meshName])
            {
                p.vbos[meshName]->copy(
                    convert(mesh, vlk::VBOType::Pos2_F32_UV_U16));
            }

            if (!p.vaos[meshName] && p.vbos[meshName])
            {
                p.vaos[meshName] = vlk::VAO::create(ctx);
                p.vaos[meshName]->bind(p.frameIndex);
            }
            
            createPipeline(
                p.fbo, pipelineName, pipelineLayoutName,
                shaderName, meshName, enableBlending,
                srcColorBlendFactor, dstColorBlendFactor,
                srcAlphaBlendFactor, dstAlphaBlendFactor,
                colorBlendOp, alphaBlendOp);

            VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            vkCmdPushConstants(
                p.cmd, pipelineLayout,
                shader->getPushStageFlags(), 0,
                sizeof(color), &color);
                
            shader->bind(p.frameIndex);
            shader->setUniform("transform.mvp", transform);

            _bindDescriptorSets(pipelineLayoutName, shaderName);

            _vkDraw(meshName);
        }
        
        void Render::drawMesh(const std::string& pipelineName,
                              const std::string& shaderName,
                              const std::string& meshName,
                              const geom::TriangleMesh2& mesh,
                              const math::Vector2i& position,
                              const image::Color4f& color,
                              const bool enableBlending)
        {
            TLRENDER_P();
            const size_t size = mesh.triangles.size();
            if (size == 0)
                return;
            
            ++(p.currentStats.meshes);
            p.currentStats.meshTriangles += mesh.triangles.size();

            auto shader = p.shaders[shaderName];
            if (!shader)
            {
                throw std::runtime_error("Unknown shader '" + shaderName + "'.");
            }
            _createBindingSet(shader);
            
            const auto transform =
                p.transform *
                math::translate(
                    math::Vector3f(position.x, position.y, 0.F));

            if (!p.vbos[meshName] ||
                (p.vbos[meshName] && p.vbos[meshName]->getSize() != size * 3))
            {
                p.vbos[meshName] = vlk::VBO::create(
                    size * 3, vlk::VBOType::Pos2_F32_UV_U16);
            }
            if (p.vbos[meshName])
            {
                p.vbos[meshName]->copy(
                    convert(mesh, vlk::VBOType::Pos2_F32_UV_U16));
            }

            if (!p.vaos[meshName] && p.vbos[meshName])
            {
                p.vaos[meshName] = vlk::VAO::create(ctx);
                p.vaos[meshName]->bind(p.frameIndex);
            }
            
            const std::string pipelineLayoutName = shaderName;

            vlk::ColorBlendStateInfo cb;
            vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
            colorBlendAttachment.blendEnable = enableBlending ?
                                               VK_TRUE : VK_FALSE;
            
            cb.attachments.push_back(colorBlendAttachment);
                
            createPipeline(pipelineName, pipelineLayoutName,
                           getRenderPass(),
                           p.shaders[shaderName],
                           p.vbos[meshName], cb);
                
            VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            vkCmdPushConstants(
                p.cmd, pipelineLayout,
                shader->getPushStageFlags(), 0,
                sizeof(color), &color);
                
            shader->bind(p.frameIndex);
            shader->setUniform("transform.mvp", transform);

            _bindDescriptorSets(pipelineLayoutName, shaderName);

            _vkDraw(meshName);
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

            _createBindingSet(p.shaders["colorMesh"]);
            
            const auto transform =
                p.transform *
                math::translate(
                    math::Vector3f(position.x, position.y, 0.F));
            
            p.shaders["colorMesh"]->bind(p.frameIndex);
            p.shaders["colorMesh"]->setUniform(
                "transform.mvp", transform, vlk::kShaderVertex);
            _bindDescriptorSets(pipelineLayoutName, "colorMesh");

            if (p.vaos["colorMesh"] && p.vbos["colorMesh"])
            {
                _vkDraw("colorMesh");
            }
        }


        
        void Render::Private::createTextMesh(
            Fl_Vk_Context& ctx, const geom::TriangleMesh2& mesh)
        {
            const size_t size = mesh.triangles.size();
            if (size == 0)
                return;
            currentStats.textTriangles += size;
            if (!vbos["text"] ||
                (vbos["text"] && vbos["text"]->getSize() != size * 3))
            {
                vbos["text"] = vlk::VBO::create(size * 3,
                                                vlk::VBOType::Pos2_F32_UV_U16);
            }
            if (vbos["text"])
            {
                vbos["text"]->copy(convert(mesh, vbos["text"]->getType()));
            }
            if (!vaos["text"] && vbos["text"])
            {
                vaos["text"] = vlk::VAO::create(ctx);
                vaos["text"]->bind(frameIndex);
            }
        }

        void Render::drawText(
            const timeline::TextInfo& info,
            const math::Vector2i& position,
            const image::Color4f& color)
        {
            TLRENDER_P();
            
            const auto& textures = p.glyphTextureAtlas->getTextures();
            
            const geom::TriangleMesh2& mesh = info.mesh;
            const unsigned textureIndex = info.textureId;
            const math::Matrix4x4f transform =
                p.transform *
                math::translate(math::Vector3f(position.x, position.y, 0.F));

            _create2DMesh("text", mesh);
            _createBindingSet(p.shaders["text"]);
            
            vlk::ColorBlendStateInfo cb;
            vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
            colorBlendAttachment.blendEnable = VK_TRUE;
                
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            
            cb.attachments.push_back(colorBlendAttachment);
            
                
            vlk::DepthStencilStateInfo ds;
            ds.depthTestEnable = VK_FALSE;
            ds.depthWriteEnable = VK_FALSE;
            ds.stencilTestEnable = VK_FALSE;
            
            createPipeline("text", "text",
                           getRenderPass(), p.shaders["text"],
                           p.vbos["text"], cb, ds);
            
            p.shaders["text"]->bind(p.frameIndex);
            p.shaders["text"]->setUniform("transform.mvp", transform);
            p.shaders["text"]->setTexture("textureSampler",
                                          textures[textureIndex]);
            _bindDescriptorSets("text", "text");
                            
            VkPipelineLayout pipelineLayout = p.pipelineLayouts["text"];
            vkCmdPushConstants(
                p.cmd, pipelineLayout,
                p.shaders["text"]->getPushStageFlags(), 0,
                sizeof(color), &color);
                            
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
                        vlk::TextureAtlasID id = 0;
                        const auto i = p.glyphIDs.find(glyph->info);
                        if (i != p.glyphIDs.end())
                        {
                            id = i->second;
                        }
                        vlk::TextureAtlasItem item;
                        if (!p.glyphTextureAtlas->getItem(id, item))
                        {
                            id = p.glyphTextureAtlas->addItem(
                                p.cmd, glyph->image, item);
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
                        const auto& min = box.min;
                        const auto& max = box.max;

                        mesh.v.push_back(math::Vector2f(min.x, min.y));
                        mesh.v.push_back(math::Vector2f(max.x + 1, min.y));
                        mesh.v.push_back(math::Vector2f(max.x + 1, max.y + 1));
                        mesh.v.push_back(math::Vector2f(min.x, max.y + 1));
                        
                        mesh.t.push_back(
                            math::Vector2f(
                                item.textureU.getMin(),
                                item.textureV.getMin()));
                        mesh.t.push_back(
                            math::Vector2f(
                                item.textureU.getMax(),
                                item.textureV.getMin()));
                        mesh.t.push_back(
                            math::Vector2f(
                                item.textureU.getMax(),
                                item.textureV.getMax()));
                        mesh.t.push_back(
                            math::Vector2f(
                                item.textureU.getMin(),
                                item.textureV.getMax()));
                        
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
            const std::shared_ptr<vlk::Texture>& texture,
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
            shader->setUniform("textureSampler", texture);
            _bindDescriptorSets(pipelineLayoutName, shaderName);

            if (p.vbos["texture"])
            {
                p.vbos["texture"]->copy(
                    convert(geom::box(box), p.vbos["texture"]->getType()));
            }
            
            VkPipelineLayout pipelineLayout = p.pipelineLayouts["texture"];
            vkCmdPushConstants(
                p.cmd, pipelineLayout,
                shader->getPushStageFlags(), 0,
                sizeof(color), &color);
            
            _vkDraw("texture");
        }

        void Render::drawImage(
            const std::shared_ptr<vlk::OffscreenBuffer>& fbo,
            const std::shared_ptr<image::Image>& image, const math::Box2i& box,
            const image::Color4f& color,
            const timeline::ImageOptions& imageOptions)
        {
            TLRENDER_P();
            ++(p.currentStats.images);

            const auto& info = image->getInfo();
            std::vector<std::shared_ptr<vlk::Texture> > textures;
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
            shader->setUniform("transform.mvp", p.transform);

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
                textures[0]->transition(
                    p.cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                textures[1]->transition(
                    p.cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                textures[2]->transition(
                    p.cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                shader->setTexture("textureSampler0", textures[0]);
                shader->setTexture("textureSampler1", textures[1]);
                shader->setTexture("textureSampler2", textures[2]);
                break;
            default:
                textures[0]->transition(
                    p.cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                shader->setTexture("textureSampler0", textures[0]);
                shader->setTexture("textureSampler1", textures[0]);
                shader->setTexture("textureSampler2", textures[0]);
                break;
            }
            bool enableBlending = true;
            VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            switch (imageOptions.alphaBlend)
            {
            case timeline::AlphaBlend::kNone:
                srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
                srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                break;
            case timeline::AlphaBlend::Straight:
                srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                break;
            case timeline::AlphaBlend::Premultiplied:
                srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                break;
            default:
                break;
            }

            fbo->beginClearRenderPass(p.cmd);
            if (p.vbos["image"])
            {
                p.vbos["image"]->copy(
                    convert(geom::box(box, true), p.vbos["image"]->getType()));
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
            _bindDescriptorSets(pipelineLayoutName, shaderName);
            fbo->setupViewportAndScissor(p.cmd);

            _vkDraw("image");
            
            fbo->endRenderPass(p.cmd);
        }

        void Render::drawImage(
            const std::shared_ptr<image::Image>& image, const math::Box2i& box,
            const image::Color4f& color,
            const timeline::ImageOptions& imageOptions)
        {
            TLRENDER_P();
            ++(p.currentStats.images);

            const auto& info = image->getInfo();
            std::vector<std::shared_ptr<vlk::Texture> > textures;
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
            shader->setUniform("transform.mvp", p.transform);

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
                textures[0]->transition(
                    p.cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                textures[1]->transition(
                    p.cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                textures[2]->transition(
                    p.cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                shader->setTexture("textureSampler0", textures[0]);
                shader->setTexture("textureSampler1", textures[1]);
                shader->setTexture("textureSampler2", textures[2]);
                break;
            default:
                textures[0]->transition(
                    p.cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                shader->setTexture("textureSampler0", textures[0]);
                shader->setTexture("textureSampler1", textures[0]);
                shader->setTexture("textureSampler2", textures[0]);
                break;
            }
            bool enableBlending = true;
            VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            switch (imageOptions.alphaBlend)
            {
            case timeline::AlphaBlend::kNone:
                srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
                srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                break;
            case timeline::AlphaBlend::Straight:
                srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                break;
            case timeline::AlphaBlend::Premultiplied:
                srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                break;
            default:
                break;
            }

            p.fbo->beginLoadRenderPass(p.cmd);
            if (p.vbos["image"])
            {
                p.vbos["image"]->copy(
                    convert(geom::box(box, false), p.vbos["image"]->getType()));
            }

            //
            // Create pipeline
            //
            const std::string pipelineName = "image";
            const std::string pipelineLayoutName = "image";
            const std::string shaderName = "image";
            const std::string meshName = "image";
            createPipeline(p.fbo, pipelineName, pipelineLayoutName,
                           shaderName, meshName, enableBlending,
                           srcColorBlendFactor, dstColorBlendFactor,
                           srcAlphaBlendFactor, dstAlphaBlendFactor);
            _bindDescriptorSets(pipelineLayoutName, shaderName);
            p.fbo->setupViewportAndScissor(p.cmd);

            if (p.clipRectEnabled)
            {
                setClipRect(p.clipRect);
            }
            
            _vkDraw("image");
                
            p.fbo->endRenderPass(p.cmd);
        }

        
    } // namespace timeline_vlk
} // namespace tl

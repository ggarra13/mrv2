// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 Gonzalo Garramuño
// All rights reserved.

#include <tlTimelineVk/RenderPrivate.h>

#include <tlVk/Vk.h>

#include <cstdint>

namespace tl
{
    namespace timeline_vlk
    {
        void Render::drawRect(const std::string& pipelineName,
                              const std::string& shaderName,
                              const std::string& meshName,
                              const math::Box2i& box,
                              const image::Color4f& color,
                              const bool enableBlending)
        {
            TLRENDER_P();
            ++(p.currentStats.rects);

            auto shader = p.shaders[shaderName];

            auto bindingSet = shader->createBindingSet();
            p.garbage[p.frameIndex].bindingSets.push_back(bindingSet);
            
            shader->bind(p.frameIndex);
            shader->setUniform("transform.mvp", p.transform);

            if (p.vbos["rect"])
            {
                p.vbos["rect"]->copy(
                    convert(geom::box(box), p.vbos["rect"]->getType()));
            }
            if (p.vaos["rect"])
            {
                p.vaos["rect"]->bind(p.frameIndex);

                const std::string& pipelineLayoutName = shaderName;
                createPipeline(p.fbo, pipelineName, pipelineLayoutName,
                               shaderName, meshName, enableBlending);
                bindDescriptorSets(pipelineLayoutName, shaderName);

                VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
                if (pipelineLayout == VK_NULL_HANDLE)
                {
                    throw std::runtime_error("drawRect '" + pipelineLayoutName
                                             + "' undefined");
                }
                vkCmdPushConstants(
                    p.cmd, pipelineLayout,
                    p.shaders[shaderName]->getPushStageFlags(), 0, sizeof(color),
                    &color);

                p.vaos["rect"]->draw(p.cmd, p.vbos["rect"]);
            }
        }
        
        void
        Render::drawRect(const math::Box2i& box, const image::Color4f& color)
        {
            drawRect("timeline", "rect", "rect", box, color, false);
        }

        void Render::drawMesh(const std::string& pipelineName,
                              const std::string& pipelineLayoutName,
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
            auto bindingSet = shader->createBindingSet();
            shader->useBindingSet(bindingSet);
            p.garbage[p.frameIndex].bindingSets.push_back(bindingSet);

            
            const auto transform =
                p.transform *
                math::translate(
                    math::Vector3f(position.x, position.y, 0.F));

            if (!p.vbos[meshName] ||
                (p.vbos[meshName] && p.vbos[meshName]->getSize() < size * 3))
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
            }
            if (p.vaos[meshName] && p.vbos[meshName])
            {
                shader->bind(p.frameIndex);
                shader->setUniform("transform.mvp", transform);

                createPipeline(
                    p.fbo, pipelineName, pipelineLayoutName,
                    shaderName, meshName, enableBlending);
                bindDescriptorSets(pipelineLayoutName, shaderName);

                const std::string pipelineLayoutName = shaderName;
                VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
                if (!pipelineLayout)
                    throw std::runtime_error("drawMesh '" +
                                             meshName +
                                             "': Invalid pipeline Layout '" +
                                             pipelineLayoutName + "'"); 
                vkCmdPushConstants(
                    p.cmd, pipelineLayout,
                    shader->getPushStageFlags(), 0,
                    sizeof(color), &color);

                p.vaos[meshName]->bind(p.frameIndex);
                p.vaos[meshName]->draw(p.cmd, p.vbos[meshName]);
            }
        }
        
        void Render::drawMesh(
            const geom::TriangleMesh2& mesh, const math::Vector2i& position,
            const image::Color4f& color)
        {
            drawMesh("timeline", "mesh", "mesh", "mesh", mesh, position, color);
        }

        void Render::drawColorMesh(
            const std::string& pipelineLayoutName, const geom::TriangleMesh2& mesh,
            const math::Vector2i& position, const image::Color4f& color)
        {
            TLRENDER_P();
            const size_t size = mesh.triangles.size();
            if (size == 0)
                return;
            
            ++(p.currentStats.meshes);
            p.currentStats.meshTriangles += mesh.triangles.size();

            auto shader = p.shaders["colorMesh"];
            auto bindingSet = shader->createBindingSet();
            shader->useBindingSet(bindingSet);
            p.garbage[p.frameIndex].bindingSets.push_back(bindingSet);
            
            shader->bind(p.frameIndex);
            const auto transform =
                p.transform *
                math::translate(
                    math::Vector3f(position.x, position.y, 0.F));
            shader->setUniform(
                "transform.mvp", transform, vlk::kShaderVertex);
            bindDescriptorSets(pipelineLayoutName, "colorMesh");

            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            if (p.vaos["colorMesh"] && p.vbos["colorMesh"])
            {
                VkPipelineLayout pipelineLayout =
                    p.pipelineLayouts[pipelineLayoutName];
                if (!pipelineLayout)
                    throw std::runtime_error("Invalid pipeline Layout '" +
                                             pipelineLayoutName + "'"); 
                vkCmdPushConstants(
                    p.cmd, pipelineLayout,
                    shader->getPushStageFlags(), 0,
                    sizeof(color), &color);
                p.vaos["colorMesh"]->draw(p.cmd, p.vbos["colorMesh"]);
            }
        }

        void Render::Private::drawTextMesh(
            Fl_Vk_Context& ctx, const geom::TriangleMesh2& mesh)
        {
            const size_t size = mesh.triangles.size();
            currentStats.textTriangles += size;
            if (size > 0)
            {
                if (!vbos["text"] ||
                    (vbos["text"] && vbos["text"]->getSize() != size * 3))
                {
                    vbos["text"] = vlk::VBO::create(
                        size * 3, vlk::VBOType::Pos2_F32_UV_U16);
                    vaos["text"].reset();
                }
                if (vbos["text"])
                {
                    vbos["text"]->copy(convert(mesh, vbos["text"]->getType()));
                }
                if (!vaos["text"] && vbos["text"])
                {
                    vaos["text"] = vlk::VAO::create(ctx);
                }
                if (vaos["text"] && vbos["text"])
                {
                    vaos["text"]->bind(frameIndex);
                    vaos["text"]->draw(cmd, vbos["text"]);
                }
            }
        }

        void Render::drawText(
            const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
            const math::Vector2i& pos, const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.text);

            return;

            p.shaders["text"]->bind(p.frameIndex);
            // p.shaders["text"]->setUniform("color", color);

            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            uint8_t textureIndex = 0;
            // const auto textures = p.glyphTextureAtlas->getTextures();
            // p.shaders["text"]->setTexture("textureSampler",
            //                               textures[textureIndex]);

            int x = 0;
            int32_t rsbDeltaPrev = 0;
            geom::TriangleMesh2 mesh;
            size_t meshIndex = 0;
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
                                glyph->image, item);
                            p.glyphIDs[glyph->info] = id;
                        }
                        if (item.textureIndex != textureIndex)
                        {
                            textureIndex = item.textureIndex;
                            // glBindTexture(
                            //     GL_TEXTURE_2D, textures[textureIndex]);

                            p.drawTextMesh(ctx, mesh);
                            mesh = geom::TriangleMesh2();
                            meshIndex = 0;
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
            p.drawTextMesh(ctx, mesh);
        }

        void Render::drawTexture(
            const std::shared_ptr<vlk::Texture>& texture,
            const math::Box2i& box, const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.textures);

            p.shaders["texture"]->bind(p.frameIndex);
            p.shaders["texture"]->setUniform("color", color);
            p.shaders["texture"]->setUniform("textureSampler", texture);

            if (p.vbos["texture"])
            {
                p.vbos["texture"]->copy(
                    convert(geom::box(box), p.vbos["texture"]->getType()));
            }
            if (p.vaos["texture"])
            {
                p.vaos["texture"]->bind(p.frameIndex);
                p.vaos["texture"]->draw(p.cmd, p.vbos["texture"]);
            }
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
            auto bindingSet = shader->createBindingSet();
            p.garbage[p.frameIndex].bindingSets.push_back(bindingSet);
            
            shader->bind(p.frameIndex);
            shader->setUniform("transform.mvp", p.transform);

            struct UBO
            {
                alignas(16) math::Vector4f yuvCoefficients;
                alignas(16) image::Color4f color;
                alignas(4) int32_t pixelType;
                alignas(4) int32_t videoLevels;
                alignas(4) int32_t imageChannels;
                alignas(4) int32_t mirrorX;
                alignas(4) int32_t mirrorY;
            };
            UBO ubo;
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
            bool enableBlending = false;
            VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            switch (imageOptions.alphaBlend)
            {
            case timeline::AlphaBlend::kNone:
                enableBlending = true;
                srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
                srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                break;
            case timeline::AlphaBlend::Straight:
                enableBlending = true;
                srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                break;
            case timeline::AlphaBlend::Premultiplied:
                enableBlending = true;
                srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                break;
            default:
                break;
            }

            fbo->beginRenderPass(p.cmd, "image");
            if (p.vbos["image"])
            {
                p.vbos["image"]->copy(
                    convert(geom::box(box), p.vbos["image"]->getType()));
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
            bindDescriptorSets(pipelineLayoutName, shaderName);
            fbo->setupViewportAndScissor(p.cmd);

            if (p.vaos["image"])
            {
                p.vaos["image"]->bind(p.frameIndex);
                p.vaos["image"]->draw(p.cmd, p.vbos["image"]);
            }
            fbo->endRenderPass(p.cmd);
        }

        void Render::bindDescriptorSets(
            const std::string& pipelineLayoutName, const std::string& shaderName)
        {
            TLRENDER_P();
            if (!p.shaders[shaderName])
            {
                throw std::runtime_error("Undefined shader " + shaderName);
            }
            if (!p.pipelineLayouts[pipelineLayoutName])
            {
                throw std::runtime_error(
                    "Undefined pipelineLayout " + pipelineLayoutName);
            }
            VkDescriptorSet descriptorSet = p.shaders[shaderName]->getDescriptorSet();
            vkCmdBindDescriptorSets(
                p.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                p.pipelineLayouts[pipelineLayoutName], 0, 1,
                &descriptorSet, 0, nullptr);
        }

        void Render::_createMesh(
            const std::string& meshName, const geom::TriangleMesh2& mesh)
        {
            TLRENDER_P();

            const size_t size = mesh.triangles.size();

            auto type = vlk::VBOType::Pos2_F32;
            if (mesh.t.size())
            {
                type = vlk::VBOType::Pos2_F32_UV_U16;
            }
            if (mesh.c.size())
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
            }
        }

        void Render::createPipeline(
            const std::shared_ptr<vlk::OffscreenBuffer>& fbo,
            const std::string& pipelineName,
            const std::string& pipelineLayoutName,
            const std::string& shaderName,
            const std::string& meshName, const bool enableBlending,
            const VkBlendFactor srcColorBlendFactor,
            const VkBlendFactor dstColorBlendFactor,
            const VkBlendFactor srcAlphaBlendFactor,
            const VkBlendFactor dstAlphaBlendFactor,
            const VkBlendOp colorBlendOp,
            const VkBlendOp alphaBlendOp)
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            const auto& shader = p.shaders[shaderName];
            const auto& mesh = p.vbos[meshName];

            if (!shader)
                throw std::runtime_error(
                    "createPipeline failed with unknown shader '" + shaderName +
                    "'");

            if (!mesh)
                throw std::runtime_error(
                    "createPipeline failed with unknown mesh '" + meshName +
                    "'");

            VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            if (!pipelineLayout)
            {
                pipelineLayout = _createPipelineLayout(pipelineLayoutName, shaderName);;
            }

            // Elements of new Pipeline
            vlk::VertexInputStateInfo vi;
            vi.bindingDescriptions = mesh->getBindingDescription();
            vi.attributeDescriptions = mesh->getAttributes();

            // Defaults are fine
            vlk::InputAssemblyStateInfo ia;

            // Defaults are fine
            vlk::ColorBlendStateInfo cb;

            // Defaults are fine
            vlk::RasterizationStateInfo rs;

            vlk::DepthStencilStateInfo ds;

            bool has_depth = fbo->hasDepth();     // Check if FBO has depth
            bool has_stencil = fbo->hasStencil(); // Check if FBO has stencil

            ds.depthTestEnable = has_depth ? VK_TRUE : VK_FALSE;
            ds.depthWriteEnable = has_depth ? VK_TRUE : VK_FALSE;
            ds.stencilTestEnable = has_stencil ? VK_TRUE : VK_FALSE;

            vlk::ViewportStateInfo vp;

            vlk::MultisampleStateInfo ms;

            vlk::DynamicStateInfo dynamicState;
            dynamicState.dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

            // Get the vertex and fragment shaders
            std::vector<vlk::PipelineCreationState::ShaderStageInfo>
                shaderStages(2);

            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].name = shader->getName();
            shaderStages[0].module = shader->getVertex();
            shaderStages[0].entryPoint = "main";

            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].name = shader->getName();
            shaderStages[1].module = shader->getFragment();
            shaderStages[1].entryPoint = "main";

            vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
            if (enableBlending)
            {
                colorBlendAttachment.blendEnable = VK_TRUE;
            }

            cb.attachments.push_back(colorBlendAttachment);

            //
            // Pass pipeline creation parameters to pipelineState.
            //
            vlk::PipelineCreationState pipelineState;
            pipelineState.vertexInputState = vi;
            pipelineState.inputAssemblyState = ia;
            pipelineState.colorBlendState = cb;
            pipelineState.rasterizationState = rs;
            pipelineState.depthStencilState = ds;
            pipelineState.viewportState = vp;
            pipelineState.multisampleState = ms;
            pipelineState.dynamicState = dynamicState;
            pipelineState.stages = shaderStages;
            pipelineState.renderPass = fbo->getRenderPass();
            pipelineState.layout = pipelineLayout;

            VkPipeline pipeline;
            if (p.pipelines.count(pipelineName) == 0)
            {
                pipeline = pipelineState.create(device);
                auto pair = std::make_pair(pipelineState, pipeline);
                p.pipelines[pipelineName] = pair;
            }
            else
            {
                const auto& pair = p.pipelines[pipelineName];
                const auto& oldPipelineState = pair.first;
                VkPipeline oldPipeline = pair.second;
                if (pipelineState != oldPipelineState ||
                    oldPipeline == VK_NULL_HANDLE)
                {
                    if (oldPipeline != VK_NULL_HANDLE)
                    {
                        p.garbage[p.frameIndex].pipelines.push_back(
                            oldPipeline);
                    }
                    pipeline = pipelineState.create(device);
                    auto pair = std::make_pair(pipelineState, pipeline);
                    p.pipelines[pipelineName] = pair;
                }
                else
                {
                    pipeline = pair.second;
                }
            }

            vkCmdBindPipeline(p.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            fbo->setupViewportAndScissor(p.cmd);
        }

    } // namespace timeline_vlk
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineVk/RenderPrivate.h>

#include <tlVk/Vk.h>

namespace tl
{
    namespace timeline_vlk
    {
        void
        Render::drawRect(const math::Box2i& box, const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.rects);

            p.shaders["rect"]->bind();
            p.shaders["rect"]->setUniform("color", color);

            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (p.vbos["rect"])
            {
                p.vbos["rect"]->copy(
                    convert(geom::box(box), p.vbos["rect"]->getType()));
            }
            if (p.vaos["rect"])
            {
                p.vaos["rect"]->bind();
                // p.vaos["rect"]->draw(
                //     GL_TRIANGLES, 0, p.vbos["rect"]->getSize());
            }
        }

        void Render::drawMesh(
            const geom::TriangleMesh2& mesh, const math::Vector2i& position,
            const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.meshes);
            const size_t size = mesh.triangles.size();
            p.currentStats.meshTriangles += mesh.triangles.size();
            if (size > 0)
            {
                p.shaders["mesh"]->bind();
                const auto transform =
                    p.transform *
                    math::translate(
                        math::Vector3f(position.x, position.y, 0.F));
                p.shaders["mesh"]->setUniform("transform.mvp", transform);
                p.shaders["mesh"]->setUniform("color", color);

                // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                if (!p.vbos["mesh"] ||
                    (p.vbos["mesh"] && p.vbos["mesh"]->getSize() < size * 3))
                {
                    p.vbos["mesh"] =
                        vlk::VBO::create(size * 3, vlk::VBOType::Pos2_F32);
                    p.vaos["mesh"].reset();
                }
                if (p.vbos["mesh"])
                {
                    p.vbos["mesh"]->copy(convert(mesh, vlk::VBOType::Pos2_F32));
                }

                if (!p.vaos["mesh"] && p.vbos["mesh"])
                {
                    p.vaos["mesh"] = vlk::VAO::create(ctx);
                }
                if (p.vaos["mesh"] && p.vbos["mesh"])
                {
                    p.vaos["mesh"]->bind();
                    // p.vaos["mesh"]->draw(GL_TRIANGLES, 0, size * 3);
                }
            }
        }

        void Render::drawColorMesh(
            const geom::TriangleMesh2& mesh, const math::Vector2i& position,
            const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.meshes);
            const size_t size = mesh.triangles.size();
            p.currentStats.meshTriangles += mesh.triangles.size();
            if (size > 0)
            {
                p.shaders["colorMesh"]->bind();
                const auto transform =
                    p.transform *
                    math::translate(
                        math::Vector3f(position.x, position.y, 0.F));
                p.shaders["colorMesh"]->setUniform("transform.mvp", transform);
                p.shaders["colorMesh"]->setUniform("color", color);

                // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                if (!p.vbos["colorMesh"] ||
                    (p.vbos["colorMesh"] &&
                     p.vbos["colorMesh"]->getSize() < size * 3))
                {
                    p.vbos["colorMesh"] = vlk::VBO::create(
                        size * 3, vlk::VBOType::Pos2_F32_Color_F32);
                    p.vaos["colorMesh"].reset();
                }
                if (p.vbos["colorMesh"])
                {
                    p.vbos["colorMesh"]->copy(
                        convert(mesh, vlk::VBOType::Pos2_F32_Color_F32));
                }

                if (!p.vaos["colorMesh"] && p.vbos["colorMesh"])
                {
                    p.vaos["colorMesh"] = vlk::VAO::create(ctx);
                }
                if (p.vaos["colorMesh"] && p.vbos["colorMesh"])
                {
                    p.vaos["colorMesh"]->bind();
                    // p.vaos["colorMesh"]->draw(GL_TRIANGLES, 0, size * 3);
                }
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
                    (vbos["text"] && vbos["text"]->getSize() < size * 3))
                {
                    vbos["text"] =
                        vlk::VBO::create(size * 3, vlk::VBOType::Pos2_F32_UV_U16);
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
                    vaos["text"]->bind();
                    // vaos["text"]->draw(GL_TRIANGLES, 0, size * 3);
                }
            }
        }

        void Render::drawText(
            const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
            const math::Vector2i& pos, const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.text);

            p.shaders["text"]->bind();
            p.shaders["text"]->setUniform("color", color);
            p.shaders["text"]->setUniform("textureSampler", 0);

            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
            uint8_t textureIndex = 0;
            // const auto textures = p.glyphTextureAtlas->getTextures();
            // glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);

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
            unsigned int id, const math::Box2i& box,
            const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.textures);

            p.shaders["texture"]->bind(p.frameIndex);
            p.shaders["texture"]->setUniform("color", color);
            p.shaders["texture"]->setUniform("textureSampler", 0);

            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
            // glBindTexture(GL_TEXTURE_2D, id);

            if (p.vbos["texture"])
            {
                p.vbos["texture"]->copy(
                    convert(geom::box(box), p.vbos["texture"]->getType()));
            }
            if (p.vaos["texture"])
            {
                p.vaos["texture"]->bind();
                // p.vaos["texture"]->draw(
                //     GL_TRIANGLES, 0, p.vbos["texture"]->getSize());
            }
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
            // setActiveTextures(info, textures);

            p.shaders["image"]->bind(p.frameIndex);
            
            struct Fragment
            {
                image::Color4f color;
                int pixelType;
                int videoLevels;
                math::Vector4f yuvCoefficients;
                int imageChannels;
                int mirrorX;
                int mirrorY;
            };
            Fragment frag;
            frag.color = color;
            frag.pixelType = static_cast<int>(info.pixelType);
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
            frag.videoLevels = static_cast<int>(videoLevels);
            frag.yuvCoefficients = image::getYUVCoefficients(info.yuvCoefficients);
            frag.imageChannels = image::getChannelCount(info.pixelType);
            frag.mirrorX = info.layout.mirror.x;
            frag.mirrorY = info.layout.mirror.y;
            p.shaders["image"]->setUniform("frag", frag);
            
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
            case image::PixelType::YUV_422P_U8:
            case image::PixelType::YUV_444P_U8:
            case image::PixelType::YUV_420P_U16:
            case image::PixelType::YUV_422P_U16:
            case image::PixelType::YUV_444P_U16:
                textures[0]->transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                textures[1]->transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                textures[2]->transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                p.shaders["image"]->setTexture("textureSampler0", textures[0]);
                p.shaders["image"]->setTexture("textureSampler1", textures[1]);
                p.shaders["image"]->setTexture("textureSampler2", textures[2]);
                break;
            default:
                textures[0]->transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                p.shaders["image"]->setTexture("textureSampler0", textures[0]);
                p.shaders["image"]->setTexture("textureSampler1", textures[0]);
                p.shaders["image"]->setTexture("textureSampler2", textures[0]);
                break;
            }
            switch (imageOptions.alphaBlend)
            {
            case timeline::AlphaBlend::kNone:
                // glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
                break;
            case timeline::AlphaBlend::Straight:
                // glBlendFuncSeparate(
                //     GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                //     GL_ONE_MINUS_SRC_ALPHA);
                break;
            case timeline::AlphaBlend::Premultiplied:
                // glBlendFuncSeparate(
                //     GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                //     GL_ONE_MINUS_SRC_ALPHA);
                break;
            default:
                break;
            }

            if (p.vbos["image"])
            {
                p.vbos["image"]->copy(
                    convert(geom::box(box), p.vbos["image"]->getType()));
            }


            //
            // Create pipeline
            //

            VkDevice device = ctx.device;

            auto shader = p.shaders["image"];
            
            if (!p.pipelineLayouts["image"])
            {
                VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
                pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                pPipelineLayoutCreateInfo.pNext = NULL;
                pPipelineLayoutCreateInfo.setLayoutCount = 1;
                pPipelineLayoutCreateInfo.pSetLayouts = &shader->getDescriptorSetLayout();
                
                VkPipelineLayout pipelineLayout;
                VkResult result = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo,
                                                         NULL,
                                                         &pipelineLayout);
                VK_CHECK(result);

                p.pipelineLayouts["image"] = pipelineLayout;
            }

            if (!p.pipelines["image"])
            {
            
                VkGraphicsPipelineCreateInfo pipeline;

                VkPipelineVertexInputStateCreateInfo vi = {};
                VkPipelineInputAssemblyStateCreateInfo ia;
                VkPipelineRasterizationStateCreateInfo rs;
                VkPipelineColorBlendStateCreateInfo cb;
                VkPipelineDepthStencilStateCreateInfo ds;
                VkPipelineViewportStateCreateInfo vp;
                VkPipelineMultisampleStateCreateInfo ms;
                VkDynamicState dynamicStateEnables[(VK_DYNAMIC_STATE_STENCIL_REFERENCE - VK_DYNAMIC_STATE_VIEWPORT + 1)];
                VkPipelineDynamicStateCreateInfo dynamicState;


                memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
                memset(&dynamicState, 0, sizeof dynamicState);
                dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamicState.pDynamicStates = dynamicStateEnables;

                memset(&pipeline, 0, sizeof(pipeline));
                pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipeline.layout = p.pipelineLayouts["image"]; 

                vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                vi.pNext = NULL;
                vi.vertexBindingDescriptionCount = 1;
                vi.pVertexBindingDescriptions = p.vbos["image"]->getBindingDescription(); // Use FBO mesh binding
                vi.vertexAttributeDescriptionCount = p.vbos["image"]->getAttributes().size();
                vi.pVertexAttributeDescriptions = p.vbos["image"]->getAttributes().data();

                memset(&ia, 0, sizeof(ia));
                ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

                memset(&rs, 0, sizeof(rs));
                rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rs.polygonMode = VK_POLYGON_MODE_FILL;
                rs.cullMode = VK_CULL_MODE_NONE;
                rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
                rs.depthClampEnable = VK_FALSE;
                rs.rasterizerDiscardEnable = VK_FALSE;
                rs.depthBiasEnable = VK_FALSE;
                rs.lineWidth = 1.0f;

                memset(&cb, 0, sizeof(cb));
                cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                VkPipelineColorBlendAttachmentState att_state[1];
                memset(att_state, 0, sizeof(att_state));
                att_state[0].colorWriteMask = 0xf;
                att_state[0].blendEnable = VK_FALSE;
                cb.attachmentCount = 1;
                cb.pAttachments = att_state;

                memset(&vp, 0, sizeof(vp));
                vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                vp.viewportCount = 1;
                dynamicStateEnables[dynamicState.dynamicStateCount++] =
                    VK_DYNAMIC_STATE_VIEWPORT;
                vp.scissorCount = 1;
                dynamicStateEnables[dynamicState.dynamicStateCount++] =
                    VK_DYNAMIC_STATE_SCISSOR;

                bool has_depth = p.fbo->hasDepth(); // Check FBO depth
                bool has_stencil = p.fbo->hasStencil(); // Check FBO stencil

                memset(&ds, 0, sizeof(ds));
                ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                ds.depthTestEnable = has_depth ? VK_TRUE : VK_FALSE;
                ds.depthWriteEnable = has_depth ? VK_TRUE : VK_FALSE;
                ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                ds.depthBoundsTestEnable = VK_FALSE;
                ds.stencilTestEnable = has_stencil ? VK_TRUE : VK_FALSE;
                ds.back.failOp = VK_STENCIL_OP_KEEP;
                ds.back.passOp = VK_STENCIL_OP_KEEP;
                ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
                ds.front = ds.back;

                memset(&ms, 0, sizeof(ms));
                ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                ms.pSampleMask = NULL;
                ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

                // Two stages: vs and fs
                pipeline.stageCount = 2;
                VkPipelineShaderStageCreateInfo shaderStages[2];
                memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

                shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
                shaderStages[0].module = shader->getVertex();
                shaderStages[0].pName = "main";

                shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                shaderStages[1].module = shader->getFragment();
                shaderStages[1].pName = "main";

                pipeline.pVertexInputState = &vi;
                pipeline.pInputAssemblyState = &ia;
                pipeline.pRasterizationState = &rs;
                pipeline.pColorBlendState = &cb;
                pipeline.pMultisampleState = &ms;
                pipeline.pViewportState = &vp;
                pipeline.pDepthStencilState = &ds;
                pipeline.pStages = shaderStages;
                pipeline.renderPass = p.fbo->getRenderPass(); // Use FBO's render pass
                pipeline.pDynamicState = &dynamicState;

                // Create a temporary pipeline cache
                VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
                pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
                VkPipelineCache pipelineCache;
                VkResult result;
                result = vkCreatePipelineCache(device, &pipelineCacheCreateInfo, NULL,
                                               &pipelineCache);
                VK_CHECK(result);

                VkPipeline graphicsPipeline;
                result = vkCreateGraphicsPipelines(device, pipelineCache, 1,
                                                   &pipeline, NULL, &graphicsPipeline);
                VK_CHECK(result);

                // Destroy the temporary pipeline cache
                vkDestroyPipelineCache(device, pipelineCache, NULL);
                p.pipelines["image"] = graphicsPipeline;
            }
            
            vkCmdBindPipeline(p.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              p.pipelines["image"]);
            
            vkCmdBindDescriptorSets(p.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    p.pipelineLayouts["image"], 0, 1,
                                    &shader->getDescriptorSet(), 0, nullptr);

            // VkViewport viewport = {};
            // viewport.width = static_cast<float>(p.renderSize.w);
            // viewport.height = static_cast<float>(p.renderSize.h);
            // viewport.minDepth = 0.0f;
            // viewport.maxDepth = 1.0f;
            // vkCmdSetViewport(p.cmd, 0, 1, &viewport);

            // VkRect2D scissor = {};
            // scissor.extent = p.fbo->getExtent();
            
            // vkCmdSetScissor(p.cmd, 0, 1, &scissor);
    

            if (p.vaos["image"])
            {
                p.vaos["image"]->upload(p.vbos["image"]->getData());
                p.vaos["image"]->draw(p.cmd, p.vbos["image"]);
            }
        }
    } // namespace timeline_vlk
} // namespace tl

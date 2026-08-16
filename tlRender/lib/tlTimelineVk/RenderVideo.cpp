// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlTimelineVk/RenderPrivate.h>
#include <tlTimelineVk/RenderStructs.h>

#include <tlVk/Vk.h>
#include <tlVk/Mesh.h>
#include <tlVk/Util.h>

#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>

#include <random>

namespace tl
{
    namespace timeline_vlk
    {
        namespace
        {
            // The filters for one of the images being drawn, which is where
            // they live now: how a picture is sampled is a fact about the
            // picture, not about the colours it is shown in.
            timeline::ImageFilters imageFilters(
                const std::vector<timeline::ImageOptions>& imageOptions,
                size_t index)
            {
                return index < imageOptions.size() ?
                    imageOptions[index].imageFilters :
                    timeline::ImageFilters();
            }
        }

        void Render::drawVideo(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            const timeline::BackgroundOptions& backgroundOptions)
        {
            TLRENDER_P();

            if (!videoFrame.empty() && !videoFrame.front().layers.empty())
            {
                _drawBackground(boxes, backgroundOptions);
            }
            switch (compareOptions.mode)
            {
            case timeline::CompareMode::A:
                _drawVideoA(videoFrame, boxes, imageOptions, displayOptions, compareOptions);
                break;
            case timeline::CompareMode::B:
                _drawVideoB(videoFrame, boxes, imageOptions, displayOptions, compareOptions);
                break;
            case timeline::CompareMode::Wipe:
                _drawVideoWipe(videoFrame, boxes, imageOptions, displayOptions, compareOptions);
                break;
            case timeline::CompareMode::Overlay:
                _drawVideoOverlay(videoFrame, boxes, imageOptions, displayOptions, compareOptions);
                break;
            case timeline::CompareMode::Difference:
                if (videoFrame.size() > 1)
                {
                    _drawVideoDifference(videoFrame, boxes, imageOptions, displayOptions, compareOptions);
                }
                else
                {
                    _drawVideoA(videoFrame, boxes, imageOptions, displayOptions, compareOptions);
                }
                break;
            case timeline::CompareMode::Multiply:
                if (videoFrame.size() > 1)
                {
                    _drawVideoMultiply(videoFrame, boxes, imageOptions, displayOptions, compareOptions);
                }
                else
                {
                    _drawVideoA(videoFrame, boxes, imageOptions, displayOptions, compareOptions);
                }
                break;
            case timeline::CompareMode::Add:
                if (videoFrame.size() > 1)
                {
                    _drawVideoAdd(videoFrame, boxes, imageOptions, displayOptions, compareOptions);
                }
                else
                {
                    _drawVideoA(videoFrame, boxes, imageOptions, displayOptions, compareOptions);
                }
                break;
            case timeline::CompareMode::Butterfly:
                _drawVideoButterfly(
                    videoFrame, boxes, imageOptions, displayOptions,
                    compareOptions);
                break;
            case timeline::CompareMode::Horizontal:
            case timeline::CompareMode::Vertical:
            case timeline::CompareMode::Tile:
                _drawVideoTile(videoFrame, boxes, imageOptions, displayOptions, compareOptions);
                break;
            default:
                break;
            }
        }

        void Render::_drawBackground(const std::vector<math::Box2i>& boxes,
                                     const timeline::BackgroundOptions& options)
        {
            TLRENDER_P();

            image::Color4f color(1.F, 1.F, 1.F);
            for (const auto& box : boxes)
            {
                switch (options.type)
                {
                case timeline::Background::Transparent:
                    break;
                case timeline::Background::Solid:
                {
                    p.fbo->transitionToColorAttachment(p.cmd);
                    p.fbo->beginClearRenderPass(p.cmd);
                    drawRect(box, options.color0);
                    p.fbo->endRenderPass(p.cmd);
                    p.fbo->transitionToShaderRead(p.cmd);
                    break;
                }
                case timeline::Background::Checkers:
                {
                    geom::TriangleMesh2 mesh = geom::checkers(box,
                                                              options.color0,
                                                              options.color1,
                                                              options.checkersSize);
                    _create2DMesh("colorMesh", mesh);
                    createPipeline(p.fbo, "checkers", "checkers", "colorMesh", "colorMesh");
                    VkPipelineLayout pipelineLayout = p.pipelineLayouts["checkers"];
                    vkCmdPushConstants(
                        p.cmd, pipelineLayout,
                        p.shaders["colorMesh"]->getPushStageFlags(), 0,
                        sizeof(color), &color);

                    p.fbo->transitionToColorAttachment(p.cmd);
                    p.fbo->beginClearRenderPass(p.cmd);
                    drawColorMesh("checkers", mesh, math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));
                    p.fbo->endRenderPass(p.cmd);
                    p.fbo->transitionToShaderRead(p.cmd);
                    break;
                }
                case timeline::Background::Gradient:
                {
                    geom::TriangleMesh2 mesh;
                    mesh.v.push_back(math::Vector2f(box.min.x, box.min.y));
                    mesh.v.push_back(math::Vector2f(box.max.x, box.min.y));
                    mesh.v.push_back(math::Vector2f(box.max.x, box.max.y));
                    mesh.v.push_back(math::Vector2f(box.min.x, box.max.y));
                    mesh.c.push_back(math::Vector4f(options.color0.r, options.color0.g, options.color0.b, options.color0.a));
                    mesh.c.push_back(math::Vector4f(options.color1.r, options.color1.g, options.color1.b, options.color1.a));
                    mesh.triangles.push_back({
                            geom::Vertex2(1, 0, 1),
                            geom::Vertex2(2, 0, 1),
                            geom::Vertex2(3, 0, 2),
                        });
                    mesh.triangles.push_back({
                            geom::Vertex2(3, 0, 2),
                            geom::Vertex2(4, 0, 2),
                            geom::Vertex2(1, 0, 1),
                        });
                    _create2DMesh("colorMesh", mesh);
                    createPipeline(p.fbo, "gradient", "gradient", "colorMesh", "colorMesh");
                    VkPipelineLayout pipelineLayout = p.pipelineLayouts["gradient"];
                    vkCmdPushConstants(
                        p.cmd, pipelineLayout,
                        p.shaders["colorMesh"]->getPushStageFlags(), 0,
                        sizeof(color), &color);
                    p.fbo->transitionToColorAttachment(p.cmd);
                    p.fbo->beginClearRenderPass(p.cmd);
                    drawColorMesh("gradient", mesh, math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));
                    p.fbo->endRenderPass(p.cmd);
                    p.fbo->transitionToShaderRead(p.cmd);
                    break;
                }
                default:
                    break;
                }
            }
        }

        void Render::drawMask(const float pct)
        {
            if (pct < 0.001F)
                return;

            TLRENDER_P();

            const math::Size2i renderSize = p.fbo->getSize();
            if (!renderSize.isValid())
                return;

            const image::Color4f color(0.F, 0.F, 0.F);
            p.fbo->transitionToColorAttachment(p.cmd);
            float aspectY = (float)renderSize.w / (float)renderSize.h;
            float aspectX = (float)renderSize.h / (float)renderSize.w;

            float target_aspect = 1.F / pct;
            float amountY = (0.5F - target_aspect * aspectY / 2);
            float amountX = (0.5F - pct * aspectX / 2);

            bool vertical = true;
            if (amountY < amountX)
            {
                vertical = false;
            }

            if (vertical)
            {
                int Y = renderSize.h * amountY;
                math::Box2i box(0, 0, renderSize.w, Y);
                p.fbo->transitionToColorAttachment(p.cmd);
                p.fbo->beginLoadRenderPass(p.cmd);
                drawRect(box, color);
                box.max.y = renderSize.h;
                box.min.y = renderSize.h - Y;
                drawRect(box, color);
                p.fbo->endRenderPass(p.cmd);
            }
            else
            {
                int X = renderSize.w * amountX;
                math::Box2i box(0, 0, X, renderSize.h);
                p.fbo->transitionToColorAttachment(p.cmd);
                p.fbo->beginLoadRenderPass(p.cmd);
                drawRect(box, color);
                box.max.x = renderSize.w;
                box.min.x = renderSize.w - X;
                drawRect(box, color);
                p.fbo->endRenderPass(p.cmd);
            }
            p.fbo->transitionToShaderRead(p.cmd);
        }

        void Render::_drawVideoA(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (!videoFrame.empty() && !boxes.empty())
            {
                _drawVideo(
                    p.fbo, "display",
                    videoFrame[0], boxes[0], !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
            }
        }

        void Render::_drawVideoB(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (videoFrame.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    p.fbo, "display",
                    videoFrame[1], boxes[1], imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
            }
        }

        void Render::_drawVideoWipe(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            auto wipeShader = p.shaders["wipe"];
            auto textureShader = p.shaders["texture"];


            image::Color4f color(1.F, 0.F, 0.F);
            VkPipelineLayout pipelineLayout;
            std::string pipelineLayoutName = "wipe_left";
            const math::Size2i& offscreenBufferSize = p.fbo->getSize();
            vlk::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            if (!displayOptions.empty())
            {
                offscreenBufferOptions.colorFilters =
                    imageFilters(imageOptions, 0);
            }
            offscreenBufferOptions.depth = vlk::OffscreenDepth::kNone;
            offscreenBufferOptions.stencil = vlk::OffscreenStencil::kNone;
            offscreenBufferOptions.clear = false;
            if (doCreate(p.buffers["wipe_image"], offscreenBufferSize,
                         offscreenBufferOptions))
            {
                if (p.buffers["wipe_image"])
                    p.garbage[p.frameIndex].buffers.push_back(std::move(p.buffers["wipe_image"]));
                p.buffers["wipe_image"] =
                    vlk::OffscreenBuffer::create(ctx, offscreenBufferSize,
                                                 offscreenBufferOptions);
            }

            float radius = 0.F;
            float x = 0.F;
            float y = 0.F;
            if (!boxes.empty())
            {
                radius = std::max(boxes[0].w(), boxes[0].h()) * 2.5F;
                x = boxes[0].w() * compareOptions.wipeCenter.x;
                y = boxes[0].h() * compareOptions.wipeCenter.y;
            }
            const float rotation = compareOptions.wipeRotation;
            math::Vector2f pts[4];
            for (size_t i = 0; i < 4; ++i)
            {
                float rad = math::deg2rad(rotation + 90.F * i + 90.F);
                pts[i].x = cos(rad) * radius + x;
                pts[i].y = sin(rad) * radius + y;
            }

            // Main FBO Transitions
            p.fbo->transitionToColorAttachment(p.cmd);
            p.fbo->transitionDepthToStencilAttachment(p.cmd);


            // Draw left image to "wipe" buffer
            if (!videoFrame.empty() && !boxes.empty())
            {
                p.buffers["wipe_image"]->transitionToColorAttachment(p.cmd);
                p.buffers["wipe_image"]->beginClearRenderPass(p.cmd);
                p.buffers["wipe_image"]->endRenderPass(p.cmd);

                _drawVideo(
                    p.buffers["wipe_image"], "display",
                    videoFrame[0], boxes[0],
                    !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());

                p.buffers["wipe_image"]->transitionToShaderRead(p.cmd);
            }

            // Draw stencil triangle mesh
            if (p.vbos["wipe"])
            {
                geom::TriangleMesh2 mesh;
                mesh.v.push_back(pts[0]);
                mesh.v.push_back(pts[1]);
                mesh.v.push_back(pts[2]);
                geom::Triangle2 tri;
                tri.v[0] = 1;
                tri.v[1] = 2;
                tri.v[2] = 3;
                mesh.triangles.push_back(tri);
                p.vbos["wipe"]->copy(convert(mesh, p.vbos["wipe"]->getType()));
            }

            // ----- FIRST RENDER PASS OF LEFT VIDEO
            p.fbo->beginLoadRenderPass(p.cmd);


            pipelineLayoutName = "wipe_left_stencil";
            {
                vlk::ColorBlendStateInfo cb;
                vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
                colorBlendAttachment.blendEnable = VK_TRUE;
                colorBlendAttachment.colorWriteMask = 0;
                cb.attachments.push_back(colorBlendAttachment);

                vlk::DepthStencilStateInfo ds;
                ds.depthTestEnable = VK_FALSE;

#if USE_DYNAMIC_STENCILS
                ctx.vkCmdSetStencilTestEnableEXT(p.cmd, VK_TRUE);
                ctx.vkCmdSetStencilOpEXT(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_REPLACE,
                                         VK_STENCIL_OP_KEEP,
                                         VK_COMPARE_OP_ALWAYS);

                vkCmdSetStencilWriteMask(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         0xFF);
                vkCmdSetStencilCompareMask(p.cmd,
                                           VK_STENCIL_FACE_FRONT_AND_BACK,
                                           0xFF);
                vkCmdSetStencilReference(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK, 1);
#else
                ds.stencilTestEnable = VK_TRUE;

                VkStencilOpState stencilOp = {};
                stencilOp.failOp = VK_STENCIL_OP_KEEP;
                stencilOp.passOp = VK_STENCIL_OP_REPLACE;
                stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
                stencilOp.compareOp = VK_COMPARE_OP_ALWAYS;
                stencilOp.compareMask = 0xFF;
                stencilOp.writeMask = 0xFF;
                stencilOp.reference = 1;
                ds.front = ds.back = stencilOp;
#endif

                // Draw left stencil mask
                createPipeline("wipe_left_stencil", pipelineLayoutName,
                               p.fbo->getLoadRenderPass(),
                               wipeShader, p.vbos["wipe"],
                               cb, ds);
            }

            pipelineLayout = p.pipelineLayouts[pipelineLayoutName];

            _createBindingSet(wipeShader);
            vkCmdPushConstants(p.cmd, pipelineLayout,
                               wipeShader->getPushStageFlags(), 0,
                               sizeof(color), &color);
            wipeShader->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);
            _bindDescriptorSets(pipelineLayoutName, wipeShader);

            _vkDraw("wipe");


            // Draw video
            pipelineLayoutName = "wipe_left_image";


            if (p.vbos["video"] && !boxes.empty())
            {
                p.vbos["video"]->copy(convert(geom::box(boxes[0], true),
                                              p.vbos["video"]->getType()));
            }

            {
                vlk::ColorBlendStateInfo cb;
                vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
                colorBlendAttachment.blendEnable = VK_TRUE;
                cb.attachments.push_back(colorBlendAttachment);

                vlk::DepthStencilStateInfo ds;
                ds.depthTestEnable = VK_FALSE;

#if USE_DYNAMIC_STENCILS
                ctx.vkCmdSetStencilTestEnableEXT(p.cmd, VK_TRUE);
                ctx.vkCmdSetStencilOpEXT(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_KEEP,
                                         VK_COMPARE_OP_EQUAL);
                vkCmdSetStencilCompareMask(p.cmd,
                                           VK_STENCIL_FACE_FRONT_AND_BACK,
                                           0xFF);
                vkCmdSetStencilWriteMask(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         0x00);
                vkCmdSetStencilReference(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK, 1);
#else
                ds.stencilTestEnable = VK_TRUE;

                VkStencilOpState stencilOp = {};
                stencilOp.failOp = VK_STENCIL_OP_KEEP;
                stencilOp.passOp = VK_STENCIL_OP_KEEP;
                stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
                stencilOp.compareOp = VK_COMPARE_OP_EQUAL;
                stencilOp.reference = 1;
                stencilOp.compareMask = 0xFF;
                stencilOp.writeMask = 0x00;
                ds.front = ds.back = stencilOp;
#endif

                createPipeline("wipe_image1",
                               pipelineLayoutName,
                               p.fbo->getLoadRenderPass(),  // \note: was clearRenderPass
                               textureShader,
                               p.vbos["video"],
                               cb, ds);
            }


            pipelineLayout = p.pipelineLayouts[pipelineLayoutName];

            _createBindingSet(textureShader);
            color = image::Color4f(1.F, 1.F, 1.F);
            vkCmdPushConstants(p.cmd, pipelineLayout,
                               textureShader->getPushStageFlags(), 0,
                               sizeof(color), &color);
            textureShader->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);
            textureShader->setFBO("textureSampler",
                                  p.buffers["wipe_image"]);
            _bindDescriptorSets(pipelineLayoutName, textureShader);

            _vkDraw("video");

            p.fbo->endRenderPass(p.cmd);
            // END FIRST RENDER PASS

            // ----- SECOND RENDER PASS OF RIGHT VIDEO

            // Draw right image to "wipe" buffer

            p.buffers["wipe_image"]->transitionToColorAttachment(p.cmd);
            p.buffers["wipe_image"]->beginClearRenderPass(p.cmd);
            p.buffers["wipe_image"]->endRenderPass(p.cmd);

            if (videoFrame.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    p.buffers["wipe_image"], "display",
                    videoFrame[1], boxes[1],
                    !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
            }

            p.buffers["wipe_image"]->transitionToShaderRead(p.cmd);

            if (p.vbos["wipe"])
            {
                geom::TriangleMesh2 mesh;
                mesh.v.push_back(pts[2]);
                mesh.v.push_back(pts[3]);
                mesh.v.push_back(pts[0]);
                geom::Triangle2 tri;
                tri.v[0] = 1;
                tri.v[1] = 2;
                tri.v[2] = 3;
                mesh.triangles.push_back(tri);
                p.vbos["wipe"]->copy(convert(mesh, p.vbos["wipe"]->getType()));
            }


            p.fbo->transitionToColorAttachment(p.cmd);
            p.fbo->transitionDepthToStencilAttachment(p.cmd);

            p.fbo->beginLoadRenderPass(p.cmd);

            pipelineLayoutName = "wipe_right_stencil";

            {
                vlk::ColorBlendStateInfo cb;
                vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
                colorBlendAttachment.blendEnable = VK_TRUE;
                colorBlendAttachment.colorWriteMask = 0;
                cb.attachments.push_back(colorBlendAttachment);

                vlk::DepthStencilStateInfo ds;
                ds.depthTestEnable = VK_FALSE;

#if USE_DYNAMIC_STENCILS
                ctx.vkCmdSetStencilTestEnableEXT(p.cmd, VK_TRUE);
                ctx.vkCmdSetStencilOpEXT(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_REPLACE,
                                         VK_STENCIL_OP_KEEP,
                                         VK_COMPARE_OP_ALWAYS);
                vkCmdSetStencilCompareMask(p.cmd,
                                           VK_STENCIL_FACE_FRONT_AND_BACK,
                                           0xFF);
                vkCmdSetStencilWriteMask(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         0xFF);
                vkCmdSetStencilReference(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK, 1);
#else
                ds.stencilTestEnable = VK_TRUE;

                VkStencilOpState stencilOp = {};

                stencilOp.failOp = VK_STENCIL_OP_KEEP;
                stencilOp.passOp = VK_STENCIL_OP_REPLACE;
                stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
                stencilOp.compareOp = VK_COMPARE_OP_ALWAYS;
                stencilOp.compareMask = 0xFF;
                stencilOp.writeMask = 0xFF;
                stencilOp.reference = 1;
                ds.front = ds.back = stencilOp;
#endif

                // Draw left stencil mask
                createPipeline("wipe_right_stencil", pipelineLayoutName,
                               p.fbo->getLoadRenderPass(),
                               wipeShader, p.vbos["wipe"],
                               cb, ds);
            }

            pipelineLayout = p.pipelineLayouts[pipelineLayoutName];

            _createBindingSet(wipeShader);
            color = image::Color4f(0.F, 1.F, 0.F);
            vkCmdPushConstants(p.cmd, pipelineLayout,
                               wipeShader->getPushStageFlags(), 0,
                               sizeof(color), &color);
            wipeShader->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);
            _bindDescriptorSets(pipelineLayoutName, wipeShader);

            _vkDraw("wipe");

            // Draw video
            pipelineLayoutName = "wipe_right_image";


            if (p.vbos["video"] && !boxes.empty())
            {
                p.vbos["video"]->copy(convert(geom::box(boxes[0], true),
                                              p.vbos["video"]->getType()));
            }

            {
                vlk::ColorBlendStateInfo cb;
                vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
                colorBlendAttachment.blendEnable = VK_TRUE;
                cb.attachments.push_back(colorBlendAttachment);

                vlk::DepthStencilStateInfo ds;
                ds.depthTestEnable = VK_FALSE;

#if USE_DYNAMIC_STENCILS
                ctx.vkCmdSetStencilTestEnableEXT(p.cmd, VK_TRUE);
                ctx.vkCmdSetStencilOpEXT(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_KEEP,
                                         VK_COMPARE_OP_EQUAL);
                vkCmdSetStencilCompareMask(p.cmd,
                                           VK_STENCIL_FACE_FRONT_AND_BACK,
                                           0xFF);
                vkCmdSetStencilWriteMask(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         0x00);
                vkCmdSetStencilReference(p.cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                         1);
#else
                ds.stencilTestEnable = VK_TRUE;

                VkStencilOpState stencilOp = {};
                stencilOp.failOp = VK_STENCIL_OP_KEEP;
                stencilOp.passOp = VK_STENCIL_OP_KEEP;
                stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
                stencilOp.compareOp = VK_COMPARE_OP_EQUAL;
                stencilOp.compareMask = 0xFF;
                stencilOp.writeMask = 0x00;
                stencilOp.reference = 1;

                ds.front = stencilOp;
                ds.back = stencilOp;
#endif

                createPipeline("wipe_right_image",
                               pipelineLayoutName,
                               p.fbo->getLoadRenderPass(),
                               textureShader,
                               p.vbos["video"],
                               cb, ds);
            }


            pipelineLayout = p.pipelineLayouts[pipelineLayoutName];

            _createBindingSet(textureShader);
            color = image::Color4f(1.F, 1.F, 1.F);
            vkCmdPushConstants(p.cmd, pipelineLayout,
                               textureShader->getPushStageFlags(), 0,
                               sizeof(color), &color);
            textureShader->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);
            textureShader->setFBO("textureSampler", p.buffers["wipe_image"]);
            _bindDescriptorSets(pipelineLayoutName, textureShader);

            _vkDraw("video");

            p.fbo->endRenderPass(p.cmd);

            // END SECOND RENDER PASS

            p.fbo->transitionToShaderRead(p.cmd);

            // Transition buffer back to color attachment
            p.buffers["wipe_image"]->transitionToColorAttachment(p.cmd);

        }

        void Render::_drawVideoOverlay(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (videoFrame.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    p.fbo, "display",
                    videoFrame[1], boxes[1],
                    imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
            }

            if (!videoFrame.empty() && !boxes.empty())
            {
                const math::Size2i offscreenBufferSize(boxes[0].w(), boxes[0].h());
                vlk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
                offscreenBufferOptions.depth = vlk::OffscreenDepth::kNone;
                offscreenBufferOptions.stencil = vlk::OffscreenStencil::kNone;
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        imageFilters(imageOptions, 0);
                }
                if (doCreate(p.buffers["overlay"], offscreenBufferSize, offscreenBufferOptions))
                {
                    if (p.buffers["overlay"])
                        p.garbage[p.frameIndex].buffers.push_back(std::move(p.buffers["texture"]));
                    p.buffers["overlay"] = vlk::OffscreenBuffer::create(ctx, offscreenBufferSize,
                                                                        offscreenBufferOptions);
                }

                p.buffers["overlay"]->transitionToColorAttachment(p.cmd);
                p.buffers["overlay"]->beginClearRenderPass(p.cmd);
                p.buffers["overlay"]->endRenderPass(p.cmd);

                if (p.buffers["overlay"])
                {
                    _drawVideo(
                        p.buffers["overlay"], "display",
                        videoFrame[0], math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                }

                if (p.buffers["overlay"])
                {

                    p.buffers["overlay"]->transitionToShaderRead(p.cmd);

                    auto textureShader = p.shaders["texture"];
                    _createBindingSet(textureShader);
                    textureShader->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);

                    const image::Color4f color = image::Color4f(1.F, 1.F, 1.F, compareOptions.overlay);

                    const std::string pipelineName = "texture";
                    const std::string shaderName = "texture";
                    const std::string meshName = "video";
                    const std::string pipelineLayoutName = shaderName;
                    createPipeline(p.fbo, pipelineName,
                                   pipelineLayoutName,
                                   shaderName, meshName,
                                   true,
                                   VK_BLEND_FACTOR_SRC_ALPHA,
                                   VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                   VK_BLEND_FACTOR_ONE,
                                   VK_BLEND_FACTOR_ONE);

                    VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
                    vkCmdPushConstants(p.cmd, pipelineLayout,
                                       textureShader->getPushStageFlags(), 0, sizeof(color), &color);

                    p.fbo->transitionToColorAttachment(p.cmd);
                    p.fbo->beginLoadRenderPass(p.cmd);

                    textureShader->setFBO("textureSampler", p.buffers["overlay"]);

                    _bindDescriptorSets(pipelineLayoutName, textureShader);


                    if (p.vbos["video"] && !boxes.empty())
                    {
                        p.vbos["video"]->copy(convert(geom::box(boxes[0], true), p.vbos["video"]->getType()));
                    }
                    _vkDraw("video");

                    p.fbo->endRenderPass(p.cmd);
                    p.fbo->transitionToShaderRead(p.cmd);

                    // Transition buffer back to color attachment
                    p.buffers["overlay"]->transitionToColorAttachment(p.cmd);
                }
            }
        }

        void Render::_drawVideoDifference(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();
            if (videoFrame.empty() || boxes.empty())
                return;

            if (_drawVideoPair(videoFrame, boxes, imageOptions, displayOptions))
            {
                p.shaders["difference"]->bind(p.frameIndex);
                _drawVideoPairShader("difference", boxes[0]);
            }
        }

        void Render::_drawVideoMultiply(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (videoFrame.empty() || boxes.empty())
                return;

            if (_drawVideoPair(
                videoFrame, boxes, imageOptions, displayOptions))
            {
                p.shaders["multiply"]->bind(p.frameIndex);
                _drawVideoPairShader("multiply", boxes[0]);
            }
        }

        void Render::_drawVideoAdd(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (videoFrame.empty() || boxes.empty())
                return;

            if (_drawVideoPair(
                videoFrame, boxes, imageOptions, displayOptions))
            {
                p.shaders["add"]->bind(p.frameIndex);
                _drawVideoPairShader("add", boxes[0]);
            }
        }

        void Render::_drawVideoButterfly(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (videoFrame.empty() || boxes.empty())
                return;

            if (_drawVideoPair(
                videoFrame, boxes, imageOptions, displayOptions))
            {
                p.shaders["butterfly"]->bind(p.frameIndex);
                _drawVideoPairShader("butterfly", boxes[0]);
            }
        }

        void Render::_drawVideoTile(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            for (size_t i = 0; i < videoFrame.size() && i < boxes.size(); ++i)
            {
                _drawVideo(
                    p.fbo, "tile",
                    videoFrame[i], boxes[i],
                    i < imageOptions.size() ?
                    std::make_shared<timeline::ImageOptions>(imageOptions[i])
                    : nullptr,
                    i < displayOptions.size() ? displayOptions[i] : timeline::DisplayOptions());
            }
        }

        void Render::_drawVideo(
            std::shared_ptr<vlk::OffscreenBuffer>& fbo,
            const std::string& pipelineName,
            const timeline::VideoFrame& videoFrame,
            const math::Box2i& box,
            const std::shared_ptr<timeline::ImageOptions>& imageOptions,
            const timeline::DisplayOptions& displayOptions)
        {
            TLRENDER_P();

            auto textureShader = p.shaders["texture"];

            // Saving and restoring the old matrix is needed for tiling.
            math::Matrix4x4 oldTransform = p.transform;
            p.transform = math::ortho(0.F, static_cast<float>(box.w()),
                                      0.F, static_cast<float>(box.h()),
                                      -1.F, 1.F);

            const math::Size2i& offscreenBufferSize = box.getSize();

            // The box a layer occupies within the offscreen buffer. Without
            // OTIO spatial coordinates a layer fills the buffer as before;
            // with them it keeps its place in the timeline canvas, scaled to
            // the buffer.
            const auto layerBox = [&videoFrame, &offscreenBufferSize](
                const std::optional<math::Box2f>& bounds)
            {
                math::Box2i out(math::Vector2i(), offscreenBufferSize);
                if (bounds.has_value() && videoFrame.canvasSize.isValid())
                {
                    const float sx = offscreenBufferSize.w /
                        static_cast<float>(videoFrame.canvasSize.w);
                    const float sy = offscreenBufferSize.h /
                        static_cast<float>(videoFrame.canvasSize.h);
                    out = math::Box2i(
                        math::Vector2i(
                            std::lround(bounds.value().min.x * sx),
                            std::lround(bounds.value().min.y * sy)),
                        math::Vector2i(
                            std::lround(bounds.value().max.x * sx),
                            std::lround(bounds.value().max.y * sy)));
                }
                return out;
            };

            vlk::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            offscreenBufferOptions.colorFilters = imageOptions.get() ?
                                                  (*imageOptions).imageFilters :
                                                  timeline::ImageFilters();
            if (doCreate(p.buffers["video"], offscreenBufferSize, offscreenBufferOptions))
            {
                if (p.buffers["video"])
                    p.garbage[p.frameIndex].buffers.push_back(std::move(p.buffers["video"]));
                p.buffers["video"] = vlk::OffscreenBuffer::create(ctx, offscreenBufferSize,
                                                                  offscreenBufferOptions);
            }

            float d1 = 1;
            float d2 = 1;
            if (p.buffers["video"])
            {
                p.buffers["video"]->beginClearRenderPass(p.cmd);
                p.buffers["video"]->endRenderPass(p.cmd);

                bool clearRenderPass = true;
                for (const auto& layer : videoFrame.layers)
                {
                    switch (layer.transition)
                    {
                    case timeline::Transition::Dissolve:
                    {
                        if (layer.image && layer.imageB)
                        {
                            if (doCreate(p.buffers["dissolve"], offscreenBufferSize,
                                         offscreenBufferOptions))
                            {
                                if (p.buffers["dissolve"])
                                    p.garbage[p.frameIndex].buffers.push_back(std::move(p.buffers["dissolve"]));
                                p.buffers["dissolve"] = vlk::OffscreenBuffer::create(ctx, offscreenBufferSize,
                                                                                     offscreenBufferOptions);
                            }
                            if (doCreate(p.buffers["dissolve2"], offscreenBufferSize, offscreenBufferOptions))
                            {
                                if (p.buffers["dissolve2"])
                                    p.garbage[p.frameIndex].buffers.push_back(std::move(p.buffers["dissolve2"]));
                                p.buffers["dissolve2"] = vlk::OffscreenBuffer::create(ctx,
                                                                                      offscreenBufferSize,
                                                                                      offscreenBufferOptions);
                            }
                            if (p.buffers["dissolve"])
                            {
                                float v = d1 = 1.F - layer.transitionValue;
                                auto dissolveImageOptions = imageOptions.get() ?
                                                            *imageOptions :
                                                            layer.imageOptions;
                                dissolveImageOptions.alphaBlend = timeline::AlphaBlend::Straight;
                                drawImage(
                                    p.buffers["dissolve"], layer.image,
                                    getBox(
                                        layerBox(layer.bounds),
                                        layer.image->getInfo(),
                                        displayOptions.aspect),
                                    image::Color4f(1.F, 1.F, 1.F, v), dissolveImageOptions);
                            }
                            if (p.buffers["dissolve2"])
                            {
                                float v = d2 = layer.transitionValue;
                                auto dissolveImageOptions = imageOptions.get() ? *imageOptions : layer.imageOptionsB;
                                dissolveImageOptions.alphaBlend = timeline::AlphaBlend::Straight;
                                drawImage(
                                    p.buffers["dissolve2"], layer.imageB,
                                    getBox(
                                        layerBox(layer.boundsB),
                                        layer.imageB->getInfo(),
                                        displayOptions.aspect),
                                    image::Color4f(1.F, 1.F, 1.F, v), dissolveImageOptions);
                            }

                            if (p.buffers["dissolve"] && p.buffers["dissolve2"])
                            {
                                p.buffers["dissolve"]->transitionToShaderRead(p.cmd);
                                p.buffers["dissolve2"]->transitionToShaderRead(p.cmd);

                                p.buffers["video"]->transitionToColorAttachment(p.cmd);

                                // --- Common Setup ---
                                const auto transform = math::ortho(0.F, static_cast<float>(box.w()), 0.F,
                                                                   static_cast<float>(box.h()), -1.F, 1.F);
                                const image::Color4f color(1.F, 1.F, 1.F);
                                const std::string pipelineNameBase = pipelineName;
                                const std::string shaderName = "dissolve";
                                const std::string meshName = "video";
                                std::string pipelineLayoutName = shaderName; // Typically shader name determines layout

                                std::string pipelineDissolveName = pipelineNameBase + "_Pass1_NoBlend";

                                // Create or find a pipeline
                                bool enableBlending = !clearRenderPass;
                                createPipeline(p.buffers["video"],
                                               pipelineDissolveName,
                                               pipelineLayoutName,
                                               shaderName, meshName,
                                               enableBlending,
                                               VK_BLEND_FACTOR_ONE,
                                               VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                               VK_BLEND_FACTOR_ONE,
                                               VK_BLEND_FACTOR_ONE);

                                VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
                                vkCmdPushConstants(p.cmd, pipelineLayout,
                                                   textureShader->getPushStageFlags(), 0, sizeof(color), &color);

                                if (clearRenderPass)
                                    p.buffers["video"]->beginClearRenderPass(p.cmd);
                                else
                                    p.buffers["video"]->beginLoadRenderPass(p.cmd);

                                auto textureShader = p.shaders["texture"];
                                _createBindingSet(textureShader);

                                textureShader->setUniform("transform.mvp", transform, vlk::kShaderVertex);
                                textureShader->setFBO("textureSampler", p.buffers["dissolve"]);

                                _bindDescriptorSets(pipelineLayoutName,
                                                    textureShader);


                                if (p.vbos["video"])
                                {
                                    p.vbos["video"]->copy(
                                        convert(geom::box(math::Box2i(0, 0,
                                                                      offscreenBufferSize.w,
                                                                      offscreenBufferSize.h),
                                                          true), p.vbos["video"]->getType()));
                                }
                                _vkDraw("video");

                                _createBindingSet(textureShader);

                                pipelineDissolveName = pipelineNameBase + "_Pass2_BlendColorForceAlpha";
                                enableBlending = true;
                                if (clearRenderPass)
                                {
                                    createPipeline(
                                        p.buffers["video"],
                                        pipelineDissolveName,
                                        pipelineLayoutName, shaderName,
                                        meshName,
                                        enableBlending,
                                        VK_BLEND_FACTOR_ONE,
                                        VK_BLEND_FACTOR_ONE,
                                        VK_BLEND_FACTOR_ONE,
                                        VK_BLEND_FACTOR_ONE);
                                }
                                else
                                {
                                    createPipeline(
                                        p.buffers["video"],
                                        pipelineDissolveName,
                                        pipelineLayoutName, shaderName,
                                        meshName,
                                        enableBlending,
                                        VK_BLEND_FACTOR_ONE,
                                        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                        VK_BLEND_FACTOR_ONE,
                                        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
                                }
                                vkCmdPushConstants(p.cmd, pipelineLayout,
                                                   textureShader->getPushStageFlags(), 0,
                                                   sizeof(color), &color);

                                textureShader->setUniform("transform.mvp", transform,
                                                                  vlk::kShaderVertex);
                                textureShader->setFBO("textureSampler", p.buffers["dissolve2"]);
                                _bindDescriptorSets(pipelineLayoutName,
                                                    textureShader);

                                if (p.vbos["video"])
                                {
                                    p.vbos["video"]->copy(
                                        convert(geom::box(math::Box2i(0, 0,
                                                                      offscreenBufferSize.w,
                                                                      offscreenBufferSize.h), true),
                                                p.vbos["video"]->getType()));
                                }
                                _vkDraw("video");

                                p.buffers["video"]->endRenderPass(p.cmd);

                                // --- Transitions ---
                                p.buffers["dissolve"]->transitionToColorAttachment(p.cmd);
                                p.buffers["dissolve2"]->transitionToColorAttachment(p.cmd);

                                p.buffers["video"]->transitionToShaderRead(p.cmd);
                            } // end if (p.buffers["dissolve"] && p.buffers["dissolve2"])
                        }
                        else if (layer.image)
                        {
                            drawImage(
                                p.buffers["video"], layer.image,
                                getBox(
                                    layerBox(layer.bounds),
                                    layer.image->getInfo(),
                                    displayOptions.aspect),
                                image::Color4f(1.F, 1.F, 1.F, 1.F - layer.transitionValue), imageOptions.get() ? *imageOptions : layer.imageOptions,
                                clearRenderPass);
                        }
                        else if (layer.imageB)
                        {
                            drawImage(
                                p.buffers["video"], layer.imageB,
                                getBox(
                                    layerBox(layer.boundsB),
                                    layer.imageB->getInfo(),
                                    displayOptions.aspect),
                                image::Color4f(1.F, 1.F, 1.F, layer.transitionValue), imageOptions.get() ? *imageOptions : layer.imageOptionsB,
                                clearRenderPass);
                        }
                        break;
                    }
                    default:
                        if (layer.image)
                        {
                            drawImage(
                                p.buffers["video"], layer.image,
                                getBox(
                                    layerBox(layer.bounds),
                                    layer.image->getInfo(),
                                    displayOptions.aspect),
                                image::Color4f(1.F, 1.F, 1.F), imageOptions.get() ? *imageOptions : layer.imageOptions, clearRenderPass);
                        }
                        break;
                    }
                    clearRenderPass = false;
                }
            }

            if (p.buffers["video"])
            {
                // Begin the new compositing render pass.
                fbo->transitionToColorAttachment(p.cmd);

                p.buffers["video"]->transitionToShaderRead(p.cmd);

                const std::string pipelineLayoutName = "display";
                const std::string shaderName = "display";
                const std::string meshName = "video";

                const auto imgOptions = imageOptions.get() ?
                                        *imageOptions :
                                        timeline::ImageOptions();

                bool enableBlending = true;
                if (imgOptions.alphaBlend == timeline::AlphaBlend::kNone)
                    enableBlending = false;

                vlk::ColorBlendStateInfo cb;

                vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
                colorBlendAttachment.blendEnable = enableBlending ? VK_TRUE : VK_FALSE;
                colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                      VK_COLOR_COMPONENT_G_BIT |
                                                      VK_COLOR_COMPONENT_B_BIT |
                                                      VK_COLOR_COMPONENT_A_BIT;

                cb.attachments.push_back(colorBlendAttachment);

                createPipeline(pipelineName,
                               pipelineLayoutName,
                               fbo->getLoadRenderPass(),
                               p.shaders["display"],
                               p.vbos["video"],
                               cb);

                fbo->setupViewportAndScissor(p.cmd);

                _createBindingSet(p.shaders["display"]);

#if defined(TLRENDER_LIBPLACEBO)
                std::size_t pushSize = p.shaders["display"]->getPushSize();
                if (pushSize > 0)
                {
                    std::vector<uint8_t> pushData(pushSize, 0);

                    VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
                    std::size_t currentOffset = 0;
                    const pl_shader_res* res = p.placeboData->res;
                    for (int j = 0; j < 2; ++j)
                    {
                        for (int i = 0; i < res->num_variables; ++i)
                        {
                            const struct pl_shader_var& shader_var = res->variables[i];
                            const struct pl_var& var = shader_var.var;
                            const std::string glsl_type = pl_var_glsl_type_name(var);
                            const bool is_float = (glsl_type == "float");
                            if (j == 0 && is_float)
                                continue;
                            if (j == 1 && !is_float)
                                continue;

                            // Ensure the variable type is float-based
                            if (var.type != PL_VAR_FLOAT)
                            {
                                throw std::runtime_error("libplacebo created a variable that is not float");
                            }

                            const struct pl_var_layout& dst_layout = pl_std430_layout(currentOffset, &var);
                            const struct pl_var_layout& src_layout = pl_var_host_layout(0, &var);

                            memcpy_layout(pushData.data(), dst_layout, shader_var.data, src_layout);
                            currentOffset = dst_layout.offset + dst_layout.size;
                        }
                    }

                    vkCmdPushConstants(p.cmd, pipelineLayout,
                                       VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                       pushData.size(), pushData.data());
                }
#endif

                fbo->beginLoadRenderPass(p.cmd);

                auto displayShader = p.shaders["display"];
                displayShader->bind(p.frameIndex);
                displayShader->setUniform("transform.mvp", oldTransform, vlk::kShaderVertex);
                displayShader->setFBO("textureSampler", p.buffers["video"]);

#if defined(TLRENDER_LIBPLACEBO)
                if (p.placeboData && p.placeboData->pcUBOSize > 0)
                {
                    std::size_t currentOffset = 0;
                    for (const auto &shader_var : p.placeboData->pcUBOvars)
                    {
                        const struct pl_var var = shader_var.var;
                        const struct pl_var_layout dst_layout = pl_std140_layout(currentOffset, &var);
                        const struct pl_var_layout& src_layout = pl_var_host_layout(0, &var);

                        memcpy_layout(p.placeboData->pcUBOData, dst_layout,
                                      shader_var.data, src_layout);

                        currentOffset = dst_layout.offset + dst_layout.size;
                    }

                    displayShader->setUniformData("pcUBO", p.placeboData->pcUBOData,
                                                         p.placeboData->pcUBOSize);
                }
#endif

#if defined(TLRENDER_OCIO)
                if (p.ocioData && p.ocioData->icsDesc)
                {
                    _updateOCIOUniforms(p.ocioData->icsDesc);
                }
                if (p.ocioData && p.ocioData->shaderDesc)
                {
                    _updateOCIOUniforms(p.ocioData->shaderDesc);
                }
                if (p.lutData && p.lutData->shaderDesc)
                {
                    _updateOCIOUniforms(p.lutData->shaderDesc);
                }
#endif

                UBOLevels uboLevels;
                uboLevels.enabled = displayOptions.levels.enabled;
                uboLevels.inLow = displayOptions.levels.inLow;
                uboLevels.inHigh = displayOptions.levels.inHigh;
                uboLevels.gamma = displayOptions.levels.gamma;
                uboLevels.outLow = displayOptions.levels.outLow;
                uboLevels.outHigh = displayOptions.levels.outHigh;
                uboLevels.gamma = uboLevels.gamma > 0.F ? (1.F / uboLevels.gamma) : 1000000.F;
                displayShader->setUniform("uboLevels", uboLevels);

                UBONormalize uboNormalize;
                uboNormalize.enabled = displayOptions.normalize.enabled;
                uboNormalize.minimum = displayOptions.normalize.minimum;
                uboNormalize.maximum = displayOptions.normalize.maximum;

                displayShader->setUniform("uboNormalize", uboNormalize);

                UBOColor uboColor;
                const bool colorMatrixEnabled = displayOptions.color != timeline::Color() && displayOptions.color.enabled;
                uboColor.enabled = colorMatrixEnabled;
                uboColor.add = displayOptions.color.add;
                uboColor.matrix = color(displayOptions.color);
                uboColor.invert = displayOptions.color.invert;

                displayShader->setUniform("uboColor", uboColor);

                UBOOptions ubo;
                ubo.channels = static_cast<int>(displayOptions.channels);
                ubo.mirrorX = displayOptions.mirror.x;
                ubo.mirrorY = displayOptions.mirror.y;
                ubo.softClip = displayOptions.softClip.enabled ? displayOptions.softClip.value : 0.F;
                ubo.videoLevels = static_cast<int>(displayOptions.videoLevels);
                ubo.invalidValues = displayOptions.invalidValues;
                displayShader->setUniform("ubo", ubo);

#if defined(TLRENDER_OCIO)
                if (p.ocioData)
                {
                    for (const auto& texture : p.ocioData->textures)
                    {
                        displayShader->setTexture(texture->getName(),
                                                         texture);
                    }
                }
                if (p.lutData)
                {
                    for (const auto& texture : p.lutData->textures)
                    {
                        displayShader->setTexture(texture->getName(),
                                                         texture);
                    }
                }
#endif // TLRENDER_OCIO
#if defined(TLRENDER_LIBPLACEBO)
                if (p.placeboData)
                {
                    for (const auto& texture : p.placeboData->textures)
                    {
                        displayShader->setTexture(texture->getName(),
                                                         texture);
                    }
                }
#endif // TLRENDER_LIBPLACEBO

                _bindDescriptorSets(pipelineLayoutName, displayShader);

                if (p.vbos["video"])
                {
                    p.vbos["video"]->copy(convert(geom::box(box, false),
                                                  p.vbos["video"]->getType()));
                }

                // Enable clipping (scissor)
                if (p.clipRectEnabled)
                    setClipRect(p.clipRect);

                _vkDraw("video");

                fbo->endRenderPass(p.cmd);

                // Transition buffer back to color attachment
                p.buffers["video"]->transitionToColorAttachment(p.cmd);

            }

            p.transform = oldTransform;
        }

        bool Render::_drawVideoPair(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions)
        {
            TLRENDER_P();
            const math::Size2i offscreenBufferSize(boxes[0].w(), boxes[0].h());

            // Each file into a buffer of its own, drawn through the whole
            // display pipeline so that what is combined is what would have
            // been shown.
            for (size_t i = 0; i < 2; ++i)
            {
                const std::string name = string::Format("compare{0}").arg(i);
                if (i > 0 && videoFrame.size() <= i)
                {
                    if (p.buffers[name])
                        p.garbage[p.frameIndex].buffers.push_back(std::move(p.buffers[name]));
                    p.buffers[name].reset();
                    continue;
                }
                vlk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
                offscreenBufferOptions.depth = vlk::OffscreenDepth::kNone;
                if (displayOptions.size() > i)
                {
                    offscreenBufferOptions.colorFilters = imageFilters(imageOptions, i);
                }
                if (doCreate(
                    p.buffers[name],
                    offscreenBufferSize,
                    offscreenBufferOptions))
                {
                    if (p.buffers[name])
                        p.garbage[p.frameIndex].buffers.push_back(std::move(p.buffers[name]));
                    p.buffers[name] = vlk::OffscreenBuffer::create(ctx,
                                                                   offscreenBufferSize,
                                                                   offscreenBufferOptions);
                }
                if (!p.buffers[name])
                {
                    continue;
                }

                p.buffers[name]->transitionToColorAttachment(p.cmd);
                p.buffers[name]->beginClearRenderPass(p.cmd);
                p.buffers[name]->endRenderPass(p.cmd);

                const auto oldTransform = p.transform;
                _drawVideo(
                    p.buffers[name], "display",
                    videoFrame[i],
                    boxes[i],
                    imageOptions.size() > i ?
                        std::make_shared<timeline::ImageOptions>(imageOptions[i]) :
                        nullptr,
                    displayOptions.size() > i ?
                    displayOptions[i] :
                    timeline::DisplayOptions());

                // Restored because the buffer above replaced it, and the
                // caller draws with the transform it came in with.
                p.transform = oldTransform;

                p.buffers[name]->transitionToShaderRead(p.cmd);
            }

            return p.buffers["compare0"] && p.buffers["compare1"];
        }

        void Render::_drawVideoPairShader(
            const std::string& shaderName,
            const math::Box2i& box)
        {
            TLRENDER_P();

            // Transition buffers to color read
            p.buffers["compare0"]->transitionToShaderRead(p.cmd);
            p.buffers["compare1"]->transitionToShaderRead(p.cmd);

            const std::string pipelineName = shaderName;
            const std::string pipelineLayoutName = shaderName;
            const std::string meshName = "video";
            const bool enableBlending = false;
            createPipeline(p.fbo, pipelineName,
                           pipelineLayoutName, shaderName, meshName,
                           enableBlending);

            // Begin the new compositing render pass.
            p.fbo->transitionToColorAttachment(p.cmd);
            p.fbo->beginLoadRenderPass(p.cmd);

            // Prepare shaders
            auto shader = p.shaders[shaderName];
            shader->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);
            shader->setFBO("textureSampler", p.buffers["compare0"]);
            shader->setFBO("textureSamplerB", p.buffers["compare1"]);
            _bindDescriptorSets(pipelineLayoutName, shader);

            if (p.vbos["video"])
            {
                p.vbos["video"]->copy(convert(
                    geom::box(box, true),
                    p.vbos["video"]->getType()));
            }

            _vkDraw("video");

            p.fbo->endRenderPass(p.cmd);

            // Transition buffer back to color attachment
            p.buffers["compare0"]->transitionToColorAttachment(p.cmd);
            p.buffers["compare1"]->transitionToColorAttachment(p.cmd);
        }

    }   // namespace timeline_vlk
} // namespace tl

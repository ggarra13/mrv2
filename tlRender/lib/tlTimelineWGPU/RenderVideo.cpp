// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlTimelineWGPU/RenderPrivate.h>
#include <tlTimelineWGPU/RenderStructs.h>

#include <tlWGPU/WGPU.h>
#include <tlWGPU/Mesh.h>
// #include <tlWGPU/Util.h>

#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>

#include <random>

namespace tl
{
    namespace timeline_wgpu
    {
        namespace
        {
            //! Color target with the given blend factors (blend operation is
            //! always Add), or with blending disabled.  The format is filled in
            //! by Render::createPipeline() from the fbo.
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

            // WebGPU: a pipeline is set on a render pass, so every case opens
            // its pass first and only then creates/binds the pipeline.  Pass
            // state does not carry over from one pass to the next.
            for (const auto& box : boxes)
            {
                switch (options.type)
                {
                case timeline::Background::Transparent:
                    break;
                case timeline::Background::Solid:
                {
                    p.renderPass = p.fbo->beginClearRenderPass(p.cmd);
                    p.currentPipeline.clear();
                    drawRect(box, options.color0);
                    p.fbo->endRenderPass(p.renderPass);
                    p.renderPass = nullptr;
                    break;
                }
                case timeline::Background::Checkers:
                {
                    geom::TriangleMesh2 mesh = geom::checkers(box,
                                                              options.color0,
                                                              options.color1,
                                                              options.checkersSize);
                    _create2DMesh("colorMesh", mesh);

                    p.renderPass = p.fbo->beginClearRenderPass(p.cmd);
                    p.currentPipeline.clear();
                    createPipeline(p.fbo, "checkers", "checkers", "colorMesh", "colorMesh");

                    // Binds the push block and draws the "colorMesh" VBO.
                    drawColorMesh("checkers", mesh, math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));

                    p.fbo->endRenderPass(p.renderPass);
                    p.renderPass = nullptr;
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

                    p.renderPass = p.fbo->beginClearRenderPass(p.cmd);
                    p.currentPipeline.clear();
                    createPipeline(p.fbo, "gradient", "gradient", "colorMesh", "colorMesh");

                    drawColorMesh("gradient", mesh, math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));

                    p.fbo->endRenderPass(p.renderPass);
                    p.renderPass = nullptr;
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
                p.renderPass = p.fbo->beginLoadRenderPass(p.cmd);
                p.currentPipeline.clear();
                drawRect(box, color);
                box.max.y = renderSize.h;
                box.min.y = renderSize.h - Y;
                drawRect(box, color);
                p.fbo->endRenderPass(p.renderPass);
                p.renderPass = nullptr;
            }
            else
            {
                int X = renderSize.w * amountX;
                math::Box2i box(0, 0, X, renderSize.h);
                p.renderPass = p.fbo->beginLoadRenderPass(p.cmd);
                p.currentPipeline.clear();
                drawRect(box, color);
                box.max.x = renderSize.w;
                box.min.x = renderSize.w - X;
                drawRect(box, color);
                p.fbo->endRenderPass(p.renderPass);
                p.renderPass = nullptr;
            }
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

            // The wipe masks each half with the stencil buffer, so the main
            // fbo must have been created with a stencil (and thus a
            // depth/stencil format).  Without one, show A.
            if (!p.fbo->hasStencil())
            {
                _drawVideoA(videoFrame, boxes, imageOptions, displayOptions,
                            compareOptions);
                return;
            }

            auto wipeShader = p.shaders["wipe"];
            auto textureShader = p.shaders["texture"];

            const math::Size2i& offscreenBufferSize = p.fbo->getSize();
            wgpu::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            if (!displayOptions.empty())
            {
                offscreenBufferOptions.colorFilters =
                    imageFilters(imageOptions, 0);
            }
            offscreenBufferOptions.depth = wgpu::OffscreenDepth::kNone;
            offscreenBufferOptions.stencil = wgpu::OffscreenStencil::kNone;
            offscreenBufferOptions.clear = false;
            if (doCreate(p.buffers["wipe_image"], offscreenBufferSize,
                         offscreenBufferOptions))
            {
                p.buffers["wipe_image"] =
                    wgpu::OffscreenBuffer::create(ctx, offscreenBufferSize,
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

            // Composites "wipe_image" into the main fbo through a stencil
            // made of one triangle:
            //  1. draw the triangle writing 1 into the stencil, no color;
            //  2. draw the image where the stencil equals 1.
            // Stencil reference and pass state are per pass in WebGPU, and the
            // stencil is cleared by every beginLoadRenderPass(), so each half
            // gets its own pass.
            auto compositeHalf = [&](const std::string& name,
                                     const math::Vector2f& a,
                                     const math::Vector2f& b,
                                     const math::Vector2f& c)
            {
                if (!p.vbos["wipe"] || !p.vbos["video"] || boxes.empty())
                    return;

                // Stencil triangle mesh
                {
                    geom::TriangleMesh2 mesh;
                    mesh.v.push_back(a);
                    mesh.v.push_back(b);
                    mesh.v.push_back(c);
                    geom::Triangle2 tri;
                    tri.v[0] = 1;
                    tri.v[1] = 2;
                    tri.v[2] = 3;
                    mesh.triangles.push_back(tri);
                    p.vbos["wipe"]->copy(convert(mesh, p.vbos["wipe"]->getType()));
                }

                p.renderPass = p.fbo->beginLoadRenderPass(p.cmd);
                p.currentPipeline.clear();

                // 1. Stencil: always pass, replace with the reference (1),
                //    no color written.
                {
                    wgpu::ColorTargetStateInfo ct;
                    ct.writeMask = WGPUColorWriteMask_None;

                    wgpu::StencilFaceStateInfo face;
                    face.compare = WGPUCompareFunction_Always;
                    face.failOp = WGPUStencilOperation_Keep;
                    face.depthFailOp = WGPUStencilOperation_Keep;
                    face.passOp = WGPUStencilOperation_Replace;

                    // No depth test.
                    wgpu::DepthStencilStateInfo ds;
                    ds.stencilFront = face;
                    ds.stencilBack = face;
                    ds.stencilReadMask = 0xFF;
                    ds.stencilWriteMask = 0xFF;

                    createPipeline(name + "_stencil", name + "_stencil",
                                   p.fbo, wipeShader, p.vbos["wipe"], ct, ds);
                }
                wgpuRenderPassEncoderSetStencilReference(p.renderPass, 1);

                _createBindingSet(wipeShader);
                PushConstants pc;
                pc.mvp = p.transform;
                pc.color = image::Color4f(0.F, 0.F, 0.F);
                _bindDescriptorSets(p.renderPass, wipeShader, &pc);
                _vkDraw("wipe");

                // 2. Video where the stencil equals the reference.
                p.vbos["video"]->copy(convert(geom::box(boxes[0]),
                                              p.vbos["video"]->getType()));
                {
                    wgpu::StencilFaceStateInfo face;
                    face.compare = WGPUCompareFunction_Equal;
                    face.failOp = WGPUStencilOperation_Keep;
                    face.depthFailOp = WGPUStencilOperation_Keep;
                    face.passOp = WGPUStencilOperation_Keep;

                    wgpu::DepthStencilStateInfo ds;
                    ds.stencilFront = face;
                    ds.stencilBack = face;
                    ds.stencilReadMask = 0xFF;
                    ds.stencilWriteMask = 0x00;

                    // Default color target: blending off (the Vulkan code
                    // enabled it with default factors, which is a plain
                    // copy of an opaque image).
                    createPipeline(name + "_image", name + "_image", p.fbo,
                                   textureShader, p.vbos["video"],
                                   wgpu::ColorTargetStateInfo(), ds);
                }
                wgpuRenderPassEncoderSetStencilReference(p.renderPass, 1);

                _createBindingSet(textureShader);
                pc.mvp = p.transform;
                pc.color = image::Color4f(1.F, 1.F, 1.F);
                textureShader->setFBO("textureSampler", p.buffers["wipe_image"]);
                _bindDescriptorSets(p.renderPass, textureShader, &pc);
                _vkDraw("video");

                p.fbo->endRenderPass(p.renderPass);
                p.renderPass = nullptr;
            };

            // ----- LEFT: video A -> "wipe_image", then through the stencil
            p.renderPass = p.buffers["wipe_image"]->beginClearRenderPass(p.cmd);
            p.buffers["wipe_image"]->endRenderPass(p.renderPass);
            p.renderPass = nullptr;

            if (!videoFrame.empty() && !boxes.empty())
            {
                _drawVideo(
                    p.buffers["wipe_image"], "display",
                    videoFrame[0], boxes[0],
                    !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
            }
            compositeHalf("wipe_left", pts[0], pts[1], pts[2]);

            // ----- RIGHT: video B -> "wipe_image", then through the stencil
            p.renderPass = p.buffers["wipe_image"]->beginClearRenderPass(p.cmd);
            p.buffers["wipe_image"]->endRenderPass(p.renderPass);
            p.renderPass = nullptr;

            if (videoFrame.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    p.buffers["wipe_image"], "display",
                    videoFrame[1], boxes[1],
                    imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
            }
            compositeHalf("wipe_right", pts[2], pts[3], pts[0]);
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
                wgpu::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
                offscreenBufferOptions.depth = wgpu::OffscreenDepth::kNone;
                offscreenBufferOptions.stencil = wgpu::OffscreenStencil::kNone;
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        imageFilters(imageOptions, 0);
                }
                if (doCreate(p.buffers["overlay"], offscreenBufferSize, offscreenBufferOptions))
                {
                    p.buffers["overlay"] = wgpu::OffscreenBuffer::create(ctx, offscreenBufferSize,
                                                                         offscreenBufferOptions);
                }

                p.renderPass = p.buffers["overlay"]->beginClearRenderPass(p.cmd);
                p.buffers["overlay"]->endRenderPass(p.renderPass);
                p.renderPass = nullptr;

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
                    auto textureShader = p.shaders["texture"];
                    const image::Color4f color = image::Color4f(1.F, 1.F, 1.F, compareOptions.overlay);

                    const std::string pipelineName = "texture";
                    const std::string shaderName = "texture";
                    const std::string meshName = "video";
                    const std::string pipelineLayoutName = shaderName;

                    // The pass first, then the pipeline set on it.
                    p.renderPass = p.fbo->beginLoadRenderPass(p.cmd);
                    p.currentPipeline.clear();

                    createPipeline(p.fbo, pipelineName,
                                   pipelineLayoutName,
                                   shaderName, meshName,
                                   true,
                                   WGPUBlendFactor_SrcAlpha,
                                   WGPUBlendFactor_OneMinusSrcAlpha,
                                   WGPUBlendFactor_One,
                                   WGPUBlendFactor_One);

                    _createBindingSet(textureShader);
                    textureShader->setFBO("textureSampler", p.buffers["overlay"]);

                    PushConstants pc;
                    pc.mvp = p.transform;
                    pc.color = color;
                    _bindDescriptorSets(p.renderPass, textureShader, &pc);

                    if (p.vbos["video"] && !boxes.empty())
                    {
                        p.vbos["video"]->copy(convert(geom::box(boxes[0]),
                                                      p.vbos["video"]->getType()));
                    }
                    _vkDraw("video");

                    p.fbo->endRenderPass(p.renderPass);
                    p.renderPass = nullptr;
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
            std::shared_ptr<wgpu::OffscreenBuffer>& fbo,
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

            wgpu::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            offscreenBufferOptions.colorFilters = imageOptions.get() ?
                                                  (*imageOptions).imageFilters :
                                                  timeline::ImageFilters();
            if (doCreate(p.buffers["video"], offscreenBufferSize, offscreenBufferOptions))
            {
                p.buffers["video"] = wgpu::OffscreenBuffer::create(ctx, offscreenBufferSize,
                                                                  offscreenBufferOptions);
            }

            float d1 = 1;
            float d2 = 1;
            if (p.buffers["video"])
            {
                p.renderPass = p.buffers["video"]->beginClearRenderPass(p.cmd);
                p.buffers["video"]->endRenderPass(p.renderPass);
                p.renderPass = nullptr;

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
                                p.buffers["dissolve"] = wgpu::OffscreenBuffer::create(ctx, offscreenBufferSize,
                                                                                      offscreenBufferOptions);
                            }
                            if (doCreate(p.buffers["dissolve2"], offscreenBufferSize, offscreenBufferOptions))
                            {
                                p.buffers["dissolve2"] = wgpu::OffscreenBuffer::create(ctx,
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
                                // --- Common Setup ---
                                const auto transform = math::ortho(0.F, static_cast<float>(box.w()), 0.F,
                                                                   static_cast<float>(box.h()), -1.F, 1.F);
                                const image::Color4f color(1.F, 1.F, 1.F);
                                const std::string pipelineNameBase = pipelineName;
                                const std::string shaderName = "dissolve";
                                const std::string meshName = "video";
                                const std::string pipelineLayoutName = shaderName;

                                auto copyQuad = [&]()
                                {
                                    if (p.vbos["video"])
                                    {
                                        p.vbos["video"]->copy(
                                            convert(geom::box(math::Box2i(0, 0,
                                                                          offscreenBufferSize.w,
                                                                          offscreenBufferSize.h)), p.vbos["video"]->getType()));
                                    }
                                };

                                // The pass first; pipelines are set on it.
                                if (clearRenderPass)
                                    p.renderPass = p.buffers["video"]->beginClearRenderPass(p.cmd);
                                else
                                    p.renderPass = p.buffers["video"]->beginLoadRenderPass(p.cmd);
                                p.currentPipeline.clear();

                                // --- Pass 1: the first image ---
                                createPipeline(p.buffers["video"],
                                               pipelineNameBase + "_Pass1_NoBlend",
                                               pipelineLayoutName,
                                               shaderName, meshName,
                                               !clearRenderPass,
                                               WGPUBlendFactor_One,
                                               WGPUBlendFactor_OneMinusSrcAlpha,
                                               WGPUBlendFactor_One,
                                               WGPUBlendFactor_One);

                                auto textureShader = p.shaders["texture"];
                                _createBindingSet(textureShader);
                                textureShader->setFBO("textureSampler", p.buffers["dissolve"]);

                                PushConstants pc;
                                pc.mvp = transform;
                                pc.color = color;
                                _bindDescriptorSets(p.renderPass, textureShader, &pc);

                                copyQuad();
                                _vkDraw("video");

                                // --- Pass 2: the second image, added ---
                                if (clearRenderPass)
                                {
                                    createPipeline(
                                        p.buffers["video"],
                                        pipelineNameBase + "_Pass2_BlendColorForceAlpha",
                                        pipelineLayoutName, shaderName,
                                        meshName, true,
                                        WGPUBlendFactor_One,
                                        WGPUBlendFactor_One,
                                        WGPUBlendFactor_One,
                                        WGPUBlendFactor_One);
                                }
                                else
                                {
                                    createPipeline(
                                        p.buffers["video"],
                                        pipelineNameBase + "_Pass2_BlendColorForceAlpha",
                                        pipelineLayoutName, shaderName,
                                        meshName, true,
                                        WGPUBlendFactor_One,
                                        WGPUBlendFactor_OneMinusSrcAlpha,
                                        WGPUBlendFactor_One,
                                        WGPUBlendFactor_OneMinusSrcAlpha);
                                }

                                _createBindingSet(textureShader);
                                textureShader->setFBO("textureSampler", p.buffers["dissolve2"]);
                                pc.mvp = transform;
                                pc.color = color;
                                _bindDescriptorSets(p.renderPass, textureShader, &pc);

                                copyQuad();
                                _vkDraw("video");

                                p.buffers["video"]->endRenderPass(p.renderPass);
                                p.renderPass = nullptr;
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
                const std::string pipelineLayoutName = "display";
                const std::string shaderName = "display";
                const std::string meshName = "video";

                const auto imgOptions = imageOptions.get() ?
                                        *imageOptions :
                                        timeline::ImageOptions();

                bool enableBlending = true;
                if (imgOptions.alphaBlend == timeline::AlphaBlend::kNone)
                    enableBlending = false;

                // Premultiplied "over"; all color channels written.
                const wgpu::ColorTargetStateInfo ct = blendTarget(
                    enableBlending, WGPUBlendFactor_One,
                    WGPUBlendFactor_OneMinusSrcAlpha, WGPUBlendFactor_One,
                    WGPUBlendFactor_OneMinusSrcAlpha);

                // The pass first; the pipeline is set on it.
                p.renderPass = fbo->beginLoadRenderPass(p.cmd);
                p.currentPipeline.clear();

                createPipeline(pipelineName,
                               pipelineLayoutName,
                               fbo,
                               p.shaders["display"],
                               p.vbos["video"],
                               ct);

                fbo->setupViewportAndScissor(p.renderPass);

                _createBindingSet(p.shaders["display"]);

                // The display shader has no push block unless libplacebo
                // (not available on WebGPU) asks for one.
                std::vector<uint8_t> pushData;
#if defined(TLRENDER_LIBPLACEBO)
                std::size_t pushSize = p.shaders["display"]->getPushSize();
                if (pushSize > 0)
                {
                    pushData.assign(pushSize, 0);

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
                }
#endif

                auto displayShader = p.shaders["display"];
                displayShader->bind(p.frameIndex);
                displayShader->setUniform("transform.mvp", oldTransform, wgpu::kShaderVertex);
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

                _bindDescriptorSets(
                    p.renderPass, displayShader,
                    pushData.empty() ? nullptr : pushData.data());

                if (p.vbos["video"])
                {
                    p.vbos["video"]->copy(convert(geom::box(box),
                                                  p.vbos["video"]->getType()));
                }

                // Enable clipping (scissor)
                if (p.clipRectEnabled)
                    setClipRect(p.clipRect);

                _vkDraw("video");

                fbo->endRenderPass(p.renderPass);
                p.renderPass = nullptr;
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
                    p.buffers[name].reset();
                    continue;
                }
                wgpu::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
                offscreenBufferOptions.depth = wgpu::OffscreenDepth::kNone;
                if (displayOptions.size() > i)
                {
                    offscreenBufferOptions.colorFilters = imageFilters(imageOptions, i);
                }
                if (doCreate(
                    p.buffers[name],
                    offscreenBufferSize,
                    offscreenBufferOptions))
                {
                    p.buffers[name] = wgpu::OffscreenBuffer::create(ctx,
                                                                    offscreenBufferSize,
                                                                    offscreenBufferOptions);
                }
                if (!p.buffers[name])
                {
                    continue;
                }

                p.renderPass = p.buffers[name]->beginClearRenderPass(p.cmd);
                p.buffers[name]->endRenderPass(p.renderPass);
                p.renderPass = nullptr;

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
            }

            return p.buffers["compare0"] && p.buffers["compare1"];
        }

        void Render::_drawVideoPairShader(
            const std::string& shaderName,
            const math::Box2i& box)
        {
            TLRENDER_P();

            const std::string pipelineName = shaderName;
            const std::string pipelineLayoutName = shaderName;
            const std::string meshName = "video";
            const bool enableBlending = false;

            // Begin the compositing pass first; the pipeline is set on it.
            p.renderPass = p.fbo->beginLoadRenderPass(p.cmd);
            p.currentPipeline.clear();

            createPipeline(p.fbo, pipelineName,
                           pipelineLayoutName, shaderName, meshName,
                           enableBlending);

            // Prepare shaders
            auto shader = p.shaders[shaderName];

            PushConstants pc;
            pc.mvp = p.transform;
            pc.color = image::Color4f(1, 1, 1, 1);
            shader->setFBO("textureSampler", p.buffers["compare0"]);
            shader->setFBO("textureSamplerB", p.buffers["compare1"]);
            _bindDescriptorSets(p.renderPass, shader, &pc);

            if (p.vbos["video"])
            {
                p.vbos["video"]->copy(convert(
                    geom::box(box),
                    p.vbos["video"]->getType()));
            }

            _vkDraw("video");

            p.fbo->endRenderPass(p.renderPass);
            p.renderPass = nullptr;
        }

    }   // namespace timeline_wgpu
} // namespace tl

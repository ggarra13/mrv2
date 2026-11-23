// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <tlTimelineWGPU/RenderPrivate.h>
#include <tlTimelineWGPU/RenderStructs.h>

#include <tlWGPU/WGPU.h>
#include <tlWGPU/Mesh.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace timeline_wgpu
    {
        void Render::drawStereo(
            const std::vector<timeline::VideoFrame>& videoData,
            const std::vector<math::Box2i>& boxes,
            const StereoType stereoType,
            const float eyeSeparation,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (videoData.empty() || boxes.empty())
                return;

            // Each eye is composited through a stencil pattern (scanlines,
            // columns or checkers), so the main fbo must have a stencil
            // (a depth/stencil format).  Without one, show the first eye.
            if (!p.fbo->hasStencil())
            {
                _drawVideoA(videoData, boxes, imageOptions, displayOptions,
                            compareOptions);
                return;
            }

            auto wipeShader = p.shaders["wipe"];
            auto textureShader = p.shaders["texture"];

            // Per eye options, falling back to the first eye's.
            auto eyeImageOptions =
                [&](const size_t i) -> std::shared_ptr<timeline::ImageOptions>
            {
                if (imageOptions.empty())
                    return nullptr;
                return std::make_shared<timeline::ImageOptions>(
                    imageOptions[i < imageOptions.size() ? i : 0]);
            };
            auto eyeDisplayOptions = [&](const size_t i) -> timeline::DisplayOptions
            {
                if (displayOptions.empty())
                    return timeline::DisplayOptions();
                return displayOptions[i < displayOptions.size() ? i : 0];
            };

            //
            // Create offscreen buffer to draw each eye into.  (How a picture
            // is sampled now lives in its ImageOptions.)
            //
            const math::Size2i& offscreenBufferSize = p.fbo->getSize();
            wgpu::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            offscreenBufferOptions.colorFilters =
                imageOptions.empty() ? timeline::ImageFilters()
                                     : imageOptions[0].imageFilters;
            offscreenBufferOptions.depth = wgpu::OffscreenDepth::kNone;
            offscreenBufferOptions.stencil = wgpu::OffscreenStencil::kNone;
            offscreenBufferOptions.clear = false;
            if (doCreate(p.buffers["stereo_image"], offscreenBufferSize,
                         offscreenBufferOptions))
            {
                p.buffers["stereo_image"] =
                    wgpu::OffscreenBuffer::create(ctx, offscreenBufferSize,
                                                 offscreenBufferOptions);
            }

            // Draw left image to the "stereo_image" buffer.  This runs its
            // own passes, so no pass may be open here (WebGPU allows one
            // open pass per encoder).  No layout transitions are needed:
            // WebGPU tracks the texture's usage by itself.
            _drawVideo(
                p.buffers["stereo_image"], "display",
                videoData[0], boxes[0],
                eyeImageOptions(0), eyeDisplayOptions(0));

            math::Box2i  box = boxes[0];
            math::Size2i size(box.w(), box.h());

            // Create the stencil triangle mesh (scanlines, columns or
            // checkers) into the "stereo" VBO.  Returns false if there is
            // nothing to draw.
            auto createStencilMesh = [&]() -> bool
            {
                geom::TriangleMesh2 mesh;
                if (stereoType == StereoType::kScanlines)
                {
                    mesh = geom::scanlines(box);
                }
                else if (stereoType == StereoType::kColumns)
                {
                    mesh = geom::columns(box);
                }
                else if (stereoType == StereoType::kCheckers)
                {
                    size.w = size.h = 1;
                    mesh = geom::checkers(box, size);
                }
                if (mesh.triangles.empty())
                    return false;

                // Pos2_F32; reuses the VBO if the size did not change.
                _create2DMesh("stereo", mesh);
                return true;
            };

            // Composites "stereo_image" into the main fbo through the
            // stencil pattern in the "stereo" VBO:
            //  1. draw the pattern writing 1 into the stencil, no color;
            //  2. draw the image where the stencil equals 1, all channels.
            // The write mask and stencil state are baked into the pipelines
            // (WebGPU has no dynamic equivalents); the stencil reference is
            // pass state.  Every pass starts with a cleared stencil.
            auto compositeEye = [&](const std::string& name,
                                    const bool clear,
                                    const math::Box2i& imageBox)
            {
                p.renderPass = clear ? p.fbo->beginClearRenderPass(p.cmd)
                                     : p.fbo->beginLoadRenderPass(p.cmd);
                p.currentPipeline.clear();
                wgpuRenderPassEncoderSetStencilReference(p.renderPass, 1);

                if (p.vbos["stereo"] && p.vbos["video"])
                {
                    // --- 1. Stencil mask: always pass, replace, no color.
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
                                       p.fbo, wipeShader, p.vbos["stereo"],
                                       ct, ds);
                    }

                    _createBindingSet(wipeShader);
                    PushConstants pc;
                    pc.mvp = p.transform;
                    pc.color = image::Color4f(1.F, 0.F, 0.F);
                    _bindDescriptorSets(p.renderPass, wipeShader, &pc);
                    _vkDraw("stereo");

                    // --- 2. The video where the stencil equals 1.
                    p.vbos["video"]->copy(convert(geom::box(imageBox, true),
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

                        // Default color target: all channels, no blending.
                        createPipeline(name + "_image", name + "_image",
                                       p.fbo, textureShader, p.vbos["video"],
                                       wgpu::ColorTargetStateInfo(), ds);
                    }

                    _createBindingSet(textureShader);
                    pc.mvp = p.transform;
                    pc.color = image::Color4f(1.F, 1.F, 1.F);
                    textureShader->setFBO("textureSampler",
                                          p.buffers["stereo_image"]);
                    _bindDescriptorSets(p.renderPass, textureShader, &pc);
                    _vkDraw("video");
                }

                p.fbo->endRenderPass(p.renderPass);
                p.renderPass = nullptr;
            };

            // ----- FIRST RENDER PASS: LEFT EYE (clears the main fbo)
            const bool hasStencil1 = createStencilMesh();
            if (hasStencil1)
            {
                compositeEye("stereo1", true, boxes[0]);
            }
            else
            {
                // Nothing to draw, but the main fbo is still cleared.
                p.renderPass = p.fbo->beginClearRenderPass(p.cmd);
                p.fbo->endRenderPass(p.renderPass);
                p.renderPass = nullptr;
            }

            // ----- Draw the right eye to the "stereo_image" buffer.  Between
            // passes, never inside one.
            const bool hasRightEye = videoData.size() > 1 && boxes.size() > 1;
            if (hasRightEye)
            {
                math::Matrix4x4f saved = getTransform();
                const math::Matrix4x4f mvp =
                    saved * math::translate(
                                math::Vector3f(eyeSeparation, 0.F, 0.F));
                setTransform(mvp);

                _drawVideo(
                    p.buffers["stereo_image"], "display",
                    videoData[1], boxes[1],
                    eyeImageOptions(1), eyeDisplayOptions(1));

                setTransform(saved);
            }

            // ----- SECOND RENDER PASS: RIGHT EYE (loads the main fbo).  The
            // pattern is shifted by one so it covers what the left one left
            // out.  With a single eye, "stereo_image" still holds the left
            // one.
            if (stereoType == StereoType::kScanlines)
            {
                box.min.y = 1;
            }
            else if (stereoType == StereoType::kColumns)
            {
                box.min.x = 1;
            }
            else if (stereoType == StereoType::kCheckers)
            {
                box.min.x = 1;
            }

            if (createStencilMesh())
            {
                compositeEye("stereo2", false,
                             hasRightEye ? boxes[1] : boxes[0]);
            }
        }

    } // namespace timeline_wgpu
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlTimelineWGPU/RenderPrivate.h>
#include <tlTimelineWGPU/RenderStructs.h>

#include <tlWGPU/Mesh.h>
#include <tlWGPU/WGPU.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace timeline_wgpu
    {

        void Render::drawAnaglyph(
            const std::vector<timeline::VideoFrame>& videoData,
            const std::vector<math::Box2i>& boxes,
            const float eyeSeparation,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (videoData.empty() || boxes.empty())
                return;

            // Color write masks are baked into the pipeline in WebGPU (there
            // is no dynamic equivalent), hence one pipeline per eye.
            const WGPUColorWriteMask redMask =
                WGPUColorWriteMask_Red | WGPUColorWriteMask_Alpha;
            const WGPUColorWriteMask cyanMask =
                WGPUColorWriteMask_Green | WGPUColorWriteMask_Blue |
                WGPUColorWriteMask_Alpha;

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
            // Create offscreen buffer to draw each eye into
            //
            const math::Size2i& offscreenBufferSize = p.fbo->getSize();
            wgpu::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            // How a picture is sampled lives in its ImageOptions now.
            offscreenBufferOptions.colorFilters =
                imageOptions.empty() ? timeline::ImageFilters()
                                     : imageOptions[0].imageFilters;
            offscreenBufferOptions.depth = wgpu::OffscreenDepth::kNone;
            offscreenBufferOptions.stencil = wgpu::OffscreenStencil::kNone;
            offscreenBufferOptions.clear = false;
            if (doCreate(
                    p.buffers["stereo_image"], offscreenBufferSize,
                    offscreenBufferOptions))
            {
                p.buffers["stereo_image"] = wgpu::OffscreenBuffer::create(
                    ctx, offscreenBufferSize, offscreenBufferOptions);
            }

            // Draws the "stereo_image" buffer into the main fbo, writing only
            // the channels in @p mask.
            //
            // This must be called with NO render pass open: WebGPU allows one
            // open pass per encoder, and _drawVideo() runs its own pass on
            // "stereo_image" (it is called between our passes, never inside
            // one).
            auto compositeEye = [&](const std::string& pipelineName,
                                    const math::Box2i& box,
                                    const WGPUColorWriteMask mask,
                                    const bool clear)
            {
                if (clear)
                    beginRenderPass();     // clears, sets p.renderPass
                else
                    beginLoadRenderPass(); // loads, sets p.renderPass
                p.currentPipeline.clear();

                if (p.vbos["video"])
                {
                    p.vbos["video"]->copy(
                        convert(geom::box(box, true), p.vbos["video"]->getType()));

                    wgpu::ColorTargetStateInfo ct;
                    ct.writeMask = mask;

                    // Default depth/stencil state == no depth test.
                    wgpu::DepthStencilStateInfo ds;

                    auto overlayShader = p.shaders["overlay"];
                    overlayShader->bind(p.frameIndex);
                    _createBindingSet(overlayShader);
                    overlayShader->setFBO(
                        "textureSampler", p.buffers["stereo_image"]);

                    createPipeline(
                        pipelineName, pipelineName, p.fbo, overlayShader,
                        p.vbos["video"], ct, ds);

                    // mvp and color are both in the push block now.
                    PushConstants pc;
                    pc.mvp = p.transform;
                    pc.color = image::Color4f(1.F, 1.F, 1.F);
                    _bindDescriptorSets(p.renderPass, overlayShader, &pc);

                    _vkDraw("video");
                }

                endRenderPass();
                p.renderPass = nullptr;
            };

            // ----- LEFT EYE: video -> "stereo_image", then red into main fbo
            _drawVideo(
                p.buffers["stereo_image"], "display", videoData[0], boxes[0],
                eyeImageOptions(0), eyeDisplayOptions(0));

            compositeEye("stereo_image1", boxes[0], redMask, true);

            // ----- RIGHT EYE: video -> "stereo_image", then cyan into main fbo
            const bool hasRightEye = videoData.size() > 1 && boxes.size() > 1;
            if (hasRightEye)
            {
                math::Matrix4x4f saved = getTransform();
                const math::Matrix4x4f mvp =
                    saved * math::translate(
                                math::Vector3f(eyeSeparation, 0.F, 0.F));
                setTransform(mvp);

                _drawVideo(
                    p.buffers["stereo_image"], "display", videoData[1],
                    boxes[1], eyeImageOptions(1), eyeDisplayOptions(1));

                setTransform(saved);
            }

            // With a single eye, "stereo_image" still holds the left eye, so
            // the cyan channels get the same image.
            compositeEye(
                "stereo_image2", hasRightEye ? boxes[1] : boxes[0], cyanMask,
                false);
        }
    } // namespace timeline_wgpu
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineVk/RenderPrivate.h>
#include <tlTimelineVk/RenderStructs.h>

#include <tlVk/Vk.h>
#include <tlVk/Mesh.h>
#include <tlVk/Util.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace
    {
        // Helper functions for transposing a matrix in-place.
        #define PL_TRANSPOSE_DIM(d, m)                                  \
            pl_transpose((d), (float[(d)*(d)]){0}, (const float *)(m))

#define PL_TRANSPOSE_2X2(m) PL_TRANSPOSE_DIM(2, m)
#define PL_TRANSPOSE_3X3(m) PL_TRANSPOSE_DIM(3, m)
#define PL_TRANSPOSE_4X4(m) PL_TRANSPOSE_DIM(4, m)

        inline float *pl_transpose(int dim, float *out, const float *in)
        {
            for (int i = 0; i < dim; i++) {
                for (int j = 0; j < dim; j++)
                    out[i * dim + j] = in[j * dim + i];
            }

            return out;
        }
    }

    
    namespace timeline_vlk
    {
        
        void Render::drawVideo(
            const std::vector<timeline::VideoData>& videoData, const std::vector<math::Box2i>& boxes, const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions, const timeline::CompareOptions& compareOptions,
            const timeline::BackgroundOptions& backgroundOptions)
        {
            TLRENDER_P();

            //! \todo Render the background only if there is valid video data
            //! and a valid layer?
            if (!videoData.empty() && !videoData.front().layers.empty())
            {
                _drawBackground(boxes, backgroundOptions);
            }
            switch (compareOptions.mode)
            {
            case timeline::CompareMode::A:
                _drawVideoA(videoData, boxes, imageOptions, displayOptions, compareOptions);
                break;
            case timeline::CompareMode::B:
                _drawVideoB(videoData, boxes, imageOptions, displayOptions, compareOptions);
                break;
            case timeline::CompareMode::Wipe:
                _drawVideoWipe(videoData, boxes, imageOptions, displayOptions, compareOptions);
                break;
            case timeline::CompareMode::Overlay:
                _drawVideoOverlay(videoData, boxes, imageOptions, displayOptions, compareOptions);
                break;
            case timeline::CompareMode::Difference:
                if (videoData.size() > 1)
                {
                    _drawVideoDifference(videoData, boxes, imageOptions, displayOptions, compareOptions);
                }
                else
                {
                    _drawVideoA(videoData, boxes, imageOptions, displayOptions, compareOptions);
                }
                break;
            case timeline::CompareMode::Horizontal:
            case timeline::CompareMode::Vertical:
            case timeline::CompareMode::Tile:
                _drawVideoTile(videoData, boxes, imageOptions, displayOptions, compareOptions);
                break;
            default:
                break;
            }
        }

        void Render::_drawBackground(const std::vector<math::Box2i>& boxes, const timeline::BackgroundOptions& options)
        {
            TLRENDER_P();

            for (const auto& box : boxes)
            {
                p.fbo->transitionToColorAttachment(p.cmd);
                switch (options.type)
                {
                case timeline::Background::Solid:
                {
                    const auto& mesh = geom::box(box);
                    p.fbo->beginRenderPass(p.cmd, "Solid BG");
                    p.fbo->setupViewportAndScissor(p.cmd);
                    drawRect("solid", "rect", "rect", box, options.color0);
                    p.fbo->endRenderPass(p.cmd);
                    break;
                }
                case timeline::Background::Checkers:
                {
                    const auto& mesh = geom::checkers(box, options.color0,
                                                      options.color1,
                                                      options.checkersSize);
                    _createMesh("colorMesh", mesh);
                    _createPipeline(p.fbo, "checkers", "checkers", "colorMesh", "colorMesh");
                    p.fbo->beginRenderPass(p.cmd, "Checkers BG");
                    drawColorMesh("checkers", mesh, math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));
                    p.fbo->endRenderPass(p.cmd);
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
                    _createMesh("colorMesh", mesh);
                    _createPipeline(p.fbo, "gradient", "gradient", "colorMesh", "colorMesh");
                    p.fbo->beginRenderPass(p.cmd, "Gradient BG");
                    drawColorMesh("gradient", mesh, math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));
                    p.fbo->endRenderPass(p.cmd);
                    break;
                }
                default:
                    break;
                }
                p.fbo->transitionToShaderRead(p.cmd);
            }
        }

        void Render::_drawVideoA(
            const std::vector<timeline::VideoData>& videoData, const std::vector<math::Box2i>& boxes, const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions, const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (!videoData.empty() && !boxes.empty())
            {
                _drawVideo(
                    videoData[0], boxes[0], !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
            }
        }

        void Render::_drawVideoB(
            const std::vector<timeline::VideoData>& videoData, const std::vector<math::Box2i>& boxes, const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions, const timeline::CompareOptions& compareOptions)
        {
            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1], boxes[1], imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
            }
        }

        void Render::_drawVideoWipe(
            const std::vector<timeline::VideoData>& videoData, const std::vector<math::Box2i>& boxes, const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions, const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

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

            // gl::SetAndRestore stencilTest(GL_STENCIL_TEST, GL_TRUE);

            // glViewport(
            //     p.viewport.x(),
            //     p.renderSize.h - p.viewport.h() - p.viewport.y(),
            //     p.viewport.w(), p.viewport.h());
            // glClear(GL_STENCIL_BUFFER_BIT);
            // glStencilFunc(GL_ALWAYS, 1, 0xFF);
            // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            p.shaders["wipe"]->bind(p.frameIndex);
            p.shaders["wipe"]->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);
            p.shaders["wipe"]->setUniform("color", image::Color4f(1.F, 0.F, 0.F));
            {
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
                if (p.vaos["wipe"])
                {
                    p.vaos["wipe"]->bind(p.frameIndex);
                }
            }
            // glStencilFunc(GL_EQUAL, 1, 0xFF);
            // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            if (!videoData.empty() && !boxes.empty())
            {
                _drawVideo(
                    videoData[0], boxes[0], !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
            }

            // glViewport(
            //     p.viewport.x(),
            //     p.renderSize.h - p.viewport.h() - p.viewport.y(),
            //     p.viewport.w(), p.viewport.h());
            // glClear(GL_STENCIL_BUFFER_BIT);
            // glStencilFunc(GL_ALWAYS, 1, 0xFF);
            // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            p.shaders["wipe"]->bind(p.frameIndex);
            p.shaders["wipe"]->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);
            p.shaders["wipe"]->setUniform("color", image::Color4f(0.F, 1.F, 0.F));
            {
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
                if (p.vaos["wipe"])
                {
                    p.vaos["wipe"]->bind(p.frameIndex);
                    p.vaos["wipe"]->draw(p.cmd, p.vbos["wipe"]);
                    p.garbage[p.frameIndex].vaos.push_back(p.vaos["wipe"]);
                }
            }
            // glStencilFunc(GL_EQUAL, 1, 0xFF);
            // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1], boxes[1], imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
            }
        }

        void Render::_drawVideoOverlay(
            const std::vector<timeline::VideoData>& videoData, const std::vector<math::Box2i>& boxes, const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions, const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1], boxes[1], imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
            }
            if (!videoData.empty() && !boxes.empty())
            {
                const math::Size2i offscreenBufferSize(boxes[0].w(), boxes[0].h());
                vlk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                }
                if (doCreate(p.buffers["overlay"], offscreenBufferSize, offscreenBufferOptions))
                {
                    p.buffers["overlay"] = vlk::OffscreenBuffer::create(ctx, offscreenBufferSize, offscreenBufferOptions);
                }

                if (p.buffers["overlay"])
                {
                    // const gl::SetAndRestore scissorTest(
                    //     GL_SCISSOR_TEST, GL_FALSE);

                    // gl::OffscreenBufferBinding binding(p.buffers["overlay"]);
                    // glViewport(
                    //     0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                    // glClearColor(0.F, 0.F, 0.F, 0.F);
                    // glClear(GL_COLOR_BUFFER_BIT);

                    p.shaders["display"]->bind(p.frameIndex);
                    p.shaders["display"]->setUniform(
                        "transform.mvp",
                        math::ortho(0.F, static_cast<float>(offscreenBufferSize.w), static_cast<float>(offscreenBufferSize.h), 0.F, -1.F, 1.F));

                    _drawVideo(
                        videoData[0], math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());

                    p.shaders["display"]->bind(p.frameIndex);
                    p.shaders["display"]->setUniform("transform.mvp", p.transform);
                }

                if (p.buffers["overlay"])
                {
                    // glBlendFuncSeparate(
                    //     GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                    //     GL_ONE);

                    // glViewport(
                    //     p.viewport.x(),
                    //     p.renderSize.h - p.viewport.h() - p.viewport.y(),
                    //     p.viewport.w(), p.viewport.h());

                    p.shaders["overlay"]->bind(p.frameIndex);
                    p.shaders["overlay"]->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);

                    image::Color4f color = image::Color4f(1.F, 1.F, 1.F, compareOptions.overlay);

                    const std::string pipelineName = "overlay";
                    const std::string shaderName = "overlay";
                    const std::string meshName = "video";
                    const std::string pipelineLayoutName = shaderName;
                    _createPipeline(p.buffers["overlay"], pipelineName,
                                    pipelineLayoutName,
                                    shaderName, meshName);
                    
                    VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];

                    _bindDescriptorSets(pipelineLayoutName, shaderName);

                    vkCmdPushConstants(p.cmd, pipelineLayout,
                                       p.shaders["overlay"]->getPushStageFlags(), 0, sizeof(color), &color);

                    p.shaders["overlay"]->setFBO("textureSampler", p.buffers["overlay"]);

                    // glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                    // glBindTexture(
                    //     GL_TEXTURE_2D, p.buffers["overlay"]->getColorID());

                    if (p.vbos["video"])
                    {
                        p.vbos["video"]->copy(convert(geom::box(boxes[0], true), p.vbos["video"]->getType()));
                    }
                    if (p.vaos["video"])
                    {
                        p.vaos["video"]->bind(p.frameIndex);
                        p.vaos["video"]->draw(p.cmd, p.vbos["video"]);
                        p.garbage[p.frameIndex].vaos.push_back(p.vaos["video"]);
                    }
                }
            }
        }

        void Render::_drawVideoDifference(
            const std::vector<timeline::VideoData>& videoData, const std::vector<math::Box2i>& boxes, const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions, const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();
            if (!videoData.empty() && !boxes.empty())
            {
                const math::Size2i offscreenBufferSize(boxes[0].w(), boxes[0].h());
                vlk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                }
                if (doCreate(p.buffers["difference0"], offscreenBufferSize, offscreenBufferOptions))
                {
                    p.buffers["difference0"] = vlk::OffscreenBuffer::create(ctx, offscreenBufferSize, offscreenBufferOptions);
                }

                if (p.buffers["difference0"])
                {
                    // const gl::SetAndRestore scissorTest(
                    //     GL_SCISSOR_TEST, GL_FALSE);

                    // gl::OffscreenBufferBinding binding(
                    //     p.buffers["difference0"]);
                    // glViewport(
                    //     0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                    // glClearColor(0.F, 0.F, 0.F, 0.F);
                    // glClear(GL_COLOR_BUFFER_BIT);

                    p.shaders["display"]->bind(p.frameIndex);
                    p.shaders["display"]->setUniform(
                        "transform.mvp",
                        math::ortho(0.F, static_cast<float>(offscreenBufferSize.w), static_cast<float>(offscreenBufferSize.h), 0.F, -1.F, 1.F));

                    _drawVideo(
                        videoData[0], math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());

                    p.shaders["display"]->bind(p.frameIndex);
                    p.shaders["display"]->setUniform("transform.mvp", p.transform);
                }

                if (videoData.size() > 1)
                {
                    offscreenBufferOptions = vlk::OffscreenBufferOptions();
                    offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
                    if (displayOptions.size() > 1)
                    {
                        offscreenBufferOptions.colorFilters = displayOptions[1].imageFilters;
                    }
                    if (doCreate(p.buffers["difference1"], offscreenBufferSize, offscreenBufferOptions))
                    {
                        p.buffers["difference1"] = vlk::OffscreenBuffer::create(ctx, offscreenBufferSize, offscreenBufferOptions);
                    }

                    if (p.buffers["difference1"])
                    {
                        // const gl::SetAndRestore scissorTest(
                        //     GL_SCISSOR_TEST, GL_FALSE);

                        // gl::OffscreenBufferBinding binding(
                        //     p.buffers["difference1"]);
                        // glViewport(
                        //     0, 0, offscreenBufferSize.w,
                        //     offscreenBufferSize.h);
                        // glClearColor(0.F, 0.F, 0.F, 0.F);
                        // glClear(GL_COLOR_BUFFER_BIT);

                        p.shaders["display"]->bind(p.frameIndex);
                        p.shaders["display"]->setUniform(
                            "transform.mvp",
                            math::ortho(0.F, static_cast<float>(offscreenBufferSize.w), static_cast<float>(offscreenBufferSize.h), 0.F, -1.F, 1.F));

                        _drawVideo(
                            videoData[1], math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                            imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                            displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
                    }
                }
                else
                {
                    p.buffers["difference1"].reset();
                }

                if (p.buffers["difference0"] && p.buffers["difference1"])
                {
                    // glBlendFuncSeparate(
                    //     GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                    // glViewport(
                    //     p.viewport.x(),
                    //     p.renderSize.h - p.viewport.h() - p.viewport.y(),
                    //     p.viewport.w(), p.viewport.h());

                    p.shaders["difference"]->bind(p.frameIndex);
                    p.shaders["difference"]->setUniform("transform.mvp", p.transform, vlk::kShaderVertex);
                    p.shaders["difference"]->setFBO("textureSampler", p.buffers["difference0"]);
                    p.shaders["difference"]->setFBO("textureSamplerB", p.buffers["difference1"]);

                    if (p.vbos["video"])
                    {
                        p.vbos["video"]->copy(convert(geom::box(boxes[0], true), p.vbos["video"]->getType()));
                    }
                    if (p.vaos["video"])
                    {
                        p.vaos["video"]->bind(p.frameIndex);
                        p.vaos["video"]->draw(p.cmd, p.vbos["video"]);
                        p.garbage[p.frameIndex].vaos.push_back(p.vaos["video"]);
                    }
                }
            }
        }

        void Render::_drawVideoTile(
            const std::vector<timeline::VideoData>& videoData, const std::vector<math::Box2i>& boxes, const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions, const timeline::CompareOptions& compareOptions)
        {
            for (size_t i = 0; i < videoData.size() && i < boxes.size(); ++i)
            {
                _drawVideo(
                    videoData[i], boxes[i], i < imageOptions.size() ? std::make_shared<timeline::ImageOptions>(imageOptions[i]) : nullptr,
                    i < displayOptions.size() ? displayOptions[i] : timeline::DisplayOptions());
            }
        }

        namespace
        {
            float knee(float x, float f)
            {
                return logf(x * f + 1.F) / f;
            }

            float knee2(float x, float y)
            {
                float f0 = 0.F;
                float f1 = 1.F;
                while (knee(x, f1) > y)
                {
                    f0 = f1;
                    f1 = f1 * 2.F;
                }
                for (size_t i = 0; i < 30; ++i)
                {
                    const float f2 = (f0 + f1) / 2.F;
                    if (knee(x, f2) < y)
                    {
                        f1 = f2;
                    }
                    else
                    {
                        f0 = f2;
                    }
                }
                return (f0 + f1) / 2.F;
            }
        } // namespace
        
        void Render::_drawVideo(
            const timeline::VideoData& videoData, const math::Box2i& box, const std::shared_ptr<timeline::ImageOptions>& imageOptions,
            const timeline::DisplayOptions& displayOptions)
        {
            TLRENDER_P();

            // \@todo: \@bug: there's no such call in Vulkan.
            // GLint viewportPrev[4] = {0, 0, 0, 0};
            // glGetIntegerv(GL_VIEWPORT, viewportPrev);

            const auto transform = math::ortho(0.F, static_cast<float>(box.w()), static_cast<float>(box.h()), 0.F, -1.F, 1.F);
            p.shaders["image"]->bind(p.frameIndex);
            p.shaders["image"]->setUniform("transform.mvp", transform, vlk::kShaderVertex);

            const math::Size2i& offscreenBufferSize = box.getSize();
            vlk::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            offscreenBufferOptions.colorFilters = displayOptions.imageFilters;
            offscreenBufferOptions.clear = true;
            if (doCreate(p.buffers["video"], offscreenBufferSize, offscreenBufferOptions))
            {
                p.buffers["video"] = vlk::OffscreenBuffer::create(ctx, offscreenBufferSize,
                                                                  offscreenBufferOptions);
            }

            float d1 = 1;
            float d2 = 1;
            bool inDissolve = false;
            if (p.buffers["video"])
            {
                for (const auto& layer : videoData.layers)
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
                                p.buffers["dissolve"] = vlk::OffscreenBuffer::create(ctx, offscreenBufferSize,
                                                                                     offscreenBufferOptions);
                            }
                            if (doCreate(p.buffers["dissolve2"], offscreenBufferSize, offscreenBufferOptions))
                            {
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
                                    image::getBox(layer.image->getAspect(),
                                                  math::Box2i(0, 0, offscreenBufferSize.w,
                                                              offscreenBufferSize.h)),
                                    image::Color4f(1.F, 1.F, 1.F, v), dissolveImageOptions);
                            }
                            if (p.buffers["dissolve2"])
                            {
                                float v = d2 = layer.transitionValue;
                                auto dissolveImageOptions = imageOptions.get() ? *imageOptions : layer.imageOptionsB;
                                dissolveImageOptions.alphaBlend = timeline::AlphaBlend::Straight;
                                drawImage(
                                    p.buffers["dissolve2"], layer.imageB,
                                    image::getBox(layer.imageB->getAspect(),
                                                  math::Box2i(0, 0, offscreenBufferSize.w,
                                                              offscreenBufferSize.h)),
                                    image::Color4f(1.F, 1.F, 1.F, v), dissolveImageOptions);
                            }
                            

#define FIRST_RENDER_PASS 1  // working fine by itself
#define SECOND_RENDER_PASS 1 // working fine by itself
                            if (p.buffers["dissolve"] && p.buffers["dissolve2"])
                            {
                                inDissolve = true;
                                
                                p.buffers["dissolve"]->transitionToShaderRead(p.cmd);
                                p.buffers["dissolve2"]->transitionToShaderRead(p.cmd);

                                p.buffers["video"]->transitionToColorAttachment(p.cmd);

                                // --- Common Setup ---
                                std::shared_ptr<vlk::ShaderBindingSet> bindingSet;
                                const auto transform = math::ortho(0.F, static_cast<float>(box.w()), 0.F,
                                                                   static_cast<float>(box.h()), -1.F, 1.F);
                                const image::Color4f color(1.F, 1.F, 1.F);
                                const std::string pipelineNameBase = "video_dissolve"; // Use distinct names
                                const std::string shaderName = "dissolve";
                                const std::string meshName = "video";
                                std::string pipelineLayoutName = shaderName; // Typically shader name determines layout
                                VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
                                // Ensure pipelineLayout is valid, create if necessary
                                if (!pipelineLayout) {
                                    pipelineLayout = _createPipelineLayout(pipelineLayoutName,
                                                                           p.shaders[shaderName]);
                                }

                                std::string pipelineName = pipelineNameBase + "_Pass1_NoBlend";

                                // Create or find a pipeline without blending
                                bool enableBlending = false;
                                _createPipeline(p.buffers["video"],
                                                pipelineName,
                                               "dissolve",
                                               shaderName, meshName,
                                               enableBlending);
                                p.buffers["video"]->beginRenderPass(p.cmd, "COMP VIDEO RENDER");

                                auto shader = p.shaders["dissolve"];
                                _createBindingSet(shader);

                                p.shaders["dissolve"]->bind(p.frameIndex);
                                p.shaders["dissolve"]->setUniform("transform.mvp", transform, vlk::kShaderVertex);
                                p.shaders["dissolve"]->setFBO("textureSampler", p.buffers["dissolve"]);

                                _bindDescriptorSets(pipelineLayoutName,
                                                    shaderName);

                                vkCmdPushConstants(p.cmd, pipelineLayout,
                                                   p.shaders["dissolve"]->getPushStageFlags(), 0, sizeof(color), &color);
                                

#if FIRST_RENDER_PASS
                                if (p.vbos["video"])
                                {
                                    p.vbos["video"]->copy(
                                        convert(geom::box(math::Box2i(0, 0,
                                                                      offscreenBufferSize.w,
                                                                      offscreenBufferSize.h),
                                                          true), p.vbos["video"]->getType()));
                                }
                                if (p.vaos["video"])
                                {
                                    p.vaos["video"]->bind(p.frameIndex);
                                    p.vaos["video"]->draw(p.cmd, p.vbos["video"]);
                                    p.garbage[p.frameIndex].vaos.push_back(p.vaos["video"]);
                                }
#endif

                                _createBindingSet(shader);

                                pipelineName = pipelineNameBase + "_Pass2_BlendColorForceAlpha";
                                enableBlending = true;
                                _createPipeline(
                                    p.buffers["video"], pipelineName,
                                    pipelineLayoutName, shaderName, meshName,
                                    enableBlending,
                                    VK_BLEND_FACTOR_SRC_ALPHA,
                                    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                    VK_BLEND_FACTOR_ONE,
                                    VK_BLEND_FACTOR_ONE,
                                    VK_BLEND_OP_ADD,
                                    VK_BLEND_OP_ADD);

                                p.shaders["dissolve"]->bind(p.frameIndex);
                                p.shaders["dissolve"]->setUniform("transform.mvp", transform,
                                                                  vlk::kShaderVertex);
                                p.shaders["dissolve"]->setFBO("textureSampler", p.buffers["dissolve2"]);
                                vkCmdPushConstants(p.cmd, pipelineLayout,
                                                   p.shaders["dissolve"]->getPushStageFlags(), 0,
                                                   sizeof(color), &color);
                                _bindDescriptorSets(pipelineLayoutName,
                                                    shaderName);

#if SECOND_RENDER_PASS
                                if (p.vbos["video"])
                                {
                                    p.vbos["video"]->copy(
                                        convert(geom::box(math::Box2i(0, 0,
                                                                      offscreenBufferSize.w,
                                                                      offscreenBufferSize.h), true),
                                                p.vbos["video"]->getType()));
                                }
                                if (p.vaos["video"])
                                {
                                    p.vaos["video"]->bind(p.frameIndex);
                                    p.vaos["video"]->draw(p.cmd, p.vbos["video"]);
                                    p.garbage[p.frameIndex].vaos.push_back(p.vaos["video"]);
                                }
#endif

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
                                image::getBox(layer.image->getAspect(), math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                image::Color4f(1.F, 1.F, 1.F, 1.F - layer.transitionValue), imageOptions.get() ? *imageOptions : layer.imageOptions);
                        }
                        else if (layer.imageB)
                        {
                            drawImage(
                                p.buffers["video"], layer.imageB,
                                image::getBox(layer.imageB->getAspect(), math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                image::Color4f(1.F, 1.F, 1.F, layer.transitionValue), imageOptions.get() ? *imageOptions : layer.imageOptionsB);
                        }
                        break;
                    }
                    default:
                        if (layer.image)
                        {
                            drawImage(
                                p.buffers["video"], layer.image,
                                image::getBox(layer.image->getAspect(), math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                image::Color4f(1.F, 1.F, 1.F), imageOptions.get() ? *imageOptions : layer.imageOptions);
                        }
                        break;
                    }
                }
            }

            if (p.buffers["video"])
            {
                // Begin the new compositing render pass.
                p.fbo->transitionToColorAttachment(p.cmd);
                
                p.buffers["video"]->transitionToShaderRead(p.cmd);
                
                const std::string pipelineName = "display";
                const std::string pipelineLayoutName = "display";
                const std::string shaderName = "display";
                const std::string meshName = "video";
                const bool enableBlending = !inDissolve;  
                _createPipeline(p.fbo, pipelineName,
                                pipelineLayoutName, shaderName, meshName,
                                enableBlending,
                                VK_BLEND_FACTOR_SRC_ALPHA,
                                VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                VK_BLEND_FACTOR_ONE,
                                VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);


                p.fbo->beginRenderPass(p.cmd, "DISPLAY PASS");

                p.shaders["display"]->bind(p.frameIndex);
                p.shaders["display"]->setFBO("textureSampler", p.buffers["video"]);

                UBOLevels uboLevels;
                uboLevels.enabled = displayOptions.levels.enabled;
                uboLevels.inLow = displayOptions.levels.inLow;
                uboLevels.inHigh = displayOptions.levels.inHigh;
                uboLevels.gamma = displayOptions.levels.gamma;
                uboLevels.outLow = displayOptions.levels.outLow;
                uboLevels.outHigh = displayOptions.levels.outHigh;
                uboLevels.gamma = uboLevels.gamma > 0.F ? (1.F / uboLevels.gamma) : 1000000.F;
                p.shaders["display"]->setUniform("uboLevels", uboLevels);

                UBONormalize uboNormalize;
                uboNormalize.enabled = displayOptions.normalize.enabled;
                uboNormalize.minimum = displayOptions.normalize.minimum;
                uboNormalize.maximum = displayOptions.normalize.maximum;

                p.shaders["display"]->setUniform("uboNormalize", uboNormalize);

                UBOColor uboColor;
                const bool colorMatrixEnabled = displayOptions.color != timeline::Color() && displayOptions.color.enabled;
                uboColor.enabled = colorMatrixEnabled;
                uboColor.add = displayOptions.color.add;
                uboColor.matrix = color(displayOptions.color);
                uboColor.invert = displayOptions.color.invert;

                p.shaders["display"]->setUniform("uboColor", uboColor);
                p.shaders["display"]->setUniform("uboEXRDisplay", displayOptions.exrDisplay);

                UBOOptions ubo;
                ubo.channels = static_cast<int>(displayOptions.channels);
                ubo.mirrorX = displayOptions.mirror.x;
                ubo.mirrorY = displayOptions.mirror.y;
                ubo.softClip = displayOptions.softClip.enabled ? displayOptions.softClip.value : 0.F;
                ubo.videoLevels = static_cast<int>(displayOptions.videoLevels);
                ubo.invalidValues = displayOptions.invalidValues;
                p.shaders["display"]->setUniform("ubo", ubo);

#if defined(TLRENDER_OCIO)
                if (p.ocioData)
                {
                    for (auto& texture : p.ocioData->textures)
                    {
                        p.shaders["display"]->setTexture(texture->getName(), texture);
                    }
                }
                if (p.lutData)
                {
                    for (auto& texture : p.lutData->textures)
                    {
                        p.shaders["display"]->setTexture(texture->getName(), texture);
                    }
                }
#endif // TLRENDER_OCIO
#if defined(TLRENDER_LIBPLACEBO)
                if (p.placeboData)
                {
                    for (auto& texture : p.placeboData->textures)
                    {
                        p.shaders["display"]->setTexture(texture->getName(), texture);
                    }

                    std::size_t pushSize = p.shaders["display"]->getPushSize();
                    if (pushSize > 0)
                    {
                        std::vector<uint8_t> pushData(pushSize, 0);
                        const pl_shader_res* res = p.placeboData->res;
                        VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
                        std::size_t currentOffset = 0;
                        for (int i = 0; i < res->num_variables; ++i)
                        {
                            const struct pl_shader_var& shader_var = res->variables[i];
                            const struct pl_var& var = shader_var.var;
                            const struct pl_var_layout& layout = pl_std430_layout(currentOffset, &var);
                            
                            // Handle matrix types (dim_m > 1 && dim_v > 1)
                            if (var.dim_m > 1 && var.dim_v > 1)
                            {
                                // For column-major matrices, we pad each column according to layout.stride
                                const float* src = reinterpret_cast<const float*>(shader_var.data);
                                uint8_t* dst = pushData.data() + layout.offset;

                                // Fill each column (dim_m = #columns, dim_v = #rows)
                                for (int col = 0; col < var.dim_m; ++col)
                                {
                                    const float* src_col = src + col * var.dim_v;
                                    float* dst_col = reinterpret_cast<float*>(dst + layout.stride * col);
                                    memcpy(dst_col, src_col, sizeof(float) * var.dim_v);
                                }
                            }
                            else
                            {
                                // Scalars, vectors, or arrays thereof — copy the block directly
                                memcpy(pushData.data() + layout.offset, shader_var.data, layout.size);
                            }

                            currentOffset = layout.offset + layout.size;
                        }
                        
                        vkCmdPushConstants(p.cmd, pipelineLayout,
                                           VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                           pushData.size(), pushData.data());
                    }
                }
#endif // TLRENDER_LIBPLACEBO

                _bindDescriptorSets(pipelineLayoutName, shaderName);

                if (p.vbos["video"])
                {
                    p.vbos["video"]->copy(convert(geom::box(box, true), p.vbos["video"]->getType()));
                }
                if (p.vaos["video"])
                {
                    p.vaos["video"]->bind(p.frameIndex);
                    p.vaos["video"]->draw(p.cmd, p.vbos["video"]);
                    p.garbage[p.frameIndex].vaos.push_back(p.vaos["video"]);
                }

                p.fbo->endRenderPass(p.cmd);

                // Transition buffer back to color attachment
                p.buffers["video"]->transitionToColorAttachment(p.cmd);
                
            }
        }
    } // namespace timeline_vlk
} // namespace tl

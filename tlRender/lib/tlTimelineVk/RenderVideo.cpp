// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineVk/RenderPrivate.h>

#include <tlVk/Vk.h>
#include <tlVk/Mesh.h>
#include <tlVk/Util.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace timeline_vlk
    {
        void Render::drawVideo(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            const timeline::BackgroundOptions& backgroundOptions)
        {
            //! \todo Render the background only if there is valid video data
            //! and a valid layer?
            if (!videoData.empty() && !videoData.front().layers.empty())
            {
                _drawBackground(boxes, backgroundOptions);
            }
            switch (compareOptions.mode)
            {
            case timeline::CompareMode::A:
                _drawVideoA(
                    videoData, boxes, imageOptions, displayOptions,
                    compareOptions);
                break;
            case timeline::CompareMode::B:
                _drawVideoB(
                    videoData, boxes, imageOptions, displayOptions,
                    compareOptions);
                break;
            case timeline::CompareMode::Wipe:
                _drawVideoWipe(
                    videoData, boxes, imageOptions, displayOptions,
                    compareOptions);
                break;
            case timeline::CompareMode::Overlay:
                _drawVideoOverlay(
                    videoData, boxes, imageOptions, displayOptions,
                    compareOptions);
                break;
            case timeline::CompareMode::Difference:
                if (videoData.size() > 1)
                {
                    _drawVideoDifference(
                        videoData, boxes, imageOptions, displayOptions,
                        compareOptions);
                }
                else
                {
                    _drawVideoA(
                        videoData, boxes, imageOptions, displayOptions,
                        compareOptions);
                }
                break;
            case timeline::CompareMode::Horizontal:
            case timeline::CompareMode::Vertical:
            case timeline::CompareMode::Tile:
                _drawVideoTile(
                    videoData, boxes, imageOptions, displayOptions,
                    compareOptions);
                break;
            default:
                break;
            }
        }

        void Render::_drawBackground(
            const std::vector<math::Box2i>& boxes,
            const timeline::BackgroundOptions& options)
        {
            TLRENDER_P();
            
            for (const auto& box : boxes)
            {
                switch (options.type)
                {
                case timeline::Background::Solid:
                {
                    const auto& mesh = geom::box(box);
                    _createMesh("rect", mesh);
                    _createPipeline(p.fbo, "solid", "rect", "rect");
                    p.shaders["rect"]->bind(p.frameIndex);
                    _bindDescriptorSets("solid", "rect");
                    p.fbo->beginRenderPass(p.cmd);
                    drawRect(box, options.color0);
                    p.fbo->endRenderPass(p.cmd);
                    break;
                }
                case timeline::Background::Checkers:
                {
                    const auto& mesh = geom::checkers(
                        box, options.color0, options.color1,
                        options.checkersSize);
                    _createMesh("colorMesh", mesh);
                    _createPipeline(p.fbo, "checkers", "colorMesh", "colorMesh");
                    p.shaders["colorMesh"]->bind(p.frameIndex);
                    _bindDescriptorSets("checkers", "colorMesh");
                    p.fbo->beginRenderPass(p.cmd);
                    drawColorMesh(mesh,
                        math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));
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
                    mesh.c.push_back(
                        math::Vector4f(
                            options.color0.r, options.color0.g,
                            options.color0.b, options.color0.a));
                    mesh.c.push_back(
                        math::Vector4f(
                            options.color1.r, options.color1.g,
                            options.color1.b, options.color1.a));
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
                    _createPipeline(p.fbo, "gradient", "colorMesh", "colorMesh");
                    p.shaders["colorMesh"]->bind(p.frameIndex);
                    _bindDescriptorSets("gradient", "colorMesh");
                    p.fbo->beginRenderPass(p.cmd);
                    drawColorMesh(
                        mesh, math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));
                    p.fbo->endRenderPass(p.cmd);
                    break;
                }
                default:
                    break;
                }
            }
        }

        void Render::_drawVideoA(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (!videoData.empty() && !boxes.empty())
            {
                _drawVideo(
                    videoData[0], boxes[0],
                    !imageOptions.empty()
                        ? std::make_shared<timeline::ImageOptions>(
                              imageOptions[0])
                        : nullptr,
                    !displayOptions.empty() ? displayOptions[0]
                                            : timeline::DisplayOptions());
            }
        }

        void Render::_drawVideoB(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1], boxes[1],
                    imageOptions.size() > 1
                        ? std::make_shared<timeline::ImageOptions>(
                              imageOptions[1])
                        : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1]
                                              : timeline::DisplayOptions());
            }
        }

        void Render::_drawVideoWipe(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
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
            p.shaders["wipe"]->setUniform(
                "color", image::Color4f(1.F, 0.F, 0.F));
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
                    p.vbos["wipe"]->copy(
                        convert(mesh, p.vbos["wipe"]->getType()));
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
                    videoData[0], boxes[0],
                    !imageOptions.empty()
                        ? std::make_shared<timeline::ImageOptions>(
                              imageOptions[0])
                        : nullptr,
                    !displayOptions.empty() ? displayOptions[0]
                                            : timeline::DisplayOptions());
            }

            // glViewport(
            //     p.viewport.x(),
            //     p.renderSize.h - p.viewport.h() - p.viewport.y(),
            //     p.viewport.w(), p.viewport.h());
            // glClear(GL_STENCIL_BUFFER_BIT);
            // glStencilFunc(GL_ALWAYS, 1, 0xFF);
            // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            p.shaders["wipe"]->bind(p.frameIndex);
            p.shaders["wipe"]->setUniform(
                "color", image::Color4f(0.F, 1.F, 0.F));
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
                    p.vbos["wipe"]->copy(
                        convert(mesh, p.vbos["wipe"]->getType()));
                }
                if (p.vaos["wipe"])
                {
                    p.vaos["wipe"]->bind(p.frameIndex);
                    p.vaos["wipe"]->upload(p.vbos["wipe"]->getData());
                    p.vaos["wipe"]->draw(p.cmd, p.vbos["wipe"]);
                }
            }
            // glStencilFunc(GL_EQUAL, 1, 0xFF);
            // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1], boxes[1],
                    imageOptions.size() > 1
                        ? std::make_shared<timeline::ImageOptions>(
                              imageOptions[1])
                        : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1]
                                              : timeline::DisplayOptions());
            }
        }

        void Render::_drawVideoOverlay(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1], boxes[1],
                    imageOptions.size() > 1
                        ? std::make_shared<timeline::ImageOptions>(
                              imageOptions[1])
                        : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1]
                                              : timeline::DisplayOptions());
            }
            if (!videoData.empty() && !boxes.empty())
            {
                const math::Size2i offscreenBufferSize(
                    boxes[0].w(), boxes[0].h());
                vlk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        displayOptions[0].imageFilters;
                }
                if (doCreate(
                        p.buffers["overlay"], offscreenBufferSize,
                        offscreenBufferOptions))
                {
                    p.buffers["overlay"] = vlk::OffscreenBuffer::create(
                        ctx, offscreenBufferSize, offscreenBufferOptions);
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
                        math::ortho(
                            0.F, static_cast<float>(offscreenBufferSize.w),
                            static_cast<float>(offscreenBufferSize.h), 0.F,
                            -1.F, 1.F));

                    _drawVideo(
                        videoData[0],
                        math::Box2i(
                            0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty()
                            ? std::make_shared<timeline::ImageOptions>(
                                  imageOptions[0])
                            : nullptr,
                        !displayOptions.empty() ? displayOptions[0]
                                                : timeline::DisplayOptions());

                    p.shaders["display"]->bind(p.frameIndex);
                    p.shaders["display"]->setUniform(
                        "transform.mvp", p.transform);
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
                    p.shaders["overlay"]->setUniform(
                        "color",
                        image::Color4f(1.F, 1.F, 1.F, compareOptions.overlay));
                    p.shaders["overlay"]->setUniform("textureSampler", 0);

                    // glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                    // glBindTexture(
                    //     GL_TEXTURE_2D, p.buffers["overlay"]->getColorID());

                    if (p.vbos["video"])
                    {
                        p.vbos["video"]->copy(convert(
                            geom::box(boxes[0], true),
                            p.vbos["video"]->getType()));
                    }
                    if (p.vaos["video"])
                    {
                        p.vaos["video"]->bind(p.frameIndex);
                        p.vaos["video"]->upload(p.vbos["video"]->getData());
                        p.vaos["video"]->draw(p.cmd, p.vbos["video"]);
                    }
                }
            }
        }

        void Render::_drawVideoDifference(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();
            if (!videoData.empty() && !boxes.empty())
            {
                const math::Size2i offscreenBufferSize(
                    boxes[0].w(), boxes[0].h());
                vlk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        displayOptions[0].imageFilters;
                }
                if (doCreate(
                        p.buffers["difference0"], offscreenBufferSize,
                        offscreenBufferOptions))
                {
                    p.buffers["difference0"] = vlk::OffscreenBuffer::create(
                        ctx, offscreenBufferSize, offscreenBufferOptions);
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
                        math::ortho(
                            0.F, static_cast<float>(offscreenBufferSize.w),
                            static_cast<float>(offscreenBufferSize.h), 0.F,
                            -1.F, 1.F));

                    _drawVideo(
                        videoData[0],
                        math::Box2i(
                            0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty()
                            ? std::make_shared<timeline::ImageOptions>(
                                  imageOptions[0])
                            : nullptr,
                        !displayOptions.empty() ? displayOptions[0]
                                                : timeline::DisplayOptions());

                    p.shaders["display"]->bind(p.frameIndex);
                    p.shaders["display"]->setUniform(
                        "transform.mvp", p.transform);
                }

                if (videoData.size() > 1)
                {
                    offscreenBufferOptions = vlk::OffscreenBufferOptions();
                    offscreenBufferOptions.colorType =
                        p.renderOptions.colorBuffer;
                    if (displayOptions.size() > 1)
                    {
                        offscreenBufferOptions.colorFilters =
                            displayOptions[1].imageFilters;
                    }
                    if (doCreate(
                            p.buffers["difference1"], offscreenBufferSize,
                            offscreenBufferOptions))
                    {
                        p.buffers["difference1"] = vlk::OffscreenBuffer::create(
                            ctx, offscreenBufferSize, offscreenBufferOptions);
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
                            math::ortho(
                                0.F, static_cast<float>(offscreenBufferSize.w),
                                static_cast<float>(offscreenBufferSize.h), 0.F,
                                -1.F, 1.F));

                        _drawVideo(
                            videoData[1],
                            math::Box2i(
                                0, 0, offscreenBufferSize.w,
                                offscreenBufferSize.h),
                            imageOptions.size() > 1
                                ? std::make_shared<timeline::ImageOptions>(
                                      imageOptions[1])
                                : nullptr,
                            displayOptions.size() > 1
                                ? displayOptions[1]
                                : timeline::DisplayOptions());
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
                    p.shaders["difference"]->setFBO("textureSampler", p.buffers["difference0"]);
                    p.shaders["difference"]->setFBO("textureSamplerB", p.buffers["difference1"]);

                    // glBindTexture(
                    //     GL_TEXTURE_2D,
                    //     p.buffers["difference0"]->getColorID());
                    // glBindTexture(
                    //     GL_TEXTURE_2D,
                    //     p.buffers["difference1"]->getColorID());

                    if (p.vbos["video"])
                    {
                        p.vbos["video"]->copy(convert(
                            geom::box(boxes[0], true),
                            p.vbos["video"]->getType()));
                    }
                    if (p.vaos["video"])
                    {
                        p.vaos["video"]->bind(p.frameIndex);
                        p.vaos["video"]->upload(p.vbos["video"]->getData());
                        p.vaos["video"]->draw(p.cmd, p.vbos["video"]);
                    }
                }
            }
        }

        void Render::_drawVideoTile(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            for (size_t i = 0; i < videoData.size() && i < boxes.size(); ++i)
            {
                _drawVideo(
                    videoData[i], boxes[i],
                    i < imageOptions.size()
                        ? std::make_shared<timeline::ImageOptions>(
                              imageOptions[i])
                        : nullptr,
                    i < displayOptions.size() ? displayOptions[i]
                                              : timeline::DisplayOptions());
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
            const timeline::VideoData& videoData, const math::Box2i& box,
            const std::shared_ptr<timeline::ImageOptions>& imageOptions,
            const timeline::DisplayOptions& displayOptions)
        {
            TLRENDER_P();

            // \@todo: \@bug: there's no such call in Vulkan.
            // GLint viewportPrev[4] = {0, 0, 0, 0};
            // glGetIntegerv(GL_VIEWPORT, viewportPrev);

            const auto transform = math::ortho(
                0.F, static_cast<float>(box.w()), static_cast<float>(box.h()),
                0.F, -1.F, 1.F);
            p.shaders["image"]->bind(p.frameIndex);
            p.shaders["image"]->setUniform(
                "transform.mvp", transform, vlk::kShaderVertex);

            const math::Size2i& offscreenBufferSize = box.getSize();
            vlk::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            offscreenBufferOptions.colorFilters = displayOptions.imageFilters;
            if (doCreate(
                    p.buffers["video"], offscreenBufferSize,
                    offscreenBufferOptions))
            {
                p.buffers["video"] = vlk::OffscreenBuffer::create(
                    ctx, offscreenBufferSize, offscreenBufferOptions);
            }

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
                            if (doCreate(
                                    p.buffers["dissolve"], offscreenBufferSize,
                                    offscreenBufferOptions))
                            {
                                // p.buffers["dissolve"] =
                                //     gl::OffscreenBuffer::create(
                                //         offscreenBufferSize,
                                //         offscreenBufferOptions);
                            }
                            if (doCreate(
                                    p.buffers["dissolve2"], offscreenBufferSize,
                                    offscreenBufferOptions))
                            {
                                // p.buffers["dissolve2"] =
                                //     gl::OffscreenBuffer::create(
                                //         offscreenBufferSize,
                                //         offscreenBufferOptions);
                            }
                            if (p.buffers["dissolve"])
                            {
                                // gl::OffscreenBufferBinding binding(
                                //     p.buffers["dissolve"]);
                                // glViewport(
                                //     0, 0, offscreenBufferSize.w,
                                //     offscreenBufferSize.h);
                                // glClearColor(0.F, 0.F, 0.F, 0.F);
                                // glClear(GL_COLOR_BUFFER_BIT);
                                float v = 1.F - layer.transitionValue;
                                auto dissolveImageOptions =
                                    imageOptions.get() ? *imageOptions
                                                       : layer.imageOptions;
                                dissolveImageOptions.alphaBlend =
                                    timeline::AlphaBlend::Straight;
                                drawImage(
                                    p.buffers["dissolve"],
                                    layer.image,
                                    image::getBox(
                                        layer.image->getAspect(),
                                        math::Box2i(
                                            0, 0, offscreenBufferSize.w,
                                            offscreenBufferSize.h)),
                                    image::Color4f(1.F, 1.F, 1.F, v),
                                    dissolveImageOptions);
                            }
                            if (p.buffers["dissolve2"])
                            {
                                // gl::OffscreenBufferBinding binding(
                                //     p.buffers["dissolve2"]);
                                // glViewport(
                                //     0, 0, offscreenBufferSize.w,
                                //     offscreenBufferSize.h);
                                // glClearColor(0.F, 0.F, 0.F, 0.F);
                                // glClear(GL_COLOR_BUFFER_BIT);
                                float v = layer.transitionValue;
                                auto dissolveImageOptions =
                                    imageOptions.get() ? *imageOptions
                                                       : layer.imageOptionsB;
                                dissolveImageOptions.alphaBlend =
                                    timeline::AlphaBlend::Straight;
                                drawImage(
                                    p.buffers["dissolve2"],
                                    layer.imageB,
                                    image::getBox(
                                        layer.imageB->getAspect(),
                                        math::Box2i(
                                            0, 0, offscreenBufferSize.w,
                                            offscreenBufferSize.h)),
                                    image::Color4f(1.F, 1.F, 1.F, v),
                                    dissolveImageOptions);
                            }
                            if (p.buffers["dissolve"] && p.buffers["dissolve2"])
                            {
                                // glBlendFuncSeparate(
                                //     GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                                //     GL_ONE);

                                p.shaders["dissolve"]->bind(p.frameIndex);
                                p.shaders["dissolve"]->setUniform(
                                    "transform.mvp", transform, vlk::kShaderVertex);
                                p.shaders["dissolve"]->setUniform(
                                    "color", image::Color4f(1.F, 1.F, 1.F));
                                p.shaders["dissolve"]->setFBO(
                                    "textureSampler", p.buffers["dissolve"]);

                                // glActiveTexture(
                                //     static_cast<GLenum>(GL_TEXTURE0));
                                // glBindTexture(
                                //     GL_TEXTURE_2D,
                                //     p.buffers["dissolve"]->getColorID());
                                if (p.vbos["video"])
                                {
                                    p.vbos["video"]->copy(convert(
                                        geom::box(
                                            math::Box2i(
                                                0, 0, offscreenBufferSize.w,
                                                offscreenBufferSize.h),
                                            true),
                                        p.vbos["video"]->getType()));
                                }
                                if (p.vaos["video"])
                                {
                                    p.vaos["video"]->bind(p.frameIndex);
                                    p.vaos["video"]->upload(p.vbos["video"]->getData());
                                    p.vaos["video"]->draw(p.cmd, p.vbos["video"]);
                                }

                                // glBindTexture(
                                //     GL_TEXTURE_2D,
                                //     p.buffers["dissolve2"]->getColorID());
                                if (p.vbos["video"])
                                {
                                    p.vbos["video"]->copy(convert(
                                        geom::box(
                                            math::Box2i(
                                                0, 0, offscreenBufferSize.w,
                                                offscreenBufferSize.h),
                                            true),
                                        p.vbos["video"]->getType()));
                                }
                                if (p.vaos["video"])
                                {
                                    p.vaos["video"]->bind(p.frameIndex);
                                    p.vaos["video"]->upload(p.vbos["video"]->getData());
                                    p.vaos["video"]->draw(p.cmd, p.vbos["video"]);
                                }
                            }
                        }
                        else if (layer.image)
                        {
                            drawImage(
                                p.buffers["video"],
                                layer.image,
                                image::getBox(
                                    layer.image->getAspect(),
                                    math::Box2i(
                                        0, 0, offscreenBufferSize.w,
                                        offscreenBufferSize.h)),
                                image::Color4f(
                                    1.F, 1.F, 1.F, 1.F - layer.transitionValue),
                                imageOptions.get() ? *imageOptions
                                                   : layer.imageOptions);
                        }
                        else if (layer.imageB)
                        {
                            drawImage(
                                p.buffers["video"],
                                layer.imageB,
                                image::getBox(
                                    layer.imageB->getAspect(),
                                    math::Box2i(
                                        0, 0, offscreenBufferSize.w,
                                        offscreenBufferSize.h)),
                                image::Color4f(
                                    1.F, 1.F, 1.F, layer.transitionValue),
                                imageOptions.get() ? *imageOptions
                                                   : layer.imageOptionsB);
                        }
                        break;
                    }
                    default:
                        if (layer.image)
                        {
                            drawImage(
                                p.buffers["video"],
                                layer.image,
                                image::getBox(
                                    layer.image->getAspect(),
                                    math::Box2i(
                                        0, 0, offscreenBufferSize.w,
                                        offscreenBufferSize.h)),
                                image::Color4f(1.F, 1.F, 1.F),
                                imageOptions.get() ? *imageOptions
                                                   : layer.imageOptions);
                        }
                        break;
                    }
                }
            }

            if (p.buffers["video"])
            {
    
                // glBlendFuncSeparate(
                //     GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                //     GL_ONE_MINUS_SRC_ALPHA);
                
                // Prepare video buffer for sampling in FBO pass.
                p.buffers["video"]->transitionToShaderRead(p.cmd);

                //                     pipeline    shader    mesh
                _createPipeline(p.fbo, "display", "display", "video", true);
                
                // Begin the new compositing render pass.
                p.fbo->beginCompositingRenderPass(p.cmd);
                
                p.fbo->setupViewportAndScissor(p.cmd);

                p.shaders["display"]->bind(p.frameIndex);
                p.shaders["display"]->setFBO("textureSampler", p.buffers["video"]);
                timeline::Levels uboLevels = displayOptions.levels;
                uboLevels.gamma = uboLevels.gamma > 0.F ?
                                  (1.F / uboLevels.gamma) : 1000000.F;
                p.shaders["display"]->setUniform("uboLevels", uboLevels);
                p.shaders["display"]->setUniform("uboNormalize", displayOptions.normalize);
                struct UBOColor
                {
                    alignas(4)  bool enabled;
                    alignas(16) math::Vector3f add;
                    alignas(16) math::Matrix4x4f matrix;
                    alignas(4)  bool invert;
                };

                UBOColor uboColor;
                const bool colorMatrixEnabled =
                    displayOptions.color != timeline::Color() &&
                    displayOptions.color.enabled;
                uboColor.enabled = colorMatrixEnabled;
                uboColor.add = uboColor.enabled ? displayOptions.color.add :
                               math::Vector3f(0.F, 0.F, 0.F);
                uboColor.matrix = color(displayOptions.color);
                uboColor.invert = displayOptions.color.invert;
                
                
                p.shaders["display"]->setUniform("uboColor", uboColor);
                p.shaders["display"]->setUniform("uboEXRDisplay", displayOptions.exrDisplay);
                struct UBO
                {
                    alignas(4) int   channels;
                    alignas(4) int   mirrorX;
                    alignas(4) int   mirrorY;
                    alignas(4) float softClip;
                    alignas(4) int   videoLevels;
                    alignas(4) int   invalidValues;
                };
                UBO ubo;
                ubo.channels = static_cast<int>(displayOptions.channels);
                ubo.mirrorX  = displayOptions.mirror.x;
                ubo.mirrorY  = displayOptions.mirror.y;
                ubo.softClip = displayOptions.softClip.enabled ? displayOptions.softClip.value : 0.F;
                ubo.videoLevels = static_cast<int>(displayOptions.videoLevels);
                ubo.invalidValues = displayOptions.invalidValues;
                p.shaders["display"]->setUniform("ubo", ubo);
                
#if defined(TLRENDER_OCIO)
                if (p.ocioData)
                {
                    for (size_t i = 0; i < p.ocioData->textures.size(); ++i)
                    {
                        p.shaders["display"]->setTexture("sampler", p.ocioData->textures[i]);
                    }
                }
                if (p.lutData)
                {
                    for (size_t i = 0; i < p.lutData->textures.size(); ++i)
                    {
                        p.shaders["display"]->setTexture("sampler", p.lutData->textures[i]);
                    }
                }
#endif // TLRENDER_OCIO
#if defined(TLRENDER_LIBPLACEBO)
                if (p.placeboData)
                {
                    for (size_t i = 0; i < p.placeboData->textures.size(); ++i)
                    {
                        p.shaders["display"]->setTexture("sampler", p.placeboData->textures[i]);
                    }
                }
#endif // TLRENDER_LIBPLACEBO

                _bindDescriptorSets("display", "display");
            
                if (p.vbos["video"])
                {
                    p.vbos["video"]->copy(convert(
                        geom::box(box, true), p.vbos["video"]->getType()));
                }
                if (p.vaos["video"])
                {
                    p.vaos["video"]->bind(p.frameIndex);
                    p.vaos["video"]->upload(p.vbos["video"]->getData());
                    p.vaos["video"]->draw(p.cmd, p.vbos["video"]);
                }
                
                p.fbo->endCompositingRenderPass(p.cmd);
                p.buffers["video"]->transitionToColorAttachment(p.cmd);
            }
        }
    } // namespace timeline_vlk
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineGL/RenderPrivate.h>

#include <tlGL/GL.h>
#include <tlGL/Mesh.h>
#include <tlGL/Util.h>

#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timeline_gl
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
            //! \todo Render the background only if there is valid video data
            //! and a valid layer?
            if (!videoFrame.empty() && !videoFrame.front().layers.empty())
            {
                _drawBackground(boxes, backgroundOptions);
            }
            switch (compareOptions.mode)
            {
            case timeline::CompareMode::A:
                _drawVideoA(
                    videoFrame, boxes, imageOptions, displayOptions,
                    compareOptions);
                break;
            case timeline::CompareMode::B:
                _drawVideoB(
                    videoFrame, boxes, imageOptions, displayOptions,
                    compareOptions);
                break;
            case timeline::CompareMode::Wipe:
                _drawVideoWipe(
                    videoFrame, boxes, imageOptions, displayOptions,
                    compareOptions);
                break;
            case timeline::CompareMode::Overlay:
                _drawVideoOverlay(
                    videoFrame, boxes, imageOptions, displayOptions,
                    compareOptions);
                break;
            case timeline::CompareMode::Difference:
                if (videoFrame.size() > 1)
                {
                    _drawVideoDifference(
                        videoFrame, boxes, imageOptions, displayOptions,
                        compareOptions);
                }
                else
                {
                    _drawVideoA(
                        videoFrame, boxes, imageOptions, displayOptions,
                        compareOptions);
                }
                break;
            case timeline::CompareMode::Multiply:
                if (videoFrame.size() > 1)
                {
                    _drawVideoMultiply(
                        videoFrame, boxes, imageOptions, displayOptions,
                        compareOptions);
                }
                else
                {
                    _drawVideoA(
                        videoFrame, boxes, imageOptions, displayOptions,
                        compareOptions);
                }
                break;
            case timeline::CompareMode::Add:
                if (videoFrame.size() > 1)
                {
                    _drawVideoAdd(
                        videoFrame, boxes, imageOptions, displayOptions,
                        compareOptions);
                }
                else
                {
                    _drawVideoA(
                        videoFrame, boxes, imageOptions, displayOptions,
                        compareOptions);
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
                _drawVideoTile(
                    videoFrame, boxes, imageOptions, displayOptions,
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
            for (const auto& box : boxes)
            {
                switch (options.type)
                {
                case timeline::Background::Solid:
                    drawRect(box, options.color0);
                    break;
                case timeline::Background::Checkers:
                    drawColorMesh(
                        geom::checkers(
                            box, options.color0, options.color1,
                            options.checkersSize),
                        math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));
                    break;
                case timeline::Background::Gradient:
                {
                    geom::TriangleMesh2 mesh;
                    mesh.v.push_back(math::Vector2f(box.min.x, box.min.y));
                    mesh.v.push_back(math::Vector2f(box.max.x, box.min.y));
                    mesh.v.push_back(math::Vector2f(box.max.x, box.max.y));
                    mesh.v.push_back(math::Vector2f(box.min.x, box.max.y));
                    mesh.c.push_back(math::Vector4f(
                        options.color0.r, options.color0.g, options.color0.b,
                        options.color0.a));
                    mesh.c.push_back(math::Vector4f(
                        options.color1.r, options.color1.g, options.color1.b,
                        options.color1.a));
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
                    drawColorMesh(
                        mesh, math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));
                    break;
                }
                default:
                    break;
                }
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

            if (videoFrame.empty() || boxes.empty())
                return;

            _drawVideo(
                videoFrame[0], boxes[0],
                !imageOptions.empty()
                ? std::make_shared<timeline::ImageOptions>(imageOptions[0])
                : nullptr,
                !displayOptions.empty() ? displayOptions[0]
                : timeline::DisplayOptions());
        }

        void Render::_drawVideoB(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            if (!videoFrame.size() > 1 || !boxes.size() > 1)
                return;

            _drawVideo(
                videoFrame[1], boxes[1],
                imageOptions.size() > 1
                ? std::make_shared<timeline::ImageOptions>(
                    imageOptions[1])
                : nullptr,
                displayOptions.size() > 1 ? displayOptions[1]
                : timeline::DisplayOptions());
        }

        void Render::_drawVideoWipe(
            const std::vector<timeline::VideoFrame>& videoFrame,
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

            gl::SetAndRestore stencilTest(GL_STENCIL_TEST, GL_TRUE);

            glViewport(
                p.viewport.x(),
                p.renderSize.h - p.viewport.h() - p.viewport.y(),
                p.viewport.w(), p.viewport.h());
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            p.shaders["wipe"]->bind();
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
                    p.vaos["wipe"]->bind();
                    p.vaos["wipe"]->draw(
                        GL_TRIANGLES, 0, p.vbos["wipe"]->getSize());
                }
            }
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            if (!videoFrame.empty() && !boxes.empty())
            {
                _drawVideo(
                    videoFrame[0], boxes[0],
                    !imageOptions.empty()
                    ? std::make_shared<timeline::ImageOptions>(
                        imageOptions[0])
                    : nullptr,
                    !displayOptions.empty() ? displayOptions[0]
                    : timeline::DisplayOptions());
            }

            glViewport(
                p.viewport.x(),
                p.renderSize.h - p.viewport.h() - p.viewport.y(),
                p.viewport.w(), p.viewport.h());
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            p.shaders["wipe"]->bind();
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
                    p.vaos["wipe"]->bind();
                    p.vaos["wipe"]->draw(
                        GL_TRIANGLES, 0, p.vbos["wipe"]->getSize());
                }
            }
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            if (videoFrame.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoFrame[1], boxes[1],
                    imageOptions.size() > 1
                    ? std::make_shared<timeline::ImageOptions>(
                        imageOptions[1])
                    : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1]
                    : timeline::DisplayOptions());
            }
        }

        void Render::_drawVideoOverlay(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (videoFrame.empty() || boxes.empty())
                return;

            if (videoFrame.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoFrame[1], boxes[1],
                    imageOptions.size() > 1
                    ? std::make_shared<timeline::ImageOptions>(
                        imageOptions[1])
                    : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1]
                    : timeline::DisplayOptions());
            }

            const math::Size2i offscreenBufferSize(boxes[0].w(), boxes[0].h());
            gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            if (!displayOptions.empty())
            {
                offscreenBufferOptions.colorFilters =
                    imageFilters(imageOptions, 0);
            }
            if (doCreate(p.buffers["overlay"], offscreenBufferSize,
                         offscreenBufferOptions))
            {
                p.buffers["overlay"] = gl::OffscreenBuffer::create(
                    offscreenBufferSize, offscreenBufferOptions);
            }

            if (p.buffers["overlay"])
            {
                const gl::SetAndRestore scissorTest(
                    GL_SCISSOR_TEST, GL_FALSE);

                gl::OffscreenBufferBinding binding(p.buffers["overlay"]);
                glViewport(
                    0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);

                p.shaders["display"]->bind();
                p.shaders["display"]->setUniform(
                    "transform.mvp",
                    math::ortho(
                        0.F, static_cast<float>(offscreenBufferSize.w),
                        static_cast<float>(offscreenBufferSize.h), 0.F,
                        -1.F, 1.F));

                _drawVideo(
                    videoFrame[0],
                    math::Box2i(
                        0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                    !imageOptions.empty()
                    ? std::make_shared<timeline::ImageOptions>(
                        imageOptions[0])
                    : nullptr,
                    !displayOptions.empty() ? displayOptions[0]
                    : timeline::DisplayOptions());

                p.shaders["display"]->bind();
                p.shaders["display"]->setUniform(
                    "transform.mvp", p.transform);
            }

            if (p.buffers["overlay"])
            {
                glBlendFuncSeparate(
                    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                glViewport(
                    p.viewport.x(),
                    p.renderSize.h - p.viewport.h() - p.viewport.y(),
                    p.viewport.w(), p.viewport.h());

                p.shaders["overlay"]->bind();
                p.shaders["overlay"]->setUniform(
                    "color",
                    image::Color4f(1.F, 1.F, 1.F, compareOptions.overlay));
                p.shaders["overlay"]->setUniform("textureSampler", 0);

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                glBindTexture(
                    GL_TEXTURE_2D, p.buffers["overlay"]->getColorID());

                if (p.vbos["video"] && !boxes.empty())
                {
                    p.vbos["video"]->copy(convert(
                                              geom::box(boxes[0], true),
                                              p.vbos["video"]->getType()));
                }
                if (p.vaos["video"])
                {
                    p.vaos["video"]->bind();
                    p.vaos["video"]->draw(
                        GL_TRIANGLES, 0, p.vbos["video"]->getSize());
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

            if (_drawVideoPair(
                    videoFrame, boxes, imageOptions, displayOptions))
            {
                p.shaders["difference"]->bind();
                p.shaders["difference"]->setUniform(
                    "gain",
                    compareOptions.differenceGain);
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

            const math::Size2i offscreenBufferSize(
                boxes[0].w(), boxes[0].h());
            gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            if (!displayOptions.empty())
            {
                offscreenBufferOptions.colorFilters =
                    imageFilters(imageOptions, 0);
            }
            if (doCreate(
                    p.buffers["multiply0"], offscreenBufferSize,
                    offscreenBufferOptions))
            {
                p.buffers["multiply0"] = gl::OffscreenBuffer::create(
                    offscreenBufferSize, offscreenBufferOptions);
            }

            if (p.buffers["multiply0"])
            {
                const gl::SetAndRestore scissorTest(
                    GL_SCISSOR_TEST, GL_FALSE);

                gl::OffscreenBufferBinding binding(
                    p.buffers["multiply0"]);
                glViewport(
                    0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);

                p.shaders["display"]->bind();
                p.shaders["display"]->setUniform(
                    "transform.mvp",
                    math::ortho(
                        0.F, static_cast<float>(offscreenBufferSize.w),
                        static_cast<float>(offscreenBufferSize.h), 0.F,
                        -1.F, 1.F));

                _drawVideo(
                    videoFrame[0],
                    math::Box2i(
                        0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                    !imageOptions.empty()
                    ? std::make_shared<timeline::ImageOptions>(
                        imageOptions[0])
                    : nullptr,
                    !displayOptions.empty() ? displayOptions[0]
                    : timeline::DisplayOptions());

                p.shaders["display"]->bind();
                p.shaders["display"]->setUniform(
                    "transform.mvp", p.transform);
            }

            if (videoFrame.size() > 1)
            {
                offscreenBufferOptions = gl::OffscreenBufferOptions();
                offscreenBufferOptions.colorType =
                    p.renderOptions.colorBuffer;
                if (displayOptions.size() > 1)
                {
                    offscreenBufferOptions.colorFilters =
                        imageFilters(imageOptions, 1);
                }
                if (doCreate(
                        p.buffers["multiply1"], offscreenBufferSize,
                        offscreenBufferOptions))
                {
                    p.buffers["multiply1"] = gl::OffscreenBuffer::create(
                        offscreenBufferSize, offscreenBufferOptions);
                }

                if (p.buffers["multiply1"])
                {
                    const gl::SetAndRestore scissorTest(
                        GL_SCISSOR_TEST, GL_FALSE);

                    gl::OffscreenBufferBinding binding(
                        p.buffers["multiply1"]);
                    glViewport(
                        0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                    glClearColor(0.F, 0.F, 0.F, 0.F);
                    glClear(GL_COLOR_BUFFER_BIT);

                    p.shaders["display"]->bind();
                    p.shaders["display"]->setUniform(
                        "transform.mvp",
                        math::ortho(
                            0.F, static_cast<float>(offscreenBufferSize.w),
                            static_cast<float>(offscreenBufferSize.h), 0.F,
                            -1.F, 1.F));

                    _drawVideo(
                        videoFrame[1],
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
                p.buffers["multiply1"].reset();
            }

            if (p.buffers["multiply0"] && p.buffers["multiply1"])
            {
                glDisable(GL_BLEND);

                glViewport(
                    p.viewport.x(),
                    p.renderSize.h - p.viewport.h() - p.viewport.y(),
                    p.viewport.w(), p.viewport.h());

                p.shaders["multiply"]->bind();
                p.shaders["multiply"]->setUniform("textureSampler", 0);
                p.shaders["multiply"]->setUniform("textureSamplerB", 1);

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                glBindTexture(
                    GL_TEXTURE_2D, p.buffers["multiply0"]->getColorID());

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE1));
                glBindTexture(
                    GL_TEXTURE_2D, p.buffers["multiply1"]->getColorID());

                if (p.vbos["video"] && !boxes.empty())
                {
                    p.vbos["video"]->copy(convert(
                                              geom::box(boxes[0], true),
                                              p.vbos["video"]->getType()));
                }
                if (p.vaos["video"])
                {
                    p.vaos["video"]->bind();
                    p.vaos["video"]->draw(
                        GL_TRIANGLES, 0, p.vbos["video"]->getSize());
                }

                glEnable(GL_BLEND);
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
            if (!videoFrame.empty() && !boxes.empty())
            {
                const math::Size2i offscreenBufferSize(
                    boxes[0].w(), boxes[0].h());
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        imageFilters(imageOptions, 0);
                }
                if (doCreate(
                        p.buffers["add0"], offscreenBufferSize,
                        offscreenBufferOptions))
                {
                    p.buffers["add0"] = gl::OffscreenBuffer::create(
                        offscreenBufferSize, offscreenBufferOptions);
                }

                if (p.buffers["add0"])
                {
                    const gl::SetAndRestore scissorTest(
                        GL_SCISSOR_TEST, GL_FALSE);

                    gl::OffscreenBufferBinding binding(
                        p.buffers["add0"]);
                    glViewport(
                        0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                    glClearColor(0.F, 0.F, 0.F, 0.F);
                    glClear(GL_COLOR_BUFFER_BIT);

                    p.shaders["display"]->bind();
                    p.shaders["display"]->setUniform(
                        "transform.mvp",
                        math::ortho(
                            0.F, static_cast<float>(offscreenBufferSize.w),
                            static_cast<float>(offscreenBufferSize.h), 0.F,
                            -1.F, 1.F));

                    _drawVideo(
                        videoFrame[0],
                        math::Box2i(
                            0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty()
                            ? std::make_shared<timeline::ImageOptions>(
                                  imageOptions[0])
                            : nullptr,
                        !displayOptions.empty() ? displayOptions[0]
                                                : timeline::DisplayOptions());

                    p.shaders["display"]->bind();
                    p.shaders["display"]->setUniform(
                        "transform.mvp", p.transform);
                }

                if (videoFrame.size() > 1)
                {
                    offscreenBufferOptions = gl::OffscreenBufferOptions();
                    offscreenBufferOptions.colorType =
                        p.renderOptions.colorBuffer;
                    if (displayOptions.size() > 1)
                    {
                        offscreenBufferOptions.colorFilters =
                            imageFilters(imageOptions, 1);
                        offscreenBufferOptions.colorFilters =
                            displayOptions[1].imageFilters;
                    }
                    if (doCreate(
                            p.buffers["add1"], offscreenBufferSize,
                            offscreenBufferOptions))
                    {
                        p.buffers["add1"] = gl::OffscreenBuffer::create(
                            offscreenBufferSize, offscreenBufferOptions);
                    }

                    if (p.buffers["add1"])
                    {
                        const gl::SetAndRestore scissorTest(
                            GL_SCISSOR_TEST, GL_FALSE);

                        gl::OffscreenBufferBinding binding(
                            p.buffers["add1"]);
                        glViewport(
                            0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);

                        p.shaders["display"]->bind();
                        p.shaders["display"]->setUniform(
                            "transform.mvp",
                            math::ortho(
                                0.F, static_cast<float>(offscreenBufferSize.w),
                                static_cast<float>(offscreenBufferSize.h), 0.F,
                                -1.F, 1.F));

                        _drawVideo(
                            videoFrame[1],
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
                    p.buffers["add1"].reset();
                }

                if (p.buffers["add0"] && p.buffers["add1"])
                {
                    glDisable(GL_BLEND);

                    glViewport(
                        p.viewport.x(),
                        p.renderSize.h - p.viewport.h() - p.viewport.y(),
                        p.viewport.w(), p.viewport.h());

                    p.shaders["add"]->bind();
                    p.shaders["add"]->setUniform("textureSampler", 0);
                    p.shaders["add"]->setUniform("textureSamplerB", 1);

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                    glBindTexture(
                        GL_TEXTURE_2D, p.buffers["add0"]->getColorID());

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE1));
                    glBindTexture(
                        GL_TEXTURE_2D, p.buffers["add1"]->getColorID());

                    if (p.vbos["video"] && !boxes.empty())
                    {
                        p.vbos["video"]->copy(convert(
                            geom::box(boxes[0], true),
                            p.vbos["video"]->getType()));
                    }
                    if (p.vaos["video"])
                    {
                        p.vaos["video"]->bind();
                        p.vaos["video"]->draw(
                            GL_TRIANGLES, 0, p.vbos["video"]->getSize());
                    }

                    glEnable(GL_BLEND);
                }
            }
        }

        void Render::_drawVideoTile(
            const std::vector<timeline::VideoFrame>& videoFrame,
            const std::vector<math::Box2i>& boxes,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            for (size_t i = 0; i < videoFrame.size() && i < boxes.size(); ++i)
            {
                _drawVideo(
                    videoFrame[i], boxes[i],
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
                p.shaders["butterfly"]->bind();
                _drawVideoPairShader("butterfly", boxes[0]);
            }
        }

        void Render::_drawVideo(
            const timeline::VideoFrame& videoFrame, const math::Box2i& box,
            const std::shared_ptr<timeline::ImageOptions>& imageOptions,
            const timeline::DisplayOptions& displayOptions)
        {
            TLRENDER_P();

            GLint viewportPrev[4] = {0, 0, 0, 0};
            glGetIntegerv(GL_VIEWPORT, viewportPrev);

            const auto transform = math::ortho(
                0.F, static_cast<float>(box.w()), static_cast<float>(box.h()),
                0.F, -1.F, 1.F);
            p.shaders["image"]->bind();
            p.shaders["image"]->setUniform("transform.mvp", transform);

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

            gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
            offscreenBufferOptions.colorFilters = displayOptions.imageFilters;
            if (doCreate(
                    p.buffers["video"], offscreenBufferSize,
                    offscreenBufferOptions))
            {
                p.buffers["video"] = gl::OffscreenBuffer::create(
                    offscreenBufferSize, offscreenBufferOptions);
            }

            if (p.buffers["video"])
            {
                const gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                gl::OffscreenBufferBinding binding(p.buffers["video"]);
                glViewport(0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);

                bool clearRenderPass = true;
                for (const auto& layer : videoFrame.layers)
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
                                p.buffers["dissolve"] =
                                    gl::OffscreenBuffer::create(
                                        offscreenBufferSize,
                                        offscreenBufferOptions);
                            }
                            if (doCreate(
                                    p.buffers["dissolve2"], offscreenBufferSize,
                                    offscreenBufferOptions))
                            {
                                p.buffers["dissolve2"] =
                                    gl::OffscreenBuffer::create(
                                        offscreenBufferSize,
                                        offscreenBufferOptions);
                            }
                            if (p.buffers["dissolve"])
                            {
                                gl::OffscreenBufferBinding binding(
                                    p.buffers["dissolve"]);
                                glViewport(
                                    0, 0, offscreenBufferSize.w,
                                    offscreenBufferSize.h);
                                glClearColor(0.F, 0.F, 0.F, 0.F);
                                glClear(GL_COLOR_BUFFER_BIT);
                                float v = 1.F - layer.transitionValue;
                                auto dissolveImageOptions =
                                    imageOptions.get() ? *imageOptions
                                                       : layer.imageOptions;
                                dissolveImageOptions.alphaBlend =
                                    timeline::AlphaBlend::Straight;
                                drawImage(
                                    layer.image,
                                    getBox(
                                        layerBox(layer.bounds),
                                        layer.image->getInfo(),
                                        displayOptions.aspect),
                                    image::Color4f(1.F, 1.F, 1.F, v),
                                    dissolveImageOptions);
                            }
                            if (p.buffers["dissolve2"])
                            {
                                gl::OffscreenBufferBinding binding(
                                    p.buffers["dissolve2"]);
                                glViewport(
                                    0, 0, offscreenBufferSize.w,
                                    offscreenBufferSize.h);
                                glClearColor(0.F, 0.F, 0.F, 0.F);
                                glClear(GL_COLOR_BUFFER_BIT);
                                float v = layer.transitionValue;
                                auto dissolveImageOptions =
                                    imageOptions.get() ? *imageOptions
                                                       : layer.imageOptionsB;
                                dissolveImageOptions.alphaBlend =
                                    timeline::AlphaBlend::Straight;
                                drawImage(
                                    layer.imageB,
                                    getBox(
                                        layerBox(layer.boundsB),
                                        layer.imageB->getInfo(),
                                        displayOptions.aspect),
                                    image::Color4f(1.F, 1.F, 1.F, v),
                                    dissolveImageOptions);
                            }
                            if (p.buffers["dissolve"] && p.buffers["dissolve2"])
                            {
                                if (clearRenderPass)
                                {
                                    glDisable(GL_BLEND);
                                }
                                else
                                {
                                    glEnable(GL_BLEND);

                                    glBlendFuncSeparate(
                                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                                        GL_ONE);
                                }

                                p.shaders["dissolve"]->bind();
                                p.shaders["dissolve"]->setUniform(
                                    "transform.mvp", transform);
                                p.shaders["dissolve"]->setUniform(
                                    "color", image::Color4f(1.F, 1.F, 1.F));
                                p.shaders["dissolve"]->setUniform(
                                    "textureSampler", 0);

                                glActiveTexture(
                                    static_cast<GLenum>(GL_TEXTURE0));
                                glBindTexture(
                                    GL_TEXTURE_2D,
                                    p.buffers["dissolve"]->getColorID());
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
                                    p.vaos["video"]->bind();
                                    p.vaos["video"]->draw(
                                        GL_TRIANGLES, 0,
                                        p.vbos["video"]->getSize());
                                }

                                glEnable(GL_BLEND);
                                if (clearRenderPass)
                                {
                                    glBlendFunc(GL_ONE, GL_ONE);
                                }
                                else
                                {
                                    glBlendFuncSeparate(
                                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                                        GL_ONE_MINUS_SRC_ALPHA);
                                }
                                glBindTexture(
                                    GL_TEXTURE_2D,
                                    p.buffers["dissolve2"]->getColorID());
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
                                    p.vaos["video"]->bind();
                                    p.vaos["video"]->draw(
                                        GL_TRIANGLES, 0,
                                        p.vbos["video"]->getSize());
                                }
                            }
                        }
                        else if (layer.image)
                        {
                            drawImage(
                                layer.image,
                                getBox(
                                    layerBox(layer.bounds),
                                    layer.image->getInfo(),
                                    displayOptions.aspect),
                                image::Color4f(
                                    1.F, 1.F, 1.F, 1.F - layer.transitionValue),
                                imageOptions.get() ? *imageOptions
                                                   : layer.imageOptions);
                        }
                        else if (layer.imageB)
                        {
                            drawImage(
                                layer.imageB,
                                getBox(
                                    layerBox(layer.boundsB),
                                    layer.imageB->getInfo(),
                                    displayOptions.aspect),
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
                                layer.image,
                                getBox(
                                    layerBox(layer.bounds),
                                    layer.image->getInfo(),
                                    displayOptions.aspect),
                                image::Color4f(1.F, 1.F, 1.F),
                                imageOptions.get() ? *imageOptions
                                                   : layer.imageOptions);
                        }
                        break;
                    }
                    clearRenderPass = false;
                }
            }

            if (p.buffers["video"])
            {
                glBlendFuncSeparate(
                    GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                    GL_ONE_MINUS_SRC_ALPHA);

                glViewport(
                    viewportPrev[0], viewportPrev[1], viewportPrev[2],
                    viewportPrev[3]);

                p.shaders["display"]->bind();
                p.shaders["display"]->setUniform("textureSampler", 0);
                p.shaders["display"]->setUniform(
                    "channels", static_cast<int>(displayOptions.channels));
                p.shaders["display"]->setUniform(
                    "mirrorX", displayOptions.mirror.x);
                p.shaders["display"]->setUniform(
                    "mirrorY", displayOptions.mirror.y);
                const bool colorMatrixEnabled =
                    displayOptions.color != timeline::Color() &&
                    displayOptions.color.enabled;
                p.shaders["display"]->setUniform(
                    "colorEnabled", colorMatrixEnabled);
                p.shaders["display"]->setUniform(
                    "colorAdd", displayOptions.color.add);
                if (colorMatrixEnabled)
                {
                    p.shaders["display"]->setUniform(
                        "colorMatrix", color(displayOptions.color));
                }
                p.shaders["display"]->setUniform(
                    "colorInvert", displayOptions.color.enabled
                                       ? displayOptions.color.invert
                                       : false);
                p.shaders["display"]->setUniform(
                    "levelsEnabled", displayOptions.levels.enabled);
                p.shaders["display"]->setUniform(
                    "levels.inLow", displayOptions.levels.inLow);
                p.shaders["display"]->setUniform(
                    "levels.inHigh", displayOptions.levels.inHigh);
                p.shaders["display"]->setUniform(
                    "levels.gamma", displayOptions.levels.gamma > 0.F
                                        ? (1.F / displayOptions.levels.gamma)
                                        : 1000000.F);
                p.shaders["display"]->setUniform(
                    "levels.outLow", displayOptions.levels.outLow);
                p.shaders["display"]->setUniform(
                    "levels.outHigh", displayOptions.levels.outHigh);
                p.shaders["display"]->setUniform(
                    "exrDisplayEnabled", displayOptions.exrDisplay.enabled);
                if (displayOptions.exrDisplay.enabled)
                {
                    const float v = powf(
                        2.F, displayOptions.exrDisplay.exposure + 2.47393F);
                    const float d = displayOptions.exrDisplay.defog;
                    const float k =
                        powf(2.F, displayOptions.exrDisplay.kneeLow);
                    const float f = knee2(
                        powf(2.F, displayOptions.exrDisplay.kneeHigh) - k,
                        powf(2.F, 3.5F) - k);
                    p.shaders["display"]->setUniform("exrDisplay.v", v);
                    p.shaders["display"]->setUniform("exrDisplay.d", d);
                    p.shaders["display"]->setUniform("exrDisplay.k", k);
                    p.shaders["display"]->setUniform("exrDisplay.f", f);
                    const float gamma =
                        displayOptions.levels.gamma > 0.F
                            ? (1.F / displayOptions.levels.gamma)
                            : 1000000.F;
                    p.shaders["display"]->setUniform("exrDisplay.g", gamma);
                }
                p.shaders["display"]->setUniform(
                    "softClip", displayOptions.softClip.enabled
                                    ? displayOptions.softClip.value
                                    : 0.F);
                p.shaders["display"]->setUniform(
                    "videoLevels",
                    static_cast<int>(displayOptions.videoLevels));
                p.shaders["display"]->setUniform(
                    "normalizeEnabled", displayOptions.normalize.enabled);
                if (displayOptions.normalize.enabled)
                {
                    p.shaders["display"]->setUniform(
                        "normalizeDisplay.minimum",
                        displayOptions.normalize.minimum);
                    p.shaders["display"]->setUniform(
                        "normalizeDisplay.maximum",
                        displayOptions.normalize.maximum);
                }
                p.shaders["display"]->setUniform(
                    "invalidValues", displayOptions.invalidValues);

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                glBindTexture(GL_TEXTURE_2D, p.buffers["video"]->getColorID());
                size_t texturesOffset = 1;
#if defined(TLRENDER_OCIO)
                if (p.ocioData)
                {
                    for (size_t i = 0; i < p.ocioData->textures.size(); ++i)
                    {
                        glActiveTexture(GL_TEXTURE0 + texturesOffset + i);
                        glBindTexture(
                            p.ocioData->textures[i].type,
                            p.ocioData->textures[i].id);
                    }
                    texturesOffset += p.ocioData->textures.size();
                }
                if (p.lutData)
                {
                    for (size_t i = 0; i < p.lutData->textures.size(); ++i)
                    {
                        glActiveTexture(GL_TEXTURE0 + texturesOffset + i);
                        glBindTexture(
                            p.lutData->textures[i].type,
                            p.lutData->textures[i].id);
                    }
                    texturesOffset += p.lutData->textures.size();
                }
#endif // TLRENDER_OCIO
#if defined(TLRENDER_LIBPLACEBO)
                if (p.placeboData)
                {
                    for (size_t i = 0; i < p.placeboData->textures.size(); ++i)
                    {
                        glActiveTexture(GL_TEXTURE0 + texturesOffset + i);
                        glBindTexture(
                            p.placeboData->textures[i].type,
                            p.placeboData->textures[i].id);
                    }
                    texturesOffset += p.placeboData->textures.size();
                }
#endif // TLRENDER_LIBPLACEBO

                if (p.vbos["video"])
                {
                    p.vbos["video"]->copy(convert(
                        geom::box(box, true), p.vbos["video"]->getType()));
                }
                if (p.vaos["video"])
                {
                    p.vaos["video"]->bind();
                    p.vaos["video"]->draw(
                        GL_TRIANGLES, 0, p.vbos["video"]->getSize());
                }
            }

            p.shaders["image"]->bind();
            p.shaders["image"]->setUniform("transform.mvp", p.transform);
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
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.renderOptions.colorBuffer;
                if (displayOptions.size() > i)
                {
                    offscreenBufferOptions.colorFilters = imageFilters(imageOptions, i);
                }
                if (doCreate(
                    p.buffers[name],
                    offscreenBufferSize,
                    offscreenBufferOptions))
                {
                    p.buffers[name] = gl::OffscreenBuffer::create(
                        offscreenBufferSize,
                        offscreenBufferOptions);
                }
                if (!p.buffers[name])
                {
                    continue;
                }

                const gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);
                gl::OffscreenBufferBinding binding(p.buffers[name]);
                glViewport(
                    0,
                    0,
                    offscreenBufferSize.w,
                    offscreenBufferSize.h);
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);

                p.shaders["display"]->bind();
                p.shaders["display"]->setUniform(
                    "transform.mvp",
                    math::ortho(
                        0.F,
                        static_cast<float>(offscreenBufferSize.w),
                        static_cast<float>(offscreenBufferSize.h),
                        0.F,
                        -1.F,
                        1.F));

                _drawVideo(
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
                p.shaders["display"]->bind();
                p.shaders["display"]->setUniform("transform.mvp", getTransform());
            }

            return p.buffers["compare0"] && p.buffers["compare1"];
        }

        void Render::_drawVideoPairShader(
            const std::string& shader,
            const math::Box2i& box)
        {
            TLRENDER_P();
            glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

            const math::Size2i renderSize = getRenderSize();
            const math::Box2i viewport = getViewport();
            glViewport(
                viewport.x(),
                renderSize.h - viewport.h() - viewport.y(),
                viewport.w(),
                viewport.h());

            p.shaders[shader]->bind();
            p.shaders[shader]->setUniform("textureSampler", 0);
            p.shaders[shader]->setUniform("textureSamplerB", 1);

            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
            glBindTexture(GL_TEXTURE_2D, p.buffers["compare0"]->getColorID());

            glActiveTexture(static_cast<GLenum>(GL_TEXTURE1));
            glBindTexture(GL_TEXTURE_2D, p.buffers["compare1"]->getColorID());

            if (p.vbos["video"])
            {
                p.vbos["video"]->copy(convert(
                    geom::box(box, true),
                    p.vbos["video"]->getType()));
            }
            if (p.vaos["video"])
            {
                p.vaos["video"]->bind();
                p.vaos["video"]->draw(GL_TRIANGLES, 0, p.vbos["video"]->getSize());
            }
        }

    } // namespace timeline_gl
} // namespace tl

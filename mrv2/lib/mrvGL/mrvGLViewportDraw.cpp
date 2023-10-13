// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlUI/DrawUtil.h>
#include <tlGL/Util.h>

#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvMemory.h"
#include "mrvCore/mrvUtil.h"

#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvGL/mrvGLDefines.h"
#include "mrvGL/mrvGLErrors.h"
#include "mrvGL/mrvGLViewport.h"
#include "mrvGL/mrvGLViewportPrivate.h"
#include "mrvGL/mrvTimelineViewportPrivate.h"
#include "mrvGL/mrvGLShape.h"
#include "mrvGL/mrvGLUtil.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrViewer.h"

namespace mrv
{
    void Viewport::_drawAnaglyph(int left, int right) const noexcept
    {
        TLRENDER_P();
        MRV2_GL();

        glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);

        gl.render->drawVideo(
            {p.videoData[left]},
            timeline::getBoxes(timeline::CompareMode::A, p.timelineSizes),
            p.imageOptions, p.displayOptions);

        if (p.stereo3DOptions.eyeSeparation != 0.F)
        {
            math::Matrix4x4f mvp = gl.render->getTransform();
            mvp = mvp * math::translate(math::Vector3f(
                            p.stereo3DOptions.eyeSeparation, 0.F, 0.F));
            gl.render->setTransform(mvp);
        }

        glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
        gl.render->drawVideo(
            {p.videoData[right]},
            timeline::getBoxes(timeline::CompareMode::A, p.timelineSizes),
            p.imageOptions, p.displayOptions);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }

    void Viewport::_drawScanlines(int left, int right) const noexcept
    {
        TLRENDER_P();
        MRV2_GL();

        glClear(GL_STENCIL_BUFFER_BIT);
        glDisable(GL_STENCIL_TEST);

        gl.render->drawVideo(
            {p.videoData[left]},
            timeline::getBoxes(timeline::CompareMode::A, p.timelineSizes),
            p.imageOptions, p.displayOptions);

        glEnable(GL_STENCIL_TEST);

        // Set 1 into the stencil buffer
        glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        const auto& renderSize = getRenderSize();
        const size_t W = renderSize.w;
        const size_t H = renderSize.h;
        image::Color4f color(1, 1, 1, 1);
        for (size_t y = 0; y < H; y += 2)
        {
            gl.lines->drawLine(
                gl.render, math::Vector2i(0, y), math::Vector2i(W, y), color,
                1);
        }

        if (p.stereo3DOptions.eyeSeparation != 0.F)
        {
            math::Matrix4x4f mvp = gl.render->getTransform();
            mvp = mvp * math::translate(math::Vector3f(
                            p.stereo3DOptions.eyeSeparation, 0.F, 0.F));
            gl.render->setTransform(mvp);
        }

        // Only write to the Stencil Buffer where 1 is not set
        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
        // Keep the content of the Stencil Buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        gl.render->drawVideo(
            {p.videoData[right]},
            timeline::getBoxes(timeline::CompareMode::A, p.timelineSizes),
            p.imageOptions, p.displayOptions);

        glDisable(GL_STENCIL_TEST);
    }

    void Viewport::_drawCheckerboard(int left, int right) const noexcept
    {
        TLRENDER_P();
        MRV2_GL();

        glClear(GL_STENCIL_BUFFER_BIT);
        glDisable(GL_STENCIL_TEST);

        gl.render->drawVideo(
            {p.videoData[left]},
            timeline::getBoxes(timeline::CompareMode::A, p.timelineSizes),
            p.imageOptions, p.displayOptions);

        glEnable(GL_STENCIL_TEST);

        // Set 1 into the stencil buffer
        glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        const auto& renderSize = getRenderSize();
        const size_t W = renderSize.w;
        const size_t H = renderSize.h;
        image::Color4f color(1, 1, 1, 1);
        std::vector< math::Vector2f > pnts;
        for (size_t y = 0; y < H; ++y)
        {
            for (size_t x = 0; x < W; ++x)
            {
                bool t = ((x + y) % 2) < 1;
                if (t)
                {
                    pnts.push_back(math::Vector2f(x, y));
                }
            }
        }

        gl.lines->drawPoints(pnts, color, 5);

        if (p.stereo3DOptions.eyeSeparation != 0.F)
        {
            math::Matrix4x4f mvp = gl.render->getTransform();
            mvp = mvp * math::translate(math::Vector3f(
                            p.stereo3DOptions.eyeSeparation, 0.F, 0.F));
            gl.render->setTransform(mvp);
        }

        // Only write to the Stencil Buffer where 1 is not set
        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
        // Keep the content of the Stencil Buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        gl.render->drawVideo(
            {p.videoData[right]},
            timeline::getBoxes(timeline::CompareMode::A, p.timelineSizes),
            p.imageOptions, p.displayOptions);

        glDisable(GL_STENCIL_TEST);
    }

    void Viewport::_drawColumns(int left, int right) const noexcept
    {
        TLRENDER_P();
        MRV2_GL();

        glClear(GL_STENCIL_BUFFER_BIT);
        glDisable(GL_STENCIL_TEST);

        gl.render->drawVideo(
            {p.videoData[left]},
            timeline::getBoxes(timeline::CompareMode::A, p.timelineSizes),
            p.imageOptions, p.displayOptions);

        glEnable(GL_STENCIL_TEST);

        // Set 1 into the stencil buffer
        glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        const auto& renderSize = getRenderSize();
        const size_t W = renderSize.w;
        const size_t H = renderSize.h;
        image::Color4f color(1, 1, 1, 1);
        for (size_t x = 0; x < W; x += 2)
        {
            gl.lines->drawLine(
                gl.render, math::Vector2i(x, 0), math::Vector2i(x, H), color,
                1);
        }

        if (p.stereo3DOptions.eyeSeparation != 0.F)
        {
            math::Matrix4x4f mvp = gl.render->getTransform();
            mvp = mvp * math::translate(math::Vector3f(
                            p.stereo3DOptions.eyeSeparation, 0.F, 0.F));
            gl.render->setTransform(mvp);
        }

        // Only write to the Stencil Buffer where 1 is not set
        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
        // Keep the content of the Stencil Buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        gl.render->drawVideo(
            {p.videoData[right]},
            timeline::getBoxes(timeline::CompareMode::A, p.timelineSizes),
            p.imageOptions, p.displayOptions);

        glDisable(GL_STENCIL_TEST);
    }

    void Viewport::_drawStereoOpenGL() noexcept
    {
        TLRENDER_P();
        MRV2_GL();

        const bool swap = p.stereo3DOptions.swapEyes;
        int left = 0, right = p.videoData.size() - 1;
        if (swap)
        {
            left = right;
            right = 0;
        }

        const auto& renderSize = getRenderSize();

        {
            StoreLocale;

            gl::OffscreenBufferBinding binding(gl.buffer);
            gl.render->begin(renderSize, p.colorConfigOptions, p.lutOptions);
            if (p.missingFrame &&
                p.missingFrameType != MissingFrameType::kBlackFrame)
            {
                _drawMissingFrame(renderSize);
            }
            else
            {
                gl.render->drawVideo(
                    {p.videoData[left]},
                    timeline::getBoxes(
                        timeline::CompareMode::A, p.timelineSizes),
                    p.imageOptions, p.displayOptions);
            }

            _drawOverlays(renderSize);

            gl.render->end();
        }

        {
            StoreLocale;
            gl::OffscreenBufferBinding binding(gl.stereoBuffer);

            gl.render->begin(renderSize, p.colorConfigOptions, p.lutOptions);

            if (p.stereo3DOptions.eyeSeparation != 0.F)
            {
                math::Matrix4x4f mvp = gl.render->getTransform();
                mvp = mvp * math::translate(math::Vector3f(
                                p.stereo3DOptions.eyeSeparation, 0.F, 0.F));
                gl.render->setTransform(mvp);
            }

            gl.render->drawVideo(
                {p.videoData[right]},
                timeline::getBoxes(timeline::CompareMode::A, p.timelineSizes),
                p.imageOptions, p.displayOptions);

            _drawOverlays(renderSize);

            gl.render->end();
        }
    }

    void Viewport::_drawStereo3D() const noexcept
    {
        TLRENDER_P();
        const bool swap = p.stereo3DOptions.swapEyes;
        int left = 0, right = p.videoData.size() - 1;
        if (swap)
        {
            left = right;
            right = 0;
        }
        assert(left != right);

        switch (p.stereo3DOptions.output)
        {
            break;
        case Stereo3DOutput::Scanlines:
            _drawScanlines(left, right);
            break;
        case Stereo3DOutput::Columns:
            _drawColumns(left, right);
            break;
        case Stereo3DOutput::Checkerboard:
            _drawCheckerboard(left, right);
            break;
        default:
        case Stereo3DOutput::Anaglyph:
            _drawAnaglyph(left, right);
        }
    }

    void
    Viewport::_drawMissingFrame(const math::Size2i& renderSize) const noexcept
    {
        TLRENDER_P();
        MRV2_GL();

        gl.render->drawVideo(
            {p.lastVideoData},
            timeline::getBoxes(p.compareOptions.mode, p.timelineSizes),
            p.imageOptions, p.displayOptions, p.compareOptions);

        if (p.missingFrameType == MissingFrameType::kScratchedFrame)
        {
            image::Color4f color(1, 0, 0, 0.8);
            gl.lines->drawLine(
                gl.render, math::Vector2i(0, 0),
                math::Vector2i(renderSize.w, renderSize.h), color, 4);
            gl.lines->drawLine(
                gl.render, math::Vector2i(0, renderSize.h),
                math::Vector2i(renderSize.w, 0), color, 4);
        }
    }

    void Viewport::_drawCursor(const math::Matrix4x4f& mvp) const noexcept
    {
        MRV2_GL();
        TLRENDER_P();
        if (p.actionMode != ActionMode::kScrub &&
            p.actionMode != ActionMode::kText &&
            p.actionMode != ActionMode::kSelection &&
            p.actionMode != ActionMode::kRotate && Fl::belowmouse() == this)
        {
            const image::Color4f color(1.F, 1.F, 1.F, 1.0F);
            const float multiplier =
                1.0F + (2.5F * (p.actionMode == ActionMode::kErase));
            const float pen_size = _getPenSize() * multiplier;
            p.mousePos = _getFocus();
            const auto& pos = _getRaster();
            gl.render->setTransform(mvp);
            gl.lines->drawCursor(gl.render, pos, pen_size / 2.0, color);
        }
    }

    void Viewport::_drawRectangleOutline(
        const math::Box2i& box, const image::Color4f& color,
        const math::Matrix4x4f& mvp) const noexcept
    {
        MRV2_GL();
#ifdef USE_ONE_PIXEL_LINES
        gl.outline->drawRect(box, color, mvp);
        CHECK_GL;
#else
        int width = 2 / _p->viewZoom; //* renderSize.w / viewportSize.w;
        if (width < 2)
            width = 2;
        gl.render->setTransform(mvp);
        CHECK_GL;
        drawRectOutline(gl.render, box, color, width);
        CHECK_GL;
#endif
    }

#ifdef USE_OPENGL2

    void Viewport::_drawGL2TextShapes()
    {
        TLRENDER_P();
        MRV2_GL();

        const auto& player = getTimelinePlayer();
        if (!player)
            return;

        const otime::RationalTime& time = p.videoData[0].time;

        const auto& annotations =
            player->getAnnotations(p.ghostPrevious, p.ghostNext);
        if (annotations.empty())
            return;

        float pixel_unit = pixels_per_unit();
        const auto& viewportSize = getViewportSize();
        const auto& renderSize = getRenderSize();

        for (const auto& annotation : annotations)
        {
            const auto& annotationTime = annotation->time;
            float alphamult = 0.F;
            if (annotation->allFrames ||
                time::floor(time) == time::floor(annotationTime))
                alphamult = 1.F;
            else
            {
                if (p.ghostPrevious)
                {
                    for (short i = p.ghostPrevious - 1; i > 0; --i)
                    {
                        otime::RationalTime offset(i, time.rate());
                        if (time::floor(time - offset) ==
                            time::floor(annotationTime))
                        {
                            alphamult = 1.F - (float)i / p.ghostPrevious;
                            break;
                        }
                    }
                }
                if (p.ghostNext)
                {
                    for (short i = 1; i < p.ghostNext; ++i)
                    {
                        otime::RationalTime offset(i, time.rate());
                        if (time::floor(time + offset) ==
                            time::floor(annotationTime))
                        {
                            alphamult = 1.F - (float)i / p.ghostNext;
                            break;
                        }
                    }
                }
            }

            if (alphamult == 0.F)
                continue;

            const auto& shapes = annotation->shapes;
            math::Vector2i pos;

            pos.x = p.viewPos.x;
            pos.y = p.viewPos.y;
            pos.x /= pixel_unit;
            pos.y /= pixel_unit;
            math::Matrix4x4f vm;
            vm = vm * math::translate(math::Vector3f(pos.x, pos.y, 0.F));
            vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));

            // No projection matrix.  Thar's set by FLTK ( and we
            // reset it -- flip it in Y -- inside mrvGL2TextShape.cpp ).

            for (auto& shape : shapes)
            {
                auto textShape = dynamic_cast< GL2TextShape* >(shape.get());
                if (!textShape)
                    continue;

                float a = shape->color.a;
                shape->color.a *= alphamult;
                textShape->pixels_per_unit = pixel_unit;
                textShape->w = w();
                textShape->h = h();
                textShape->viewZoom = p.viewZoom;
                shape->matrix = vm;
                CHECK_GL;
                shape->draw(gl.render, gl.lines);
                CHECK_GL;
                shape->color.a = a;
            }
        }
    }
#endif

    void Viewport::_drawShape(
        const std::shared_ptr< draw::Shape >& shape,
        const float alphamult) noexcept
    {
        TLRENDER_P();
        MRV2_GL();

#ifdef USE_OPENGL2
        auto gl2Shape = dynamic_cast< GL2TextShape* >(shape.get());
        if (gl2Shape)
            return;
#else
        auto textShape = dynamic_cast< GLTextShape* >(shape.get());
        if (textShape && !textShape->text.empty())
        {
            const auto& viewportSize = getViewportSize();
            math::Matrix4x4f vm;
            vm = vm *
                 math::translate(math::Vector3f(p.viewPos.x, p.viewPos.y, 0.F));
            vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));
            auto pm = math::ortho(
                0.F, static_cast<float>(viewportSize.w), 0.F,
                static_cast<float>(viewportSize.h), -1.F, 1.F);
            auto mvp = pm * vm;
            mvp = mvp * math::scale(math::Vector3f(1.F, -1.F, 1.F));
            shape->matrix = mvp;
        }
#endif
        auto note = dynamic_cast< draw::NoteShape* >(shape.get());
        if (note)
        {
            if (annotationsPanel && alphamult == 1.F)
                annotationsPanel->notes->value(note->text.c_str());
        }
        else
        {
            float alpha = shape->color.a;
            shape->color.a *= alphamult;
            shape->color.a *= shape->fade;
            shape->draw(gl.render, gl.lines);
            shape->color.a = alpha;
        }
    }

    void Viewport::_drawAnnotations(const math::Matrix4x4f& mvp)
    {
        TLRENDER_P();
        MRV2_GL();

        const auto& player = getTimelinePlayer();
        if (!player)
            return;

        const otime::RationalTime& time = p.videoData[0].time;

        if (annotationsPanel)
        {
            annotationsPanel->notes->value("");
        }

        const auto& annotations =
            player->getAnnotations(p.ghostPrevious, p.ghostNext);
        if (annotations.empty())
            return;

        const auto& renderSize = getRenderSize();
        const auto& viewportSize = getViewportSize();
        const math::Matrix4x4f m = math::ortho(
            0.F, static_cast<float>(renderSize.w), 0.F,
            static_cast<float>(renderSize.h), -1.F, 1.F);

        for (const auto& annotation : annotations)
        {
            const auto& annotationTime = annotation->time;
            float alphamult = 0.F;
            if (annotation->allFrames ||
                time::floor(time) == time::floor(annotationTime))
                alphamult = 1.F;
            else
            {
                if (p.ghostPrevious)
                {
                    for (short i = p.ghostPrevious - 1; i > 0; --i)
                    {
                        otime::RationalTime offset(i, time.rate());
                        if (time::floor(time - offset) ==
                            time::floor(annotationTime))
                        {
                            alphamult = 1.F - (float)i / p.ghostPrevious;
                            break;
                        }
                    }
                }
                if (p.ghostNext)
                {
                    for (short i = 1; i < p.ghostNext; ++i)
                    {
                        otime::RationalTime offset(i, time.rate());
                        if (time::floor(time + offset) ==
                            time::floor(annotationTime))
                        {
                            alphamult = 1.F - (float)i / p.ghostNext;
                            break;
                        }
                    }
                }
            }

            if (alphamult == 0.F)
                continue;

            {
                gl::OffscreenBufferBinding binding(gl.annotation);
                gl.render->begin(
                    viewportSize, timeline::ColorConfigOptions(),
                    timeline::LUTOptions());
                gl.render->setTransform(mvp);
                const auto& shapes = annotation->shapes;
                for (const auto& shape : shapes)
                {
                    _drawShape(shape, alphamult);
                }
                gl.render->end();
            }

            gl::SetAndRestore(GL_BLEND, GL_TRUE);

            glBlendFuncSeparate(
                GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA,
                GL_ONE_MINUS_SRC_ALPHA);

            glViewport(0, 0, GLsizei(viewportSize.w), GLsizei(viewportSize.h));

            gl.annotationShader->bind();
            gl.annotationShader->setUniform("transform.mvp", m);
            timeline::Channels channels = timeline::Channels::Color;
            if (!p.displayOptions.empty())
                channels = p.displayOptions[0].channels;
            gl.annotationShader->setUniform(
                "channels", static_cast<int>(channels));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gl.annotation->getColorID());

            if (gl.vao && gl.vbo)
            {
                gl.vao->bind();
                gl.vao->draw(GL_TRIANGLES, 0, gl.vbo->getSize());
            }
        }
    }

    void Viewport::_drawCropMask(const math::Size2i& renderSize) const noexcept
    {
        MRV2_GL();

        double aspectY = (double)renderSize.w / (double)renderSize.h;
        double aspectX = (double)renderSize.h / (double)renderSize.w;

        double target_aspect = 1.0 / _p->masking;
        double amountY = (0.5 - target_aspect * aspectY / 2);
        double amountX = (0.5 - _p->masking * aspectX / 2);

        bool vertical = true;
        if (amountY < amountX)
        {
            vertical = false;
        }

        image::Color4f maskColor(0, 0, 0, 1);

        if (vertical)
        {
            int Y = renderSize.h * amountY;
            math::Box2i box(0, 0, renderSize.w, Y);
            gl.render->drawRect(box, maskColor);
            box.max.y = renderSize.h;
            box.min.y = renderSize.h - Y;
            gl.render->drawRect(box, maskColor);
        }
        else
        {
            int X = renderSize.w * amountX;
            math::Box2i box(0, 0, X, renderSize.h);
            gl.render->drawRect(box, maskColor);
            box.max.x = renderSize.w;
            box.min.x = renderSize.w - X;
            gl.render->drawRect(box, maskColor);
        }
    }

    inline void Viewport::_drawText(
        const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
        math::Vector2i& pos, const int16_t lineHeight,
        const image::Color4f& labelColor) const noexcept
    {
        MRV2_GL();
        const image::Color4f shadowColor(0.F, 0.F, 0.F, 0.7F);
        math::Vector2i shadowPos{pos.x + 2, pos.y + 2};
        gl.render->drawText(glyphs, shadowPos, shadowColor);
        gl.render->drawText(glyphs, pos, labelColor);
        pos.y += lineHeight;
    }

    void Viewport::_drawSafeAreas(
        const float percentX, const float percentY,
        const float pixelAspectRatio, const image::Color4f& color,
        const math::Matrix4x4f& mvp, const char* label) const noexcept
    {
        MRV2_GL();
        const auto& renderSize = getRenderSize();
        const auto& viewportSize = getViewportSize();
        double aspectX = (double)renderSize.h / (double)renderSize.w;
        double aspectY = (double)renderSize.w / (double)renderSize.h;

        double amountY = (0.5 - percentY * aspectY / 2);
        double amountX = (0.5 - percentX * aspectX / 2);

        bool vertical = true;
        if (amountY < amountX)
        {
            vertical = false;
        }

        math::Box2i box;
        int X, Y;
        if (vertical)
        {
            X = renderSize.w * percentX;
            Y = renderSize.h * amountY;
        }
        else
        {
            X = renderSize.w * amountX / pixelAspectRatio;
            Y = renderSize.h * percentY;
        }
        box.min.x = renderSize.w - X;
        box.min.y = -(renderSize.h - Y);
        box.max.x = X;
        box.max.y = -Y;
#if 0
        // @bug:
        //
        // Using USE_ONE_PIXEL_LINES would make the primary display flicker
        // after the secondary one was closed.
        _drawRectangleOutline( box, color, mvp );
#else
        int width = 2 / _p->viewZoom; //* renderSize.w / viewportSize.w;
        if (width < 2)
            width = 2;
        gl.render->setTransform(mvp);
        drawRectOutline(gl.render, box, color, width);
#endif

        //
        // Draw the text too
        //
        static const std::string fontFamily = "NotoSans-Regular";
        Viewport* self = const_cast< Viewport* >(this);
        const image::FontInfo fontInfo(fontFamily, 12 * width);
        const auto glyphs = _p->fontSystem->getGlyphs(label, fontInfo);
        math::Vector2i pos(box.max.x, box.max.y - 2 * width);
        // Set the projection matrix
        gl.render->setTransform(mvp);
        gl.render->drawText(glyphs, pos, color);
    }

    void Viewport::_drawSafeAreas() const noexcept
    {
        TLRENDER_P();
        if (p.timelinePlayers.empty())
            return;
        const auto& player = p.timelinePlayers[0];
        const auto& info = player->player()->getIOInfo();
        const auto& video = info.video[0];
        const auto pr = video.size.pixelAspectRatio;

        const auto& viewportSize = getViewportSize();
        const auto& renderSize = getRenderSize();

        math::Matrix4x4f vm;
        vm =
            vm * math::translate(math::Vector3f(p.viewPos.x, p.viewPos.y, 0.F));
        vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));
        const auto pm = math::ortho(
            0.F, static_cast<float>(viewportSize.w), 0.F,
            static_cast<float>(viewportSize.h), -1.F, 1.F);
        auto mvp = pm * vm;
        mvp = mvp * math::scale(math::Vector3f(1.F, -1.F, 1.F));

        double aspect = (double)renderSize.w / pr / (double)renderSize.h;
        if (aspect <= 1.78)
        {
            // For HDTV, NTSC or PAL, we just use the action/title areas
            image::Color4f color(1.F, 0.F, 0.F);
            _drawSafeAreas(aspect * 0.9, 0.9F, pr, color, mvp, "tv action");
            _drawSafeAreas(aspect * 0.8F, 0.8F, pr, color, mvp, "tv title");
        }
        else
        {
            // For film, we use the different film ratios
            image::Color4f color(1.F, 0.F, 0.F);
            // Assume film, draw 2.35, 1.85, 1.66 and hdtv areas
            _drawSafeAreas(2.35, 1.F, pr, color, mvp, _("2.35"));

            // Draw HDTV safe aeas
            _drawSafeAreas(1.77 * 0.9, 0.9F, pr, color, mvp, "tv action");
            _drawSafeAreas(1.77 * 0.8F, 0.8F, pr, color, mvp, "tv title");

            color = image::Color4f(1.F, 1.0f, 0.F);
            _drawSafeAreas(1.89, 1.F, pr, color, mvp, _("1.85"));
            color = image::Color4f(0.F, 1.0f, 1.F);
            _drawSafeAreas(1.66, 1.F, pr, color, mvp, _("1.66"));
            // Draw hdtv too
            color = image::Color4f(1.F, 0.0f, 1.F);
            _drawSafeAreas(1.77, 1.0, pr, color, mvp, "hdtv");
        }
    }

    void Viewport::_drawHUD() const noexcept
    {
        TLRENDER_P();
        MRV2_GL();

        if (!p.fontSystem)
            return;

        const auto& viewportSize = getViewportSize();

        static const std::string fontFamily = "NotoSans-Regular";
        Viewport* self = const_cast< Viewport* >(this);
        uint16_t fontSize = 12 * self->pixels_per_unit();

        Fl_Color c = p.ui->uiPrefs->uiPrefsViewHud->color();
        uint8_t r, g, b;
        Fl::get_color(c, r, g, b);

        const image::Color4f labelColor(r / 255.F, g / 255.F, b / 255.F);

        const image::FontInfo fontInfo(fontFamily, fontSize);
        const image::FontMetrics fontMetrics =
            p.fontSystem->getMetrics(fontInfo);
        auto lineHeight = fontMetrics.lineHeight;
        math::Vector2i pos(20, lineHeight * 2);

        if (p.timelinePlayers.empty())
            return;
        const auto& player = p.timelinePlayers[0];

        const auto& path = player->path();
        const otime::RationalTime& time = p.videoData[0].time;
        int64_t frame = time.to_frames();

        timeline::RenderOptions renderOptions;
        renderOptions.clear = false;
        gl.render->begin(
            viewportSize, timeline::ColorConfigOptions(),
            timeline::LUTOptions(), renderOptions);

        char buf[512];
        if (p.hud & HudDisplay::kDirectory)
        {
            const auto& directory = path.getDirectory();
            _drawText(
                p.fontSystem->getGlyphs(directory, fontInfo), pos, lineHeight,
                labelColor);
        }

        if (p.hud & HudDisplay::kFilename)
        {
            const std::string& fullname =
                createStringFromPathAndTime(path, time);
            _drawText(
                p.fontSystem->getGlyphs(fullname, fontInfo), pos, lineHeight,
                labelColor);
        }

        if (p.hud & HudDisplay::kResolution)
        {
            const auto& inPlayer = player->player();
            if (inPlayer)
            {
                const auto& info = inPlayer->getIOInfo();
                const auto& video = info.video[0];
                if (video.size.pixelAspectRatio != 1.0)
                {
                    int width = video.size.w * video.size.pixelAspectRatio;
                    snprintf(
                        buf, 512, "%d x %d  ( %.3g )  %d x %d", video.size.w,
                        video.size.h, video.size.pixelAspectRatio, width,
                        video.size.h);
                }
                else
                {
                    snprintf(buf, 512, "%d x %d", video.size.w, video.size.h);
                }
                _drawText(
                    p.fontSystem->getGlyphs(buf, fontInfo), pos, lineHeight,
                    labelColor);
            }
        }

        std::string tmp;
        tmp.reserve(512);
        if (p.hud & HudDisplay::kFrame)
        {
            snprintf(buf, 512, "F: %" PRId64 " ", frame);
            tmp += buf;
        }

        if (p.hud & HudDisplay::kFrameRange)
        {
            const auto& range = player->timeRange();
            frame = range.start_time().to_frames();
            const int64_t last_frame = range.end_time_inclusive().to_frames();
            snprintf(
                buf, 512, "Range: %" PRId64 " -  %" PRId64, frame, last_frame);
            tmp += buf;
        }

        if (p.hud & HudDisplay::kTimecode)
        {
            snprintf(buf, 512, "TC: %s ", time.to_timecode(nullptr).c_str());
            tmp += buf;
        }

        if (p.hud & HudDisplay::kFPS)
        {
            if (player->playback() != timeline::Playback::Stop)
            {
                int64_t frame_diff = (time.value() - p.lastTime.value());
                int64_t absdiff = std::abs(frame_diff);
                if (absdiff > 1 && absdiff < 60)
                    p.skippedFrames += absdiff - 1;
            }
            snprintf(
                buf, 512, "SF: %" PRIu64 " FPS: %.3f", p.skippedFrames,
                player->speed());
            tmp += buf;
        }

        p.lastTime = time;

        if (!tmp.empty())
            _drawText(
                p.fontSystem->getGlyphs(tmp, fontInfo), pos, lineHeight,
                labelColor);

        tmp.clear();
        if (p.hud & HudDisplay::kFrameCount)
        {
            const otime::TimeRange& range = player->timeRange();
            const otime::RationalTime& duration =
                range.end_time_inclusive() - range.start_time();
            snprintf(buf, 512, "FC: %" PRId64, (int64_t)duration.to_frames());
            tmp += buf;
        }

        if (!tmp.empty())
            _drawText(
                p.fontSystem->getGlyphs(tmp, fontInfo), pos, lineHeight,
                labelColor);

        tmp.clear();
        if (p.hud & HudDisplay::kMemory)
        {
            uint64_t totalVirtualMem = 0;
            uint64_t virtualMemUsed = 0;
            uint64_t virtualMemUsedByMe = 0;
            uint64_t totalPhysMem = 0;
            uint64_t physMemUsed = 0;
            uint64_t physMemUsedByMe = 0;
            memory_information(
                totalVirtualMem, virtualMemUsed, virtualMemUsedByMe,
                totalPhysMem, physMemUsed, physMemUsedByMe);

            snprintf(
                buf, 512,
                _("PMem: %" PRIu64 " / %" PRIu64 " MB  VMem: %" PRIu64
                  " / %" PRIu64 " MB"),
                physMemUsedByMe, totalPhysMem, virtualMemUsedByMe,
                totalVirtualMem);
            tmp += buf;
        }

        if (!tmp.empty())
            _drawText(
                p.fontSystem->getGlyphs(tmp, fontInfo), pos, lineHeight,
                labelColor);

        if (p.hud & HudDisplay::kCache)
        {
            const auto& cacheInfo = player->cacheInfo();

            uint64_t aheadVideoFrames = 0, behindVideoFrames = 0;
            uint64_t aheadAudioFrames = 0, behindAudioFrames = 0;

            otime::TimeRange currentRange(
                otime::RationalTime(
                    static_cast<double>(frame), player->defaultSpeed()),
                otime::RationalTime(1.0, player->defaultSpeed()));

            for (const auto& i : cacheInfo.videoFrames)
            {
                if (i.intersects(currentRange))
                {
                    aheadVideoFrames +=
                        i.end_time_inclusive().to_frames() - frame;
                    behindVideoFrames += frame - i.start_time().to_frames();
                }
            }

            for (const auto& i : cacheInfo.audioFrames)
            {
                if (i.intersects(currentRange))
                {
                    aheadAudioFrames +=
                        i.end_time_inclusive().to_frames() - frame;
                    behindAudioFrames += frame - i.start_time().to_frames();
                }
            }
            snprintf(
                buf, 512, _("Ahead    V: % 4" PRIu64 "    A: % 4" PRIu64),
                aheadVideoFrames, aheadAudioFrames);
            _drawText(
                p.fontSystem->getGlyphs(buf, fontInfo), pos, lineHeight,
                labelColor);
            snprintf(
                buf, 512, _("Behind   V: % 4" PRIu64 "    A: % 4" PRIu64),
                behindVideoFrames, behindAudioFrames);
            _drawText(
                p.fontSystem->getGlyphs(buf, fontInfo), pos, lineHeight,
                labelColor);
        }

        if (p.hud & HudDisplay::kAttributes)
        {
            image::Tags tags;
            if (!p.videoData.empty() && !p.videoData[0].layers.empty())
            {
                if (p.videoData[0].layers[0].image)
                {
                    tags = p.videoData[0].layers[0].image->getTags();
                    for (const auto& tag : tags)
                    {
                        if (pos.y > viewportSize.h)
                            return;
                        snprintf(
                            buf, 512, "%s = %s", tag.first.c_str(),
                            tag.second.c_str());
                        _drawText(
                            p.fontSystem->getGlyphs(buf, fontInfo), pos,
                            lineHeight, labelColor);
                    }
                }
            }
            const auto& inPlayer = player->player();
            if (!inPlayer)
                return;
            const auto& info = inPlayer->getIOInfo();
            for (const auto& tag : info.tags)
            {
                if (pos.y > viewportSize.h)
                    return;
                snprintf(
                    buf, 512, "%s = %s", tag.first.c_str(), tag.second.c_str());
                _drawText(
                    p.fontSystem->getGlyphs(buf, fontInfo), pos, lineHeight,
                    labelColor);
            }
        }
    }

    void Viewport::_drawWindowArea(const std::string& dw) const noexcept
    {
        TLRENDER_P();
        MRV2_GL();
        const auto& renderSize = getRenderSize();
        const auto& viewportSize = getViewportSize();
        const image::Color4f color(0.5, 0.5, 0.5, 1.0);

        math::Box2i box;
        std::stringstream ss(dw);
        ss >> box.min.x >> box.min.y >> box.max.x >> box.max.y;

        box.min.y = -(renderSize.h - box.min.y);
        box.max.y = -(renderSize.h - box.max.y);

        math::Matrix4x4f vm;
        vm =
            vm * math::translate(math::Vector3f(p.viewPos.x, p.viewPos.y, 0.F));
        vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));
        const auto pm = math::ortho(
            0.F, static_cast<float>(viewportSize.w), 0.F,
            static_cast<float>(viewportSize.h), -1.F, 1.F);
        auto mvp = pm * vm;
        mvp = mvp * math::scale(math::Vector3f(1.F, -1.F, 1.F));
        gl.render->setTransform(mvp);
        drawRectOutline(gl.render, box, color, 2);
    }

    void Viewport::_drawDataWindow() const noexcept
    {
        TLRENDER_P();
        if (p.videoData.empty() || p.videoData[0].layers.empty())
            return;

        const auto& tags = p.videoData[0].layers[0].image->getTags();
        image::Tags::const_iterator i = tags.find("Data Window");
        if (i == tags.end())
            return;

        const std::string& dw = i->second;
        _drawWindowArea(dw);
    }

    void Viewport::_drawDisplayWindow() const noexcept
    {
        TLRENDER_P();
        if (p.videoData.empty() || p.videoData[0].layers.empty())
            return;

        const auto& tags = p.videoData[0].layers[0].image->getTags();
        image::Tags::const_iterator i = tags.find("Display Window");
        if (i == tags.end())
            return;

        const std::string& dw = i->second;
        _drawWindowArea(dw);
    }

    void Viewport::_drawOverlays(const math::Size2i& renderSize) const noexcept
    {
        TLRENDER_P();
        if (p.masking > 0.0001F)
            _drawCropMask(renderSize);
    }

    void Viewport::_drawHelpText() const noexcept
    {
        TLRENDER_P();
        if (p.timelinePlayers.empty())
            return;
        if (!p.fontSystem)
            return;

        MRV2_GL();

        static const std::string fontFamily = "NotoSans-Regular";
        Viewport* self = const_cast< Viewport* >(this);
        uint16_t fontSize = 16 * self->pixels_per_unit();

        const image::Color4f labelColor(255.F, 255.F, 255.F, p.helpTextFade);

        char buf[512];
        const image::FontInfo fontInfo(fontFamily, fontSize);
        const image::FontMetrics fontMetrics =
            p.fontSystem->getMetrics(fontInfo);
        const int labelSpacing = fontInfo.size / 2;
        auto lineHeight = fontMetrics.lineHeight;
        const math::Size2i labelSize =
            p.fontSystem->getSize(p.helpText, fontInfo);

        const auto& viewportSize = getViewportSize();

        timeline::RenderOptions renderOptions;
        renderOptions.clear = false;

        const math::Box2i labelBox(0, 20, viewportSize.w - 20, viewportSize.h);
        math::Box2i box = math::Box2i(
            labelBox.max.x + 1 - labelSpacing * 2 - labelSize.w, labelBox.min.y,
            labelSize.w + labelSpacing * 2, fontMetrics.lineHeight);
        auto pos = math::Vector2i(
            labelBox.max.x + 1 - labelSpacing - labelSize.w,
            labelBox.min.y + fontMetrics.ascender);

        gl.render->begin(
            viewportSize, timeline::ColorConfigOptions(),
            timeline::LUTOptions(), renderOptions);

        gl.render->drawRect(
            box, image::Color4f(0.F, 0.F, 0.F, 0.7F * p.helpTextFade));

        _drawText(
            p.fontSystem->getGlyphs(p.helpText, fontInfo), pos, lineHeight,
            labelColor);

        gl.render->end();
    }

    void Viewport::_drawBackground() const noexcept
    {
        MRV2_GL();
        TLRENDER_P();
        if (p.timelinePlayers.empty())
            return;
        const auto& viewportSize = getViewportSize();

        timeline::RenderOptions renderOptions;
        renderOptions.clear = false;

        gl.render->begin(
            viewportSize, timeline::ColorConfigOptions(),
            timeline::LUTOptions(), renderOptions);

        switch (p.backgroundOptions.type)
        {
        case timeline::Background::Solid:
            gl.render->clearViewport(p.backgroundOptions.solidColor);
            break;
        case timeline::Background::Checkers:
            gl.render->clearViewport(image::Color4f(0.F, 0.F, 0.F));
            gl.render->drawColorMesh(
                ui::checkers(
                    math::Box2i(0, 0, viewportSize.w, viewportSize.h),
                    p.backgroundOptions.checkersColor0,
                    p.backgroundOptions.checkersColor1,
                    p.backgroundOptions.checkersSize),
                math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));
            break;
        default:
            break;
        }

        gl.render->end();
    }
} // namespace mrv

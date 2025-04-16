// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlIO/System.h>

#include <tlDevice/IOutput.h>

#include <tlCore/String.h>
#include <tlCore/Mesh.h>
#include <tlVk/Util.h>

#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvMemory.h"
#include "mrvCore/mrvUtil.h"

#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvVk/mrvVkDefines.h"
#include "mrvVk/mrvVkViewport.h"
#include "mrvVk/mrvVkViewportPrivate.h"
#include "mrvVk/mrvTimelineViewportPrivate.h"
#include "mrvVk/mrvVkShape.h"
#include "mrvVk/mrvVkUtil.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrViewer.h"

namespace
{
    const unsigned kFPSAverageFrames = 10;
}

namespace mrv
{
    namespace vulkan
    {
    
        void Viewport::_drawAnaglyph(int left, int right) const noexcept
        {
            TLRENDER_P();
            MRV2_VK();

            // glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);

            vk.render->drawVideo(
                {p.videoData[left]},
                timeline::getBoxes(timeline::CompareMode::A, {p.videoData[left]}),
                p.imageOptions, p.displayOptions);

            if (p.stereo3DOptions.eyeSeparation != 0.F)
            {
                math::Matrix4x4f mvp = vk.render->getTransform();
                mvp = mvp * math::translate(math::Vector3f(
                                                p.stereo3DOptions.eyeSeparation, 0.F, 0.F));
                vk.render->setTransform(mvp);
            }

            // glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
            vk.render->drawVideo(
                {p.videoData[right]},
                timeline::getBoxes(timeline::CompareMode::A, {p.videoData[right]}),
                p.imageOptions, p.displayOptions);

            // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }

        void Viewport::_drawScanlines(int left, int right) const noexcept
        {
            TLRENDER_P();
            MRV2_VK();

            // glClear(GL_STENCIL_BUFFER_BIT);
            // glDisable(GL_STENCIL_TEST);

            vk.render->drawVideo(
                {p.videoData[left]},
                timeline::getBoxes(timeline::CompareMode::A, {p.videoData[left]}),
                p.imageOptions, p.displayOptions);

            // glEnable(GL_STENCIL_TEST);

            // // Set 1 into the stencil buffer
            // glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
            // glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
            // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            const auto& renderSize = getRenderSize();
            const size_t W = renderSize.w;
            const size_t H = renderSize.h;
            image::Color4f color(1, 1, 1, 1);
            for (size_t y = 0; y < H; y += 2)
            {
                vk.lines->drawLine(
                    vk.render, math::Vector2i(0, y), math::Vector2i(W, y), color,
                    1);
            }

            if (p.stereo3DOptions.eyeSeparation != 0.F)
            {
                math::Matrix4x4f mvp = vk.render->getTransform();
                mvp = mvp * math::translate(math::Vector3f(
                                                p.stereo3DOptions.eyeSeparation, 0.F, 0.F));
                vk.render->setTransform(mvp);
            }

            // // Only write to the Stencil Buffer where 1 is not set
            // glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
            // // Keep the content of the Stencil Buffer
            // glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            vk.render->drawVideo(
                {p.videoData[right]},
                timeline::getBoxes(timeline::CompareMode::A, {p.videoData[right]}),
                p.imageOptions, p.displayOptions);

            // glDisable(GL_STENCIL_TEST)
                ;
        }

        void Viewport::_drawCheckerboard(int left, int right) const noexcept
        {
            TLRENDER_P();
            MRV2_VK();

            // glClear(GL_STENCIL_BUFFER_BIT);
            // glDisable(GL_STENCIL_TEST);

            vk.render->drawVideo(
                {p.videoData[left]},
                timeline::getBoxes(timeline::CompareMode::A, {p.videoData[left]}),
                p.imageOptions, p.displayOptions);

            // glEnable(GL_STENCIL_TEST);

            // // Set 1 into the stencil buffer
            // glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
            // glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
            // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

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

            vk.lines->drawPoints(pnts, color, 5);

            if (p.stereo3DOptions.eyeSeparation != 0.F)
            {
                math::Matrix4x4f mvp = vk.render->getTransform();
                mvp = mvp * math::translate(math::Vector3f(
                                                p.stereo3DOptions.eyeSeparation, 0.F, 0.F));
                vk.render->setTransform(mvp);
            }

            // // Only write to the Stencil Buffer where 1 is not set
            // glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
            // // Keep the content of the Stencil Buffer
            // glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            vk.render->drawVideo(
                {p.videoData[right]},
                timeline::getBoxes(timeline::CompareMode::A, {p.videoData[right]}),
                p.imageOptions, p.displayOptions);

            // glDisable(GL_STENCIL_TEST);
        }

        void Viewport::_drawColumns(int left, int right) const noexcept
        {
            TLRENDER_P();
            MRV2_VK();

            // glClear(GL_STENCIL_BUFFER_BIT);
            // glDisable(GL_STENCIL_TEST);

            vk.render->drawVideo(
                {p.videoData[left]},
                timeline::getBoxes(timeline::CompareMode::A, {p.videoData[left]}),
                p.imageOptions, p.displayOptions);

            // glEnable(GL_STENCIL_TEST);

            // // Set 1 into the stencil buffer
            // glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
            // glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
            // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            const auto& renderSize = getRenderSize();
            const size_t W = renderSize.w;
            const size_t H = renderSize.h;
            image::Color4f color(1, 1, 1, 1);
            for (size_t x = 0; x < W; x += 2)
            {
                vk.lines->drawLine(
                    vk.render, math::Vector2i(x, 0), math::Vector2i(x, H), color,
                    1);
            }

            if (p.stereo3DOptions.eyeSeparation != 0.F)
            {
                math::Matrix4x4f mvp = vk.render->getTransform();
                mvp = mvp * math::translate(math::Vector3f(
                                                p.stereo3DOptions.eyeSeparation, 0.F, 0.F));
                vk.render->setTransform(mvp);
            }

            // // Only write to the Stencil Buffer where 1 is not set
            // glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
            // // Keep the content of the Stencil Buffer
            // glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            vk.render->drawVideo(
                {p.videoData[right]},
                timeline::getBoxes(timeline::CompareMode::A, {p.videoData[right]}),
                p.imageOptions, p.displayOptions);

            // glDisable(GL_STENCIL_TEST);
        }

        void Viewport::_drawStereoVulkan() noexcept
        {
            TLRENDER_P();
            MRV2_VK();

            const bool swap = p.stereo3DOptions.swapEyes;
            int left = 0, right = p.videoData.size() - 1;
            if (swap)
            {
                left = right;
                right = 0;
            }

            const auto& renderSize = getRenderSize();

            {
                locale::SetAndRestore saved;

                vk::OffscreenBufferBinding binding(vk.buffer);
                vk.render->begin(renderSize);
                vk.render->setOCIOOptions(p.ocioOptions);
                vk.render->setLUTOptions(p.lutOptions);
                if (p.missingFrame &&
                    p.missingFrameType != MissingFrameType::kBlackFrame)
                {
                    _drawMissingFrame(renderSize);
                }
                else
                {
                    vk.render->drawVideo(
                        {p.videoData[left]},
                        timeline::getBoxes(
                            timeline::CompareMode::A, {p.videoData[left]}),
                        p.imageOptions, p.displayOptions, p.compareOptions,
                        getBackgroundOptions());
                }

                _drawOverlays(renderSize);

                vk.render->end();
            }

            {
                locale::SetAndRestore saved;
                vk::OffscreenBufferBinding binding(vk.stereoBuffer);

                vk.render->begin(renderSize);
                vk.render->setOCIOOptions(p.ocioOptions);
                vk.render->setLUTOptions(p.lutOptions);

                if (p.stereo3DOptions.eyeSeparation != 0.F)
                {
                    math::Matrix4x4f mvp = vk.render->getTransform();
                    mvp = mvp * math::translate(math::Vector3f(
                                                    p.stereo3DOptions.eyeSeparation, 0.F, 0.F));
                    vk.render->setTransform(mvp);
                }

                vk.render->drawVideo(
                    {p.videoData[right]},
                    timeline::getBoxes(
                        timeline::CompareMode::A, {p.videoData[right]}),
                    p.imageOptions, p.displayOptions);

                _drawOverlays(renderSize);

                vk.render->end();
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
            MRV2_VK();

            vk.render->drawVideo(
                {p.lastVideoData},
                timeline::getBoxes(p.compareOptions.mode, {p.lastVideoData}),
                p.imageOptions, p.displayOptions, p.compareOptions,
                getBackgroundOptions());

            if (p.missingFrameType == MissingFrameType::kScratchedFrame)
            {
                image::Color4f color(1, 0, 0, 0.8);
                vk.lines->drawLine(
                    vk.render, math::Vector2i(0, 0),
                    math::Vector2i(renderSize.w, renderSize.h), color, 4);
                vk.lines->drawLine(
                    vk.render, math::Vector2i(0, renderSize.h),
                    math::Vector2i(renderSize.w, 0), color, 4);
            }
        }

        void Viewport::_drawCursor(const math::Matrix4x4f& mvp) const noexcept
        {
            MRV2_VK();
            TLRENDER_P();
            const image::Color4f color(1.F, 1.F, 1.F, 1.0F);
            const float multiplier =
                1.0F + (2.5F * (p.actionMode == ActionMode::kErase));
            const float pen_size = _getPenSize() * multiplier;
            p.mousePos = _getFocus();
            const auto& pos = _getRasterf();
            vk.render->setTransform(mvp);
            vk.lines->drawCursor(vk.render, pos, pen_size, color);
        }

        void Viewport::_drawRectangleOutline(
            const math::Box2i& box, const image::Color4f& color,
            const math::Matrix4x4f& mvp) const noexcept
        {
            MRV2_VK();
#ifdef USE_ONE_PIXEL_LINES
            vk.outline->drawRect(box, color, mvp);
            
#else
            int width = 2 / _p->viewZoom; //* renderSize.w / viewportSize.w;
            if (width < 2)
                width = 2;
            vk.render->setTransform(mvp);
            
            drawRectOutline(vk.render, box, color, width);
            
#endif
        }

        void Viewport::_drawShape(
            const std::shared_ptr< draw::Shape >& shape,
            const float alphamult) noexcept
        {
            TLRENDER_P();
            MRV2_VK();
            
            auto note = dynamic_cast< draw::NoteShape* >(shape.get());
            if (note)
            {
                if (panel::annotationsPanel && alphamult == 1.F)
                    panel::annotationsPanel->notes->value(note->text.c_str());
            }
            else
            {
                float alpha = shape->color.a;
                shape->color.a *= alphamult;
                shape->color.a *= shape->fade;
                shape->draw(vk.render, vk.lines);
                shape->color.a = alpha;
            }
        }

        void Viewport::_drawAnnotations(
            const std::shared_ptr<tl::vk::OffscreenBuffer>& overlay,
            const math::Matrix4x4f& renderMVP, const otime::RationalTime& time,
            const std::vector<std::shared_ptr<draw::Annotation>>& annotations,
            const math::Size2i& renderSize)
        {
            TLRENDER_P();
            MRV2_VK();

            vk::OffscreenBufferBinding binding(overlay);
            

            timeline::RenderOptions renderOptions;
            renderOptions.colorBuffer = image::PixelType::RGBA_U8;

            vk.render->begin(renderSize, renderOptions);
            vk.render->setOCIOOptions(timeline::OCIOOptions());
            vk.render->setLUTOptions(timeline::LUTOptions());

            for (const auto& annotation : annotations)
            {
                const auto& annotationTime = annotation->time;
                float alphamult = 0.F;
                if (annotation->allFrames || time.floor() == annotationTime.floor())
                    alphamult = 1.F;
                else
                {
                    if (p.ghostPrevious)
                    {
                        for (short i = p.ghostPrevious - 1; i > 0; --i)
                        {
                            otime::RationalTime offset(i, time.rate());
                            if ((time - offset).floor() == annotationTime.floor())
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
                            if ((time + offset).floor() == annotationTime.floor())
                            {
                                alphamult = 1.F - (float)i / p.ghostNext;
                                break;
                            }
                        }
                    }
                }

                const auto& shapes = annotation->shapes;
                for (const auto& shape : shapes)
                {
                    if (alphamult <= 0.001F)
                        continue;
                    vk.render->setTransform(renderMVP);
                    
                    _drawShape(shape, alphamult);
                    
                }
            }

            vk.render->end();
        }

        void Viewport::_compositeAnnotations(
            const math::Matrix4x4f& mvp, const math::Size2i& viewportSize)
        {
            TLRENDER_P();
            MRV2_VK();

            // vk::SetAndRestore(GL_BLEND, GL_TRUE);

            // glBlendFuncSeparate(
            //     GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA,
            //     GL_ONE_MINUS_SRC_ALPHA); // this is needed to composite soft
            // brushes correctly.  Note that the
            // standardard premult composite is
            // done later in the shaders.
            

            // glViewport(0, 0, GLsizei(viewportSize.w), GLsizei(viewportSize.h));
            

            vk.annotationShader->bind();
            vk.annotationShader->setUniform("transform.mvp", mvp);
            timeline::Channels channels = timeline::Channels::Color;
            if (!p.displayOptions.empty())
                channels = p.displayOptions[0].channels;
            vk.annotationShader->setUniform("channels", static_cast<int>(channels));
        }

        void Viewport::_compositeOverlay(
            const std::shared_ptr<tl::vk::OffscreenBuffer>& overlay,
            const math::Matrix4x4f& mvp, const math::Size2i& viewportSize)
        {
            MRV2_VK();
            TLRENDER_P();
            if (!vk.overlayPBO)
                return;

            _compositeAnnotations(overlay, mvp, viewportSize);
            

            const size_t width = overlay->getSize().w;
            const size_t height = overlay->getSize().h;

            // Bind the overlay texture
            // glBindTexture(GL_TEXTURE_2D, overlay->getColorID());
            

            // Wait for the annotation PBO fence
            // GLenum waitReturn = glClientWaitSync(
            //     vk.overlayFence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
            
            // if (waitReturn == GL_TIMEOUT_EXPIRED)
            //     return;

            // glDeleteSync(vk.overlayFence);
            

            auto outputDevice = App::app->outputDevice();
            if (outputDevice)
            {
                // Retrieve pixel data
                // glBindTexture(GL_TEXTURE_2D, overlay->getColorID());

                const image::PixelType pixelType = image::PixelType::RGBA_U8;
                vk.annotationImage = image::Image::create(width, height, pixelType);

                // glGetTexImage(
                //     GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                //     vk.annotationImage->getData());
            }

            // Restore drawing to main viewport (needed for HUD and other annotation
            // code)
            // glViewport(0, 0, GLsizei(viewportSize.w), GLsizei(viewportSize.h));
            // glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
        }

        void Viewport::_compositeAnnotations(
            const std::shared_ptr<tl::vk::OffscreenBuffer>& overlay,
            const math::Matrix4x4f& orthoMatrix, const math::Size2i& viewportSize)
        {
            MRV2_VK();
            TLRENDER_P();

            _compositeAnnotations(orthoMatrix, viewportSize);

            // glActiveTexture(GL_TEXTURE0);
            // glBindTexture(GL_TEXTURE_2D, overlay->getColorID());

            if (vk.vao && vk.vbo)
            {
                vk.vao->bind();
                // vk.vao->draw(GL_TRIANGLES, 0, vk.vbo->getSize());
            }
        }

        void Viewport::_drawCropMask(const math::Size2i& renderSize) const noexcept
        {
            MRV2_VK();

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
                vk.render->drawRect(box, maskColor);
                box.max.y = renderSize.h;
                box.min.y = renderSize.h - Y;
                vk.render->drawRect(box, maskColor);
            }
            else
            {
                int X = renderSize.w * amountX;
                math::Box2i box(0, 0, X, renderSize.h);
                vk.render->drawRect(box, maskColor);
                box.max.x = renderSize.w;
                box.min.x = renderSize.w - X;
                vk.render->drawRect(box, maskColor);
            }
        }

        inline void Viewport::_drawText(
            const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
            math::Vector2i& pos, const int16_t lineHeight,
            const image::Color4f& labelColor) const noexcept
        {
            MRV2_VK();
            const image::Color4f shadowColor(0.F, 0.F, 0.F, 0.7F);
            math::Vector2i shadowPos{pos.x + 2, pos.y + 2};
            vk.render->drawText(glyphs, shadowPos, shadowColor);
            vk.render->drawText(glyphs, pos, labelColor);
            pos.y += lineHeight;
        }

        void Viewport::_drawSafeAreas(
            const float percentX, const float percentY,
            const float pixelAspectRatio, const image::Color4f& color,
            const math::Matrix4x4f& mvp, const char* label) const noexcept
        {
            MRV2_VK();
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

            int width = 2 / _p->viewZoom; //* renderSize.w / viewportSize.w;

#ifdef USE_ONE_PIXEL_LINES
            vk.outline->drawRect(box, color, mvp);
#else
            if (width < 2)
                width = 2;
            vk.render->setTransform(mvp);
            drawRectOutline(vk.render, box, color, width);
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
            vk.render->setTransform(mvp);
            vk.render->drawText(glyphs, pos, color);
        }

        void Viewport::_drawSafeAreas() const noexcept
        {
            TLRENDER_P();
            if (!p.player)
                return;

            const auto& info = p.player->player()->getIOInfo();
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

        void Viewport::_drawHUD(float alpha) const noexcept
        {
            TLRENDER_P();
            MRV2_VK();

            if (!p.fontSystem || p.videoData.empty() || !p.player)
                return;

            const auto& viewportSize = getViewportSize();

            static const std::string fontFamily = "NotoSans-Regular";
            Viewport* self = const_cast< Viewport* >(this);
            uint16_t fontSize = 12 * self->pixels_per_unit();

            Fl_Color c = p.ui->uiPrefs->uiPrefsViewHud->color();
            uint8_t r, g, b;
            Fl::get_color(c, r, g, b);

            const image::Color4f labelColor(r / 255.F, g / 255.F, b / 255.F, alpha);

            const image::FontInfo fontInfo(fontFamily, fontSize);
            const image::FontMetrics fontMetrics =
                p.fontSystem->getMetrics(fontInfo);
            auto lineHeight = fontMetrics.lineHeight;
            math::Vector2i pos(20, lineHeight * 2);

            const auto player = p.player;

            const auto& path = player->path();
            const otime::RationalTime& time = p.videoData[0].time;
            int64_t frame = time.to_frames();

            timeline::RenderOptions renderOptions;
            renderOptions.clear = false;
            vk.render->begin(viewportSize, renderOptions);

            char buf[512];
            if (p.hud & HudDisplay::kDirectory)
            {
                const auto& directory = path.getDirectory();
                _drawText(
                    p.fontSystem->getGlyphs(directory, fontInfo), pos, lineHeight,
                    labelColor);
            }

            bool otioClip = false;
            otime::RationalTime clipTime;
            if (p.hud & HudDisplay::kFilename)
            {
                std::string fullname = createStringFromPathAndTime(path, time);
                if (path.getExtension() == ".otio")
                {
                    otioClip = true;
                    auto i = p.tagData.find("otioClipName");
                    if (i != p.tagData.end())
                    {
                        auto otioClipName = i->second;
                        fullname += " ( " + otioClipName + " )";
                    }
                    i = p.tagData.find("otioClipTime");
                    if (i != p.tagData.end())
                    {
                        std::stringstream ss(i->second);
                        ss >> clipTime;
                    }
                }
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
                    if (video.size.pixelAspectRatio != 1.F)
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
                if (otioClip)
                {
                    int64_t clipFrame = clipTime.to_frames();
                    snprintf(buf, 512, " ( %" PRId64 " )", clipFrame);
                    tmp += buf;
                }
            }

            if (p.hud & HudDisplay::kFrameRange)
            {
                const auto& range = player->timeRange();
                frame = range.start_time().to_frames();
                const int64_t last_frame = range.end_time_inclusive().to_frames();
                snprintf(
                    buf, 512, " Range: %" PRId64 " -  %" PRId64, frame, last_frame);
                tmp += buf;
            }

            if (p.hud & HudDisplay::kTimecode)
            {
                snprintf(buf, 512, " TC: %s ", time.to_timecode(nullptr).c_str());
                tmp += buf;
                if (otioClip)
                {
                    snprintf(
                        buf, 512, " ( %s ) ",
                        clipTime.to_timecode(nullptr).c_str());
                    tmp += buf;
                }
            }

            if (p.hud & HudDisplay::kFPS)
            {
                if (player->playback() != timeline::Playback::Stop &&
                    (p.actionMode != ActionMode::kScrub || p.lastEvent != FL_DRAG))
                {
                    // Calculate skipped frames
                    int64_t absdiff = std::abs(time.value() - p.lastFrame);
                    if (absdiff > 1 && absdiff < 60)
                        p.droppedFrames += absdiff - 1;

                    // Calculate elapsed time
                    auto currentTime = std::chrono::high_resolution_clock::now();
                    auto frameTime =
                        std::chrono::duration<double>(currentTime - p.startTime)
                        .count();
                    p.frameTimes.push_back(frameTime);

                    // Average FPS over kFPSAverageFrames
                    if (p.frameTimes.size() >= kFPSAverageFrames)
                    {
                        p.frameTimes.pop_front();
                    }

                    // Calculate average frame time
                    double averageFrameTime = 0.0;
                    for (const double time : p.frameTimes)
                    {
                        averageFrameTime += time;
                    }
                    averageFrameTime /= p.frameTimes.size();

                    const double fps = 1.0 / averageFrameTime;

                    snprintf(
                        buf, 512, "DF: %" PRIu64 " FPS: %.2f/%.3f", p.droppedFrames,
                        fps, player->speed());

                    tmp += buf;
                    p.startTime = std::chrono::high_resolution_clock::now();
                }
            }

            p.lastFrame = time.value();

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
                _drawText(
                    p.fontSystem->getGlyphs(_("Cache:"), fontInfo), pos, lineHeight,
                    labelColor);
                const auto ioSystem =
                    App::app->getContext()->getSystem<io::System>();
                const auto& cache = ioSystem->getCache();
                const size_t maxCache = cache->getMax() / memory::gigabyte;
                const float pctCache = cache->getPercentage();
                const float usedCache = maxCache * (pctCache / 100.F);
                snprintf(
                    buf, 512, _("    Used: %.2g of %zu Gb (%.2g %%)"), usedCache,
                    maxCache, pctCache);
                _drawText(
                    p.fontSystem->getGlyphs(buf, fontInfo), pos, lineHeight,
                    labelColor);
                snprintf(
                    buf, 512, _("    Ahead    V: % 4" PRIu64 "    A: % 4" PRIu64),
                    aheadVideoFrames, aheadAudioFrames);
                _drawText(
                    p.fontSystem->getGlyphs(buf, fontInfo), pos, lineHeight,
                    labelColor);
                snprintf(
                    buf, 512, _("    Behind   V: % 4" PRIu64 "    A: % 4" PRIu64),
                    behindVideoFrames, behindAudioFrames);
                _drawText(
                    p.fontSystem->getGlyphs(buf, fontInfo), pos, lineHeight,
                    labelColor);
            }

            if (p.hud & HudDisplay::kAttributes)
            {
                for (const auto& tag : p.tagData)
                {
                    if (pos.y > viewportSize.h)
                        break;

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
            MRV2_VK();
            const auto& renderSize = getRenderSize();
            const auto& viewportSize = getViewportSize();
            const image::Color4f color(0.5, 0.5, 0.5, 1.0);

            math::Box2i box;
            std::stringstream ss(dw);
            ss >> box;

            box.min.y = -(renderSize.h - box.min.y);
            box.max.y = -(renderSize.h - box.max.y);

            math::Matrix4x4f vm =
                math::translate(math::Vector3f(p.viewPos.x, p.viewPos.y, 0.F));
            vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));
            const auto pm = math::ortho(
                0.F, static_cast<float>(viewportSize.w), 0.F,
                static_cast<float>(viewportSize.h), -1.F, 1.F);
            auto mvp = pm * vm;
            mvp = mvp * math::scale(math::Vector3f(1.F, -1.F, 1.F));
#ifdef USE_ONE_PIXEL_LINES
            _drawRectangleOutline(box, color, mvp);
#else
            vk.render->setTransform(mvp);
            drawRectOutline(vk.render, box, color, 2);
#endif
        }

        void Viewport::_drawDataWindow() const noexcept
        {
            TLRENDER_P();
            ;
            image::Tags::const_iterator i = p.tagData.find("Data Window");
            if (i == p.tagData.end())
                return;

            const std::string& dw = i->second;
            _drawWindowArea(dw);
        }

        void Viewport::_drawDisplayWindow() const noexcept
        {
            TLRENDER_P();

            image::Tags::const_iterator i = p.tagData.find("Display Window");
            if (i == p.tagData.end())
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
            if (!p.player)
                return;
            if (!p.fontSystem)
                return;

            MRV2_VK();

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

            vk.render->begin(viewportSize, renderOptions);
            vk.render->setOCIOOptions(timeline::OCIOOptions());
            vk.render->setLUTOptions(timeline::LUTOptions());

            vk.render->drawRect(
                box, image::Color4f(0.F, 0.F, 0.F, 0.7F * p.helpTextFade));

            _drawText(
                p.fontSystem->getGlyphs(p.helpText, fontInfo), pos, lineHeight,
                labelColor);

            vk.render->end();
        }

        void Viewport::_createPBOs(const math::Size2i& renderSize)
        {
            // MRV2_VK();
            // if (renderSize.w > 0 && renderSize.h > 0)
            // {
            //     if (vk.pboIDs[0] != 0)
            //     {
            //         glDeleteBuffers(2, vk.pboIDs);
            //         
            //         glDeleteSync(vk.pboFences[0]);
            //         
            //         glDeleteSync(vk.pboFences[1]);
            //         
            //     }
            //     glGenBuffers(2, vk.pboIDs);
            //     

            //     const vk::OffscreenBufferOptions& options = vk.buffer->getOptions();
            //     const size_t dataSize = renderSize.w * renderSize.h *
            //                             image::getChannelCount(options.colorType) *
            //                             image::getBitDepth(options.colorType);
            //     
            //     for (int i = 0; i < 2; ++i)
            //     {
            //         glBindBuffer(GL_PIXEL_PACK_BUFFER, vk.pboIDs[i]);
            //         
            //         glBufferData(
            //             GL_PIXEL_PACK_BUFFER, dataSize, nullptr, GL_STREAM_READ);
            //         
            //         vk.pboFences[i] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            //         
            //     }
            //     glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            //     
            // }
        }
        void Viewport::_createOverlayPBO(const math::Size2i& renderSize)
        {
            MRV2_VK();
            // if (renderSize.w > 0 && renderSize.h > 0)
            // {
            //     if (vk.overlayPBO != 0)
            //     {
            //         // Delete existing PBO if any
            //         glDeleteBuffers(1, &vk.overlayPBO);
            //     }
            //     glGenBuffers(1, &vk.overlayPBO);
            //     glBindBuffer(GL_PIXEL_PACK_BUFFER, vk.overlayPBO);
            //     glBufferData(
            //         GL_PIXEL_PACK_BUFFER, renderSize.w * renderSize.h * 4, nullptr,
            //         GL_STREAM_READ); // Allocate memory
            //     glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            // }
        }

    } // namespace opengl

} // namespace mrv

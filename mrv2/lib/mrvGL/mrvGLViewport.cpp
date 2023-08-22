// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cinttypes>

#include <tlCore/FontSystem.h>
#include <tlCore/StringFormat.h>

#include <tlGL/OffscreenBuffer.h>
#include <tlTimeline/GLRenderPrivate.h>
#include <tlGL/Init.h>
#include <tlGL/Util.h>

// mrViewer includes
#include "mrViewer.h"

#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvI8N.h"

#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvGL/mrvGLViewportPrivate.h"
#include "mrvGL/mrvGLUtil.h"
#include "mrvGL/mrvGLShaders.h"
#include "mrvGL/mrvGLShape.h"
#include "mrvGL/mrvTimelineViewport.h"
#include "mrvGL/mrvTimelineViewportPrivate.h"
#include "mrvGL/mrvGLViewport.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvDummyClient.h"

#include "mrvApp/mrvSettingsObject.h"

namespace
{
    const char* kModule = "view";
}

namespace mrv
{
    using namespace tl;

    Viewport::Viewport(int X, int Y, int W, int H, const char* L) :
        TimelineViewport(X, Y, W, H, L),
        _gl(new GLPrivate)
    {
        int stereo = 0;
        // if (can_do(FL_STEREO))
        //     stereo = FL_STEREO;
        int fl_double = FL_DOUBLE;
#ifdef __APPLE__
        // fl_double = 0; // @bug:  FL_DOUBLE in Tile suffers from flicker and
        //         red screen.
#endif

        mode(FL_RGB | fl_double | FL_ALPHA | FL_STENCIL | FL_OPENGL3 | stereo);
    }

    Viewport::~Viewport() {}

    void Viewport::setContext(const std::weak_ptr<system::Context>& context)
    {
        _gl->context = context;
    }

    //! Refresh window by clearing the associated resources.
    void Viewport::refresh()
    {
        TLRENDER_P();
        MRV2_GL();
        if (gl.render)
            glDeleteBuffers(2, gl.pboIds);
        gl.render.reset();
        gl.outline.reset();
        gl.lines.reset();
#ifdef USE_ONE_PIXEL_LINES
        gl.outline.reset();
#endif
        gl.buffer.reset();
        gl.annotation.reset();
        gl.shader.reset();
        gl.stereoShader.reset();
        gl.annotationShader.reset();
        gl.vbo.reset();
        gl.vao.reset();
        p.fontSystem.reset();
        gl.index = 0;
        gl.nextIndex = 1;
    }

    bool Viewport::_initializeGLResources()
    {
        TLRENDER_P();
        MRV2_GL();

        if (auto context = gl.context.lock())
        {

            gl.render = timeline::GLRender::create(context);
            CHECK_GL;

            glGenBuffers(2, gl.pboIds);
            CHECK_GL;

            p.fontSystem = image::FontSystem::create(context);
            CHECK_GL;

#ifdef USE_ONE_PIXEL_LINES
            gl.outline = std::make_shared<tl::gl::Outline>();
#endif
            gl.lines = std::make_shared<tl::gl::Lines>();
            CHECK_GL;

            try
            {
                const std::string& vertexSource = timeline::vertexSource();
                gl.shader =
                    gl::Shader::create(vertexSource, textureFragmentSource());
                CHECK_GL;
                gl.stereoShader =
                    gl::Shader::create(vertexSource, stereoFragmentSource());
                CHECK_GL;
                gl.annotationShader = gl::Shader::create(
                    vertexSource, annotationFragmentSource());
                CHECK_GL;
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
                return false;
            }
        }

        gl.vbo.reset();
        gl.vao.reset();
        gl.buffer.reset();
        gl.stereoBuffer.reset();
        return true;
    }

    bool Viewport::_initializeGL()
    {
        gl::initGLAD();

        refresh();

        return _initializeGLResources();
    }

    void Viewport::draw()
    {
        TLRENDER_P();
        MRV2_GL();

        if (!valid())
        {
            if (_initializeGL())
                valid(1);
        }

        CHECK_GL;

#ifdef DEBUG_SPEED
        auto start_time = std::chrono::steady_clock::now();
#endif

        const auto& viewportSize = getViewportSize();
        const auto& renderSize = getRenderSize();
        try
        {
            if (renderSize.isValid())
            {
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        p.displayOptions[0].imageFilters;
                }
                offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
                offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
                CHECK_GL;
                if (gl::doCreate(gl.buffer, renderSize, offscreenBufferOptions))
                {
                    CHECK_GL;
                    gl.buffer = gl::OffscreenBuffer::create(
                        renderSize, offscreenBufferOptions);
                    CHECK_GL;
                    unsigned dataSize =
                        renderSize.w * renderSize.h * 4 * sizeof(GLfloat);
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[0]);
                    CHECK_GL;
                    glBufferData(
                        GL_PIXEL_PACK_BUFFER, dataSize, 0, GL_STREAM_READ);
                    CHECK_GL;
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[1]);
                    CHECK_GL;
                    glBufferData(
                        GL_PIXEL_PACK_BUFFER, dataSize, 0, GL_STREAM_READ);
                    CHECK_GL;
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                    CHECK_GL;
                }

                if (can_do(FL_STEREO))
                {
                    CHECK_GL;
                    if (gl::doCreate(
                            gl.stereoBuffer, renderSize,
                            offscreenBufferOptions))
                    {
                        CHECK_GL;
                        gl.stereoBuffer = gl::OffscreenBuffer::create(
                            renderSize, offscreenBufferOptions);
                        CHECK_GL;
                    }
                    CHECK_GL;
                }
            }
            else
            {
                CHECK_GL;
                gl.buffer.reset();
                CHECK_GL;
                gl.stereoBuffer.reset();
                CHECK_GL;
            }

            if (gl.buffer && gl.render)
            {
                if (p.stereo3DOptions.output == Stereo3DOutput::OpenGL &&
                    p.stereo3DOptions.input == Stereo3DInput::Image &&
                    p.videoData.size() > 1)
                {
                    CHECK_GL;
                    _drawStereoOpenGL();
                    CHECK_GL;
                }
                else
                {
                    CHECK_GL;
                    gl::OffscreenBufferBinding binding(gl.buffer);
                    CHECK_GL;
                    char* saved_locale = strdup(setlocale(LC_NUMERIC, NULL));
                    setlocale(LC_NUMERIC, "C");
                    gl.render->begin(
                        renderSize, p.colorConfigOptions, p.lutOptions);
                    CHECK_GL;
                    if (p.missingFrame &&
                        p.missingFrameType != MissingFrameType::kBlackFrame)
                    {
                        _drawMissingFrame(renderSize);
                        CHECK_GL;
                    }
                    else
                    {
                        if (p.stereo3DOptions.input == Stereo3DInput::Image &&
                            p.videoData.size() > 1)
                        {
                            _drawStereo3D();
                            CHECK_GL;
                        }
                        else
                        {
                            CHECK_GL;
                            if (!p.videoData.empty())
                            {
                                gl.render->drawVideo(
                                    p.videoData,
                                    timeline::getBoxes(
                                        p.compareOptions.mode, p.timelineSizes),
                                    p.imageOptions, p.displayOptions,
                                    p.compareOptions);
                                CHECK_GL;
                            }
                        }
                    }
                    CHECK_GL;
                    _drawOverlays(renderSize);
                    CHECK_GL;
                    gl.render->end();
                    CHECK_GL;
                    setlocale(LC_NUMERIC, saved_locale);
                    free(saved_locale);
                }
            }
        }
        catch (const std::exception& e)
        {
        }

        glViewport(0, 0, GLsizei(viewportSize.w), GLsizei(viewportSize.h));
        CHECK_GL;

        float r = 0.F, g = 0.F, b = 0.F, a = 1.F;
        if (!p.presentation && !p.blackBackground)
        {
            uint8_t ur, ug, ub;
            Fl::get_color(p.ui->uiPrefs->uiPrefsViewBG->color(), ur, ug, ub);
            r = ur / 255.0f;
            g = ug / 255.0f;
            b = ub / 255.0f;
        }

        glDrawBuffer(GL_BACK_LEFT);
        CHECK_GL;
        glClearColor(r, g, b, a);
        CHECK_GL;
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        CHECK_GL;

        if (gl.buffer && gl.shader)
        {
            math::Matrix4x4f mvp;

            if (p.environmentMapOptions.type != EnvironmentMapOptions::kNone)
            {
                mvp = _createEnvironmentMap();
            }
            else
            {
                mvp = _createTexturedRectangle();
            }

            CHECK_GL;
            gl.shader->bind();
            CHECK_GL;
            gl.shader->setUniform("transform.mvp", mvp);
            CHECK_GL;

            glActiveTexture(GL_TEXTURE0);
            CHECK_GL;
            glBindTexture(GL_TEXTURE_2D, gl.buffer->getColorID());
            CHECK_GL;

            if (gl.vao && gl.vbo)
            {
                CHECK_GL;
                gl.vao->bind();
                CHECK_GL;
                gl.vao->draw(GL_TRIANGLES, 0, gl.vbo->getSize());
                CHECK_GL;
            }

            if (p.stereo3DOptions.output == Stereo3DOutput::OpenGL &&
                p.stereo3DOptions.input == Stereo3DInput::Image &&
                p.videoData.size() > 1)
            {
                CHECK_GL;
                gl.shader->bind();
                CHECK_GL;
                gl.shader->setUniform("transform.mvp", mvp);
                CHECK_GL;

                glActiveTexture(GL_TEXTURE0);
                CHECK_GL;
                glBindTexture(GL_TEXTURE_2D, gl.stereoBuffer->getColorID());
                CHECK_GL;

                if (gl.vao && gl.vbo)
                {
                    glDrawBuffer(GL_BACK_RIGHT);
                    CHECK_GL;
                    gl.vao->bind();
                    CHECK_GL;
                    gl.vao->draw(GL_TRIANGLES, 0, gl.vbo->getSize());
                    CHECK_GL;
                }
            }

            if (gl.vao && gl.vbo)
            {
                math::Box2i selection = p.colorAreaInfo.box = p.selection;
                if (selection.max.x >= 0)
                {
                    // Check min < max
                    if (selection.min.x > selection.max.x)
                    {
                        int tmp = selection.max.x;
                        selection.max.x = selection.min.x;
                        selection.min.x = tmp;
                    }
                    if (selection.min.y > selection.max.y)
                    {
                        int tmp = selection.max.y;
                        selection.max.y = selection.min.y;
                        selection.min.y = tmp;
                    }
                    // Copy it again in case it changed
                    p.colorAreaInfo.box = selection;

                    _mapBuffer();
                    CHECK_GL;
                }
                else
                {
                    p.image = nullptr;
                }
                if (colorAreaPanel)
                {
                    _calculateColorArea(p.colorAreaInfo);
                    colorAreaPanel->update(p.colorAreaInfo);
                }
                if (histogramPanel)
                {
                    histogramPanel->update(p.colorAreaInfo);
                }
                if (vectorscopePanel)
                {
                    vectorscopePanel->update(p.colorAreaInfo);
                }

                // Uodate the pixel bar from here only if we are playing a movie
                // and one that is not 1 frames long.
                bool update = !_shouldUpdatePixelBar();
                if (update)
                    updatePixelBar();

                CHECK_GL;
                _unmapBuffer();
                CHECK_GL;

                update = _isPlaybackStopped() || _isSingleFrame();
                if (update)
                    updatePixelBar();

                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        p.displayOptions[0].imageFilters;
                }
                offscreenBufferOptions.depth = gl::OffscreenDepth::None;
                offscreenBufferOptions.stencil = gl::OffscreenStencil::None;
                CHECK_GL;
                if (gl::doCreate(
                        gl.annotation, viewportSize, offscreenBufferOptions))
                {
                    CHECK_GL;
                    gl.annotation = gl::OffscreenBuffer::create(
                        viewportSize, offscreenBufferOptions);
                    CHECK_GL;
                }

                if (p.showAnnotations && gl.annotation)
                {
                    CHECK_GL;
                    _drawAnnotations(mvp);
                    CHECK_GL;
                }

                if (p.selection.max.x >= 0)
                {
                    Fl_Color c = p.ui->uiPrefs->uiPrefsViewSelection->color();
                    uint8_t r, g, b;
                    Fl::get_color(c, r, g, b);

                    const image::Color4f color(r / 255.F, g / 255.F, b / 255.F);

                    math::Box2i selection = p.selection;
                    if (selection.min == selection.max)
                    {
                        selection.max.x++;
                        selection.max.y++;
                    }
                    _drawRectangleOutline(selection, color, mvp);
                    CHECK_GL;
                }

                // Refresh media info panel if there's data window present
                if (imageInfoPanel && !p.videoData.empty() &&
                    !p.videoData[0].layers.empty() &&
                    p.videoData[0].layers[0].image)
                {
                    const auto& tags =
                        p.videoData[0].layers[0].image->getTags();
                    image::Tags::const_iterator i = tags.find("Data Window");
                    if (i != tags.end())
                        imageInfoPanel->refresh();
                }

                if (p.dataWindow)
                    _drawDataWindow();
                CHECK_GL;
                if (p.displayWindow)
                    _drawDisplayWindow();
                CHECK_GL;

                if (p.safeAreas)
                    _drawSafeAreas();
                CHECK_GL;

                _drawCursor(mvp);
                CHECK_GL;
            }

            if (p.hudActive && p.hud != HudDisplay::kNone)
                _drawHUD();
            CHECK_GL;

            if (!p.helpText.empty())
                _drawHelpText();
            CHECK_GL;
        }

        MultilineInput* w = getMultilineInput();
        if (w)
        {
            std_any value;
            value = p.ui->app->settingsObject()->value(kFontSize);
            int font_size = std_any_cast<int>(value);
            double pixels_unit = pixels_per_unit();
            double pct = 1.0; // viewportSize.w / 1024.F;
            double fontSize = font_size * pct * p.viewZoom / pixels_unit;
            w->textsize(fontSize);
            math::Vector2i pos(w->pos.x, w->pos.y);
            // This works to pan without a change in zoom!
            // pos.x = pos.x + p.viewPos.x / p.viewZoom
            //         - w->viewPos.x / w->viewZoom;
            // pos.y = pos.y - p.viewPos.y / p.viewZoom
            //         - w->viewPos.y / w->viewZoom;
            // cur.x = (cur.x - p.viewPos.x / pixels_unit) / p.viewZoom;
            // cur.y = (cur.y - p.viewPos.y / pixels_unit) / p.viewZoom;

            // pos.x = (pos.x - w->viewPos.x / pixels_unit) /
            //         w->viewZoom;
            // pos.y = (pos.y - w->viewPos.y / pixels_unit) /
            //         w->viewZoom;

            // pos.x += cur.x;
            // pos.y -= cur.y;

            // std::cerr << "pos=" << pos << std::endl;
            // std::cerr << "p.viewPos=" << p.viewPos << std::endl;
            // std::cerr << "END " << pos << std::endl;
            w->Fl_Widget::position(pos.x, pos.y);
        }

#ifdef USE_OPENGL2
        Fl_Gl_Window::draw_begin(); // Set up 1:1 projectionç
        Fl_Window::draw();          // Draw FLTK children
        glViewport(0, 0, viewportSize.w, viewportSize.h);
        if (p.showAnnotations)
            _drawGL2TextShapes();
        Fl_Gl_Window::draw_end(); // Restore GL state
#else
        Fl_Gl_Window::draw();
#endif

#ifdef DEBUG_SPEED
        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end_time - start_time;
        std::cout << "GL::draw() duration " << diff.count() << std::endl;
#endif
    }

    void Viewport::_calculateColorAreaFullValues(area::Info& info) noexcept
    {
        TLRENDER_P();
        MRV2_GL();

        PixelToolBarClass* c = p.ui->uiPixelWindow;
        BrightnessType brightness_type = (BrightnessType)c->uiLType->value();
        int hsv_colorspace = c->uiBColorType->value() + 1;

        const int maxX = info.box.max.x;
        const int maxY = info.box.max.y;
        const auto& renderSize = gl.buffer->getSize();

        for (int Y = info.box.y(); Y <= maxY; ++Y)
        {
            for (int X = info.box.x(); X <= maxX; ++X)
            {
                image::Color4f rgba, hsv;
                rgba.b = p.image[(X + Y * renderSize.w) * 4];
                rgba.g = p.image[(X + Y * renderSize.w) * 4 + 1];
                rgba.r = p.image[(X + Y * renderSize.w) * 4 + 2];
                rgba.a = p.image[(X + Y * renderSize.w) * 4 + 3];

                info.rgba.mean.r += rgba.r;
                info.rgba.mean.g += rgba.g;
                info.rgba.mean.b += rgba.b;
                info.rgba.mean.a += rgba.a;

                if (rgba.r < info.rgba.min.r)
                    info.rgba.min.r = rgba.r;
                if (rgba.g < info.rgba.min.g)
                    info.rgba.min.g = rgba.g;
                if (rgba.b < info.rgba.min.b)
                    info.rgba.min.b = rgba.b;
                if (rgba.a < info.rgba.min.a)
                    info.rgba.min.a = rgba.a;

                if (rgba.r > info.rgba.max.r)
                    info.rgba.max.r = rgba.r;
                if (rgba.g > info.rgba.max.g)
                    info.rgba.max.g = rgba.g;
                if (rgba.b > info.rgba.max.b)
                    info.rgba.max.b = rgba.b;
                if (rgba.a > info.rgba.max.a)
                    info.rgba.max.a = rgba.a;

                hsv = rgba_to_hsv(hsv_colorspace, rgba);
                hsv.a = calculate_brightness(rgba, brightness_type);
                hsv_to_info(hsv, info);
            }
        }

        int num = info.box.w() * info.box.h();
        info.rgba.mean.r /= num;
        info.rgba.mean.g /= num;
        info.rgba.mean.b /= num;
        info.rgba.mean.a /= num;

        info.rgba.diff.r = info.rgba.max.r - info.rgba.min.r;
        info.rgba.diff.g = info.rgba.max.g - info.rgba.min.g;
        info.rgba.diff.b = info.rgba.max.b - info.rgba.min.b;
        info.rgba.diff.a = info.rgba.max.a - info.rgba.min.a;

        info.hsv.mean.r /= num;
        info.hsv.mean.g /= num;
        info.hsv.mean.b /= num;
        info.hsv.mean.a /= num;

        info.hsv.diff.r = info.hsv.max.r - info.hsv.min.r;
        info.hsv.diff.g = info.hsv.max.g - info.hsv.min.g;
        info.hsv.diff.b = info.hsv.max.b - info.hsv.min.b;
        info.hsv.diff.a = info.hsv.max.a - info.hsv.min.a;
    }

    void Viewport::_calculateColorArea(area::Info& info)
    {
        TLRENDER_P();
        MRV2_GL();

        if (!p.image || !gl.buffer)
            return;

        info.rgba.max.r = std::numeric_limits<float>::min();
        info.rgba.max.g = std::numeric_limits<float>::min();
        info.rgba.max.b = std::numeric_limits<float>::min();
        info.rgba.max.a = std::numeric_limits<float>::min();

        info.rgba.min.r = std::numeric_limits<float>::max();
        info.rgba.min.g = std::numeric_limits<float>::max();
        info.rgba.min.b = std::numeric_limits<float>::max();
        info.rgba.min.a = std::numeric_limits<float>::max();

        info.rgba.mean.r = info.rgba.mean.g = info.rgba.mean.b =
            info.rgba.mean.a = 0.F;

        info.hsv.max.r = std::numeric_limits<float>::min();
        info.hsv.max.g = std::numeric_limits<float>::min();
        info.hsv.max.b = std::numeric_limits<float>::min();
        info.hsv.max.a = std::numeric_limits<float>::min();

        info.hsv.min.r = std::numeric_limits<float>::max();
        info.hsv.min.g = std::numeric_limits<float>::max();
        info.hsv.min.b = std::numeric_limits<float>::max();
        info.hsv.min.a = std::numeric_limits<float>::max();

        info.hsv.mean.r = info.hsv.mean.g = info.hsv.mean.b = info.hsv.mean.a =
            0.F;

        if (p.ui->uiPixelWindow->uiPixelValue->value() == PixelValue::kFull)
            _calculateColorAreaFullValues(info);
        else
            _calculateColorAreaRawValues(info);
    }

    void Viewport::_mapBuffer() const noexcept
    {
        MRV2_GL();
        TLRENDER_P();

        if (p.ui->uiPixelWindow->uiPixelValue->value() == PixelValue::kFull)
        {

            // For faster access, we muse use BGRA.
            constexpr GLenum format = GL_BGRA;
            constexpr GLenum type = GL_FLOAT;

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);

            gl::OffscreenBufferBinding binding(gl.buffer);
            const auto& renderSize = gl.buffer->getSize();

            // bool update = _shouldUpdatePixelBar();
            bool stopped = _isPlaybackStopped();
            bool single_frame = _isSingleFrame();

            // set the target framebuffer to read
            // "index" is used to read pixels from framebuffer to a PBO
            // "nextIndex" is used to update pixels in the other PBO
            gl.index = (gl.index + 1) % 2;
            gl.nextIndex = (gl.index + 1) % 2;

            // If we are a single frame, we do a normal ReadPixels of front
            // buffer.
            if (single_frame)
            {
                _unmapBuffer();
                _mallocBuffer();
                if (!p.image)
                    return;
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                CHECK_GL;
                glReadBuffer(GL_FRONT);
                CHECK_GL;
                glReadPixels(
                    0, 0, renderSize.w, renderSize.h, format, type, p.image);
                return;
            }
            else
            {

                // read pixels from framebuffer to PBO
                // glReadPixels() should return immediately.
                glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[gl.index]);
                CHECK_GL;

                glReadPixels(0, 0, renderSize.w, renderSize.h, format, type, 0);
                CHECK_GL;

                // map the PBO to process its data by CPU
                glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[gl.nextIndex]);
                CHECK_GL;

                // We are stopped, read the first PBO.
                if (stopped)
                {
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[gl.index]);
                    CHECK_GL;
                }
            }

            if (p.rawImage)
            {
                free(p.image);
                p.image = nullptr;
            }

            p.image = (float*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
            CHECK_GL;
            p.rawImage = false;
        }
        else
        {
            TimelineViewport::_mapBuffer();
        }
    }

    void Viewport::_unmapBuffer() const noexcept
    {
        TLRENDER_P();

        if (p.image)
        {
            if (!p.rawImage)
            {
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                CHECK_GL;
                p.image = nullptr;
                p.rawImage = true;
            }
        }

        // back to conventional pixel operation
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        CHECK_GL;
    }

    void Viewport::_readPixel(image::Color4f& rgba) const noexcept
    {

        TLRENDER_P();
        MRV2_GL();

        math::Vector2i pos;
        pos.x = (p.mousePos.x - p.viewPos.x) / p.viewZoom;
        pos.y = (p.mousePos.y - p.viewPos.y) / p.viewZoom;

        if (p.ui->uiPixelWindow->uiPixelValue->value() != PixelValue::kFull)
        {
            if (_isEnvironmentMap())
                return;

            rgba.r = rgba.g = rgba.b = rgba.a = 0.f;

            for (const auto& video : p.videoData)
            {
                for (const auto& layer : video.layers)
                {
                    const auto& image = layer.image;
                    if (!image->isValid())
                        continue;

                    image::Color4f pixel, pixelB;

                    _getPixelValue(pixel, image, pos);

                    const auto& imageB = layer.image;
                    if (imageB->isValid())
                    {
                        _getPixelValue(pixelB, imageB, pos);

                        if (layer.transition == timeline::Transition::Dissolve)
                        {
                            float f2 = layer.transitionValue;
                            float f = 1.0 - f2;
                            pixel.r = pixel.r * f + pixelB.r * f2;
                            pixel.g = pixel.g * f + pixelB.g * f2;
                            pixel.b = pixel.b * f + pixelB.b * f2;
                            pixel.a = pixel.a * f + pixelB.a * f2;
                        }
                    }
                    rgba.r += pixel.r;
                    rgba.g += pixel.g;
                    rgba.b += pixel.b;
                    rgba.a += pixel.a;
                }
            }
        }
        else
        {
            // This is needed as the FL_MOVE of fltk wouuld get called
            // before the draw routine
            if (!gl.buffer || !valid())
                return;

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);

            // We use ReadPixels when the movie is stopped or has only a
            // a single frame.
            bool update = _shouldUpdatePixelBar();

            if (_isEnvironmentMap())
            {
                update = true;
            }

            const GLenum type = GL_FLOAT;

            if (update)
            {
                _unmapBuffer();
                if (_isEnvironmentMap())
                {
                    pos = _getFocus();
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    glReadBuffer(GL_FRONT);
                    glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, type, &rgba);
                    return;
                }
                else
                {
                    CHECK_GL;
                    Viewport* self = const_cast<Viewport*>(this);
                    self->make_current();
                    CHECK_GL;
                    gl::OffscreenBufferBinding binding(gl.buffer);
                    CHECK_GL;
                    assert(pos.x < gl.buffer->getSize().w);
                    assert(pos.y < gl.buffer->getSize().h);
                    glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, type, &rgba);
                    CHECK_GL;
                    return;
                }
            }

            if (!p.image)
                _mapBuffer();

            if (p.image)
            {
                const auto& renderSize = gl.buffer->getSize();
                rgba.b = p.image[(pos.x + pos.y * renderSize.w) * 4];
                rgba.g = p.image[(pos.x + pos.y * renderSize.w) * 4 + 1];
                rgba.r = p.image[(pos.x + pos.y * renderSize.w) * 4 + 2];
                rgba.a = p.image[(pos.x + pos.y * renderSize.w) * 4 + 3];
            }
        }

        _unmapBuffer();
    }

    int Viewport::handle(int event)
    {
        return TimelineViewport::handle(event);
    }

    void Viewport::_pushAnnotationShape(const std::string& command) const
    {
        // We should not update tcp client when not needed
        if (dynamic_cast< DummyClient* >(tcp))
            return;

        bool send = _p->ui->uiPrefs->SendAnnotations->value();
        if (!send)
            return;

        const auto player = getTimelinePlayer();
        if (!player)
            return;

        auto annotation = player->getAnnotation();
        if (!annotation)
            return;
        auto shape = annotation->lastShape().get();
        if (!shape)
            return;
        Message value;
        if (dynamic_cast< GLArrowShape* >(shape))
        {
            auto s = dynamic_cast< GLArrowShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< GLRectangleShape* >(shape))
        {
            auto s = dynamic_cast< GLRectangleShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< GLCircleShape* >(shape))
        {
            auto s = dynamic_cast< GLCircleShape* >(shape);
            value = *s;
        }
#ifdef USE_OPENGL2
        else if (dynamic_cast< GL2TextShape* >(shape))
        {
            auto s = dynamic_cast< GL2TextShape* >(shape);
            value = *s;
        }
#else
        else if (dynamic_cast< GLTextShape* >(shape))
        {
            auto s = dynamic_cast< GLTextShape* >(shape);
            value = *s;
        }
#endif
        else if (dynamic_cast< draw::NoteShape* >(shape))
        {
            auto s = dynamic_cast< draw::NoteShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< GLErasePathShape* >(shape))
        {
            auto s = dynamic_cast< GLErasePathShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< GLPathShape* >(shape))
        {
            auto s = dynamic_cast< GLPathShape* >(shape);
            value = *s;
        }
        else
        {
            throw std::runtime_error("Unknown shape");
        }
        Message msg;
        msg["command"] = command;
        msg["value"] = value;
        tcp->pushMessage(msg);
    }

} // namespace mrv

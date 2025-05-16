// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cinttypes>

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <tlCore/FontSystem.h>
#include <tlCore/StringFormat.h>

#include <tlDevice/IOutput.h>

#include <tlGL/OffscreenBuffer.h>
#include <tlTimelineGL/RenderPrivate.h>
#include <tlGL/Init.h>
#include <tlGL/Util.h>

// mrViewer .fl includes
#include "mrViewer.h"
#include "mrvHotkeyUI.h"

#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvI8N.h"

#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvFl/mrvOCIO.h"
#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvGL/mrvGLViewportPrivate.h"
#include "mrvGL/mrvGLDefines.h"
#include "mrvGL/mrvGLErrors.h"
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
    namespace opengl
    {
    
        using namespace tl;

        Viewport::Viewport(int X, int Y, int W, int H, const char* L) :
            TimelineViewport(X, Y, W, H, L),
            _gl(new GLPrivate)
        {
            int stereo = 0;
            int fl_double = FL_DOUBLE; // needed on Linux and _WIN32
#if defined(__APPLE__)
            // \bug: Do not use FL_DOUBLE on APPLE as it makes
            // playback slow
            fl_double = 0;
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
            gl.render.reset();
            gl.lines.reset();
            gl.buffer.reset();
            gl.annotation.reset();
            gl.overlay.reset();
            gl.shader.reset();
            gl.annotationShader.reset();
            gl.vbo.reset();
            gl.vao.reset();
            p.fontSystem.reset();
            gl.currentPBOIndex = 0;
            gl.nextPBOIndex = 1;
        }

        void Viewport::_initializeGLResources()
        {
            TLRENDER_P();
            MRV2_GL();

#ifdef __APPLE__
            // On Apple, there's no OpenGL BACK buffer.
#    undef GL_BACK_LEFT
#    undef GL_BACK_RIGHT
#    define GL_BACK_LEFT GL_FRONT_LEFT
#    define GL_BACK_RIGHT GL_FRONT_RIGHT
#endif

            if (auto context = gl.context.lock())
            {

                gl.render = timeline_gl::Render::create(context);
                p.fontSystem = image::FontSystem::create(context);

                gl.lines = std::make_shared<opengl::Lines>();

                try
                {
                    const std::string& vertexSource = timeline_gl::vertexSource();
                    gl.shader =
                        gl::Shader::create(vertexSource, textureFragmentSource());
                    gl.annotationShader = gl::Shader::create(
                        vertexSource, annotationFragmentSource());
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR(e.what());
                }
            }
        }

        void Viewport::_initializeGL()
        {
            MRV2_GL();
            gl::initGLAD();

#ifdef TLRENDER_API_GL_4_1_Debug
            if (!gl.init_debug)
            {
                gl.init_debug = true;
#    ifndef __APPLE__
                // Apple's OpenGL 4.1 does not support glDebugMessageCallback.
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(glDebugOutput, nullptr);
                glDebugMessageControl(
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE), 0, nullptr, GL_TRUE);
#    endif
            }
#endif

            refresh();

            _initializeGLResources();
        }

        int Viewport::handle(int event)
        {
            switch (event)
            {
            case FL_HIDE:
                refresh();
                valid(0);
                break;
            default:
                break;
            }

            return TimelineViewport::handle(event);
        }

        void Viewport::draw()
        {
            TLRENDER_P();
            MRV2_GL();

            make_current(); // needed to work with GLFW

            if (!valid())
            {
                _initializeGL();
                CHECK_GL;

                if (p.ui->uiPrefs->uiPrefsOpenGLVsync->value() ==
                    MonitorVSync::kVSyncNone)
                    swap_interval(0);
                else
                    swap_interval(1);

                valid(1);
            }
            CHECK_GL;

            const auto& viewportSize = getViewportSize();
            const auto& renderSize = getRenderSize();

            bool hasAlpha = false;
            const float alpha = p.ui->uiMain->get_alpha() / 255.F;
            if (alpha < 1.0F)
            {
                hasAlpha = true;
            }

            const bool transparent =
                hasAlpha ||
                getBackgroundOptions().type == timeline::Background::Transparent;

            CHECK_GL;
            try
            {
                if (renderSize.isValid())
                {
                    gl.colorBufferType = image::PixelType::RGBA_U8;
                    int accuracy = p.ui->uiPrefs->uiPrefsColorAccuracy->value();
                    switch (accuracy)
                    {
                    case kAccuracyFloat32:
                        gl.colorBufferType = image::PixelType::RGBA_F32;
                        hasAlpha = true;
                        break;
                    case kAccuracyFloat16:
                        gl.colorBufferType = image::PixelType::RGBA_F16;
                        hasAlpha = true;
                        break;
                    case kAccuracyAuto:
                        image::PixelType pixelType = image::PixelType::RGBA_U8;
                        auto& video = p.videoData[0];
                        if (p.missingFrame &&
                            p.missingFrameType != MissingFrameType::kBlackFrame)
                        {
                            video = p.lastVideoData;
                        }

                        if (!video.layers.empty() && video.layers[0].image &&
                            video.layers[0].image->isValid())
                        {
                            pixelType = video.layers[0].image->getPixelType();
                            switch (pixelType)
                            {
                            case image::PixelType::RGBA_F32:
                            case image::PixelType::LA_F32:
                                hasAlpha = true;
                            case image::PixelType::RGB_F32:
                            case image::PixelType::L_F32:
                                gl.colorBufferType = image::PixelType::RGBA_F32;
                                break;
                            case image::PixelType::RGBA_F16:
                            case image::PixelType::LA_F16:
                                hasAlpha = true;
                            case image::PixelType::RGB_F16:
                            case image::PixelType::L_F16:
                                gl.colorBufferType = image::PixelType::RGBA_F16;
                                break;
                            case image::PixelType::RGBA_U16:
                            case image::PixelType::LA_U16:
                                hasAlpha = true;
                            case image::PixelType::RGB_U16:
                            case image::PixelType::L_U16:
                                gl.colorBufferType = image::PixelType::RGBA_U16;
                                break;
                            case image::PixelType::RGBA_U8:
                            case image::PixelType::LA_U8:
                                hasAlpha = true;
                                break;
                            default:
                                break;
                            }
                        }
                        break;
                    }

                    CHECK_GL;
                    gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType = gl.colorBufferType;

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
                        _createPBOs(renderSize);
                        CHECK_GL;
                    }
                    CHECK_GL;

                    if (can_do(FL_STEREO))
                    {
                        if (gl::doCreate(
                                gl.stereoBuffer, renderSize,
                                offscreenBufferOptions))
                        {
                            gl.stereoBuffer = gl::OffscreenBuffer::create(
                                renderSize, offscreenBufferOptions);
                        }
                    }
                    CHECK_GL;
                }
                else
                {
                    gl.buffer.reset();
                    gl.stereoBuffer.reset();
                    CHECK_GL;
                }
                CHECK_GL;

                if (gl.buffer && gl.render)
                {

                    if (p.stereo3DOptions.output == Stereo3DOutput::Glasses &&
                        p.stereo3DOptions.input == Stereo3DInput::Image &&
                        p.videoData.size() > 1 && p.showVideo)
                    {
                        _drawStereoOpenGL();
                    }
                    else
                    {
                        gl::OffscreenBufferBinding binding(gl.buffer);

                        locale::SetAndRestore saved;
                        timeline::RenderOptions renderOptions;
                        renderOptions.colorBuffer = gl.colorBufferType;

                        gl.render->begin(renderSize, renderOptions);

                        if (p.showVideo)
                        {
                            int screen = this->screen_num();
                            if (screen >= 0 && !p.monitorOCIOOptions.empty() &&
                                screen < p.monitorOCIOOptions.size())
                            {
                                timeline::OCIOOptions o = p.ocioOptions;
                                o.display = p.monitorOCIOOptions[screen].display;
                                o.view = p.monitorOCIOOptions[screen].view;
                                gl.render->setOCIOOptions(o);

                                _updateMonitorDisplayView(screen, o);
                            }
                            else
                            {
                                gl.render->setOCIOOptions(p.ocioOptions);
                                _updateMonitorDisplayView(screen, p.ocioOptions);
                            }

                            gl.render->setLUTOptions(p.lutOptions);
                            gl.render->setHDROptions(p.hdrOptions);
                            if (p.missingFrame &&
                                p.missingFrameType != MissingFrameType::kBlackFrame)
                            {
                                _drawMissingFrame(renderSize);
                            }
                            else
                            {
                                if (p.stereo3DOptions.input ==
                                    Stereo3DInput::Image &&
                                    p.videoData.size() > 1)
                                {
                                    _drawStereo3D();
                                }
                                else
                                {
                                    gl.render->drawVideo(
                                        p.videoData,
                                        timeline::getBoxes(
                                            p.compareOptions.mode, p.videoData),
                                        p.imageOptions, p.displayOptions,
                                        p.compareOptions, getBackgroundOptions());
                                }
                            }
                            _drawOverlays(renderSize);
                            gl.render->end();
                        }
                    }
                }
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
                gl.buffer.reset();
                gl.stereoBuffer.reset();
            }
            CHECK_GL;

            float r = 0.F, g = 0.F, b = 0.F, a = 0.F;

            if (!p.presentation)
            {
                Fl_Color c = p.ui->uiPrefs->uiPrefsViewBG->color();

                uint8_t ur = 0, ug = 0, ub = 0, ua = 0;
                Fl::get_color(c, ur, ug, ub, ua);

                r = ur / 255.0f;
                g = ug / 255.0f;
                b = ub / 255.0f;
                a = alpha;

                if (desktop::Wayland())
                {
                    p.ui->uiViewGroup->color(fl_rgb_color(ur, ug, ub));
                    p.ui->uiViewGroup->redraw();
                }
            }
            else
            {
                if (desktop::Wayland())
                {
                    p.ui->uiViewGroup->color(fl_rgb_color(0, 0, 0));
                    p.ui->uiViewGroup->redraw();
                }

                // Hide the cursor if in presentation time after 3 seconds of
                // inactivity.
                const auto& time = std::chrono::high_resolution_clock::now();
                const auto elapsedTime =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        time - p.presentationTime)
                    .count();
                if (elapsedTime >= 3000)
                {
                    // If user is changing preferences or hotkeys, keep
                    // the default cursor.
                    if (p.ui->uiPrefs->uiMain->visible() ||
                        p.ui->uiHotkey->uiMain->visible() ||
                        p.ui->uiAbout->uiMain->visible())
                    {
                        window()->cursor(FL_CURSOR_DEFAULT);
                    }
                    else
                    {
                        window()->cursor(FL_CURSOR_NONE);
                    }
                    p.presentationTime = time;
                }
            }

            CHECK_GL;
            glDrawBuffer(GL_BACK_LEFT);
            CHECK_GL;

            glViewport(0, 0, GLsizei(viewportSize.w), GLsizei(viewportSize.h));
            glClearStencil(0);
            glClearColor(r, g, b, a);
            glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            CHECK_GL;

            const auto& player = getTimelinePlayer();
            if (!player)
            {
#ifdef __APPLE__
                set_window_transparency(alpha);
#endif
                Fl_Gl_Window::draw();
                return;
            }

            _updateDevices();

            const auto& annotations =
                player->getAnnotations(p.ghostPrevious, p.ghostNext);

            MultilineInput* w = getMultilineInput();
            if (w)
            {
                std_any value;
                int font_size = App::app->settings()->getValue<int>(kFontSize);
                double pixels_unit = pixels_per_unit();
                double pct = renderSize.h / 1024.F;
                double fontSize = font_size * pct * p.viewZoom / pixels_unit;
                w->textsize(fontSize);
                math::Vector2i pos(w->pos.x, w->pos.y);
                w->Fl_Widget::position(pos.x, pos.y);
            }

            // Flag for FLTK's opengl1 text annotations drawings
            bool draw_opengl1 = false;

#ifdef USE_OPENGL2
            for (const auto& annotation : annotations)
            {
                for (const auto& shape : annotation->shapes)
                {
                    if (dynamic_cast<GL2TextShape*>(shape.get()))
                    {
                        draw_opengl1 = true;
                        break;
                    }
                }
                if (draw_opengl1 == true)
                    break;
            }
#endif

            const auto& currentTime = player->currentTime();

            if (gl.buffer && gl.shader)
            {
                math::Matrix4x4f mvp;

                const float rotation = _getRotation();
                if (p.presentation ||
                    p.ui->uiPrefs->uiPrefsBlitViewports->value() == kNoBlit ||
                    p.environmentMapOptions.type != EnvironmentMapOptions::kNone ||
                    rotation != 0.F || (transparent && hasAlpha))
                {
                    if (p.environmentMapOptions.type !=
                        EnvironmentMapOptions::kNone)
                    {
                        mvp = _createEnvironmentMap();
                    }
                    else
                    {
                        mvp = _createTexturedRectangle();
                    }

                    if (p.imageOptions[0].alphaBlend == timeline::AlphaBlend::kNone)
                    {
                        glDisable(GL_BLEND);
                    }

                    gl.shader->bind();
                    gl.shader->setUniform("transform.mvp", mvp);
#ifdef __APPLE__
                    gl.shader->setUniform("opacity", 1.0F);
                    set_window_transparency(alpha);
#else
                    if (desktop::Wayland())
                        gl.shader->setUniform("opacity", alpha);
                    else if (desktop::X11() || desktop::Windows())
                        gl.shader->setUniform("opacity", 1.0F);
#endif

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, gl.buffer->getColorID());

                    if (gl.vao && gl.vbo)
                    {
                        gl.vao->bind();
                        gl.vao->draw(GL_TRIANGLES, 0, gl.vbo->getSize());
                    }

                    if (p.stereo3DOptions.output == Stereo3DOutput::Glasses &&
                        p.stereo3DOptions.input == Stereo3DInput::Image)
                    {
                        gl.shader->bind();
                        gl.shader->setUniform("transform.mvp", mvp);
                        gl.shader->setUniform("opacity", alpha);

                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, gl.stereoBuffer->getColorID());

                        if (gl.vao && gl.vbo)
                        {
                            glDrawBuffer(GL_BACK_RIGHT);
                            gl.vao->bind();
                            gl.vao->draw(GL_TRIANGLES, 0, gl.vbo->getSize());
                        }
                    }

                    if (p.imageOptions[0].alphaBlend == timeline::AlphaBlend::kNone)
                    {
                        glEnable(GL_BLEND);
                    }
                }
                else
                {
                    if (annotations.empty())
                        mvp = _projectionMatrix();
                    else
                        mvp = _createTexturedRectangle();

                    const GLint viewportX = p.viewPos.x;
                    const GLint viewportY = p.viewPos.y;
                    const GLsizei sizeW = renderSize.w * p.viewZoom;
                    const GLsizei sizeH = renderSize.h * p.viewZoom;
                    if (sizeW > 0 && sizeH > 0)
                    {
                        glViewport(viewportX, viewportY, sizeW, sizeH);

                        glBindFramebuffer(GL_READ_FRAMEBUFFER, gl.buffer->getID());
                        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // 0 is screen

                        // Blit the offscreen buffer contents to the viewport
                        GLenum filter = GL_NEAREST;
                        if (!p.displayOptions.empty())
                        {
                            const auto& filters = p.displayOptions[0].imageFilters;
                            if (p.viewZoom < 1.0f &&
                                filters.minify == timeline::ImageFilter::Linear)
                                filter = GL_LINEAR;
                            else if (
                                p.viewZoom > 1.0f &&
                                filters.magnify == timeline::ImageFilter::Linear)
                                filter = GL_LINEAR;
                        }
                        glBlitFramebuffer(
                            0, 0, renderSize.w, renderSize.h, viewportX, viewportY,
                            sizeW + viewportX, sizeH + viewportY,
                            GL_COLOR_BUFFER_BIT, filter);

                        if (p.stereo3DOptions.output == Stereo3DOutput::Glasses &&
                            p.stereo3DOptions.input == Stereo3DInput::Image)
                        {
                            glBindFramebuffer(
                                GL_READ_FRAMEBUFFER, gl.stereoBuffer->getID());
                            glBindFramebuffer(
                                GL_DRAW_FRAMEBUFFER, 0); // 0 is screen
                            glDrawBuffer(GL_BACK_RIGHT);
                            glBlitFramebuffer(
                                0, 0, renderSize.w, renderSize.h, viewportX,
                                viewportY, sizeW + viewportX, sizeH + viewportY,
                                GL_COLOR_BUFFER_BIT, filter);
                        }

                        glViewport(
                            0, 0, GLsizei(viewportSize.w), GLsizei(viewportSize.h));
                    }
                }

                //
                // Draw annotations for output device in an overlay buffer.
                //
                auto outputDevice = App::app->outputDevice();
                if (outputDevice && gl.buffer && p.showAnnotations)
                {
                    gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                    if (!p.displayOptions.empty())
                    {
                        offscreenBufferOptions.colorFilters =
                            p.displayOptions[0].imageFilters;
                    }
                    offscreenBufferOptions.depth = gl::OffscreenDepth::kNone;
                    offscreenBufferOptions.stencil = gl::OffscreenStencil::kNone;
                    if (gl::doCreate(
                            gl.overlay, renderSize, offscreenBufferOptions))
                    {
                        gl.overlay = gl::OffscreenBuffer::create(
                            renderSize, offscreenBufferOptions);
                        CHECK_GL;
                        _createOverlayPBO(renderSize);
                        CHECK_GL;
                    }
                    CHECK_GL;

                    const math::Matrix4x4f& renderMVP = _renderProjectionMatrix();
                    _drawAnnotations(
                        gl.overlay, renderMVP, currentTime, annotations,
                        renderSize);
                    CHECK_GL;

                    // Copy data to PBO:
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.overlayPBO);
                    CHECK_GL;
                    glReadPixels(
                        0, 0, renderSize.w, renderSize.h, GL_RGBA, GL_UNSIGNED_BYTE,
                        nullptr);
                    CHECK_GL;
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                    CHECK_GL;

                    // Create a fence for the overlay PBO
                    gl.overlayFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
                    CHECK_GL;

                    // Wait for the fence to complete before compositing
                    GLenum waitReturn =
                        glClientWaitSync(gl.overlayFence, 0, GL_TIMEOUT_IGNORED);
                    CHECK_GL;
                    if (waitReturn == GL_TIMEOUT_EXPIRED)
                    {
                        LOG_ERROR("glClientWaitSync: Timeout occurred!");
                    }

                    math::Matrix4x4f overlayMVP;
                    _compositeOverlay(gl.overlay, overlayMVP, viewportSize);

                    if (!draw_opengl1)
                    {
                        outputDevice->setOverlay(gl.annotationImage);
                    }
                }

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

                    // // Sanity check
                    // const auto& renderSize = gl.buffer->getSize();
                    // if (selection.min.x >= renderSize.w)
                    //     selection.min.x = renderSize.w - 1;
                    // if (selection.min.y >= renderSize.h)
                    //     selection.min.y = renderSize.h - 1;
                    
                    // if (selection.max.x >= renderSize.w)
                    //     selection.max.x = renderSize.w - 1;
                    // if (selection.max.y >= renderSize.h)
                    //     selection.max.y = renderSize.h - 1;

            
                    // Copy it again in case it changed
                    p.colorAreaInfo.box = selection;

                    if (panel::colorAreaPanel || panel::histogramPanel ||
                        panel::vectorscopePanel)
                    {
                        _mapBuffer();

                        if (panel::colorAreaPanel)
                        {
                            _calculateColorArea(p.colorAreaInfo);
                            panel::colorAreaPanel->update(p.colorAreaInfo);
                        }
                        if (panel::histogramPanel)
                        {
                            panel::histogramPanel->update(p.colorAreaInfo);
                        }
                        if (panel::vectorscopePanel)
                        {
                            panel::vectorscopePanel->update(p.colorAreaInfo);
                        }
                    }
                    else
                    {
                        p.image = nullptr;
                    }
                }
                else
                {
                    p.image = nullptr;
                }

                // Update the pixel bar from here only if we are playing a movie
                // and one that is not 1 frames long.
                bool update = !_shouldUpdatePixelBar();
                if (update)
                    updatePixelBar();

                _unmapBuffer();

                update = _isPlaybackStopped() || _isSingleFrame();
                if (update)
                    updatePixelBar();

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
                }

                if (panel::annotationsPanel)
                {
                    panel::annotationsPanel->notes->value("");
                }

                if (p.showAnnotations && !annotations.empty())
                {
                    gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                    if (!p.displayOptions.empty())
                    {
                        offscreenBufferOptions.colorFilters =
                            p.displayOptions[0].imageFilters;
                    }
                    offscreenBufferOptions.depth = gl::OffscreenDepth::kNone;
                    offscreenBufferOptions.stencil = gl::OffscreenStencil::kNone;
                    if (gl::doCreate(
                            gl.annotation, viewportSize, offscreenBufferOptions))
                    {
                        gl.annotation = gl::OffscreenBuffer::create(
                            viewportSize, offscreenBufferOptions);
                    }

                    _drawAnnotations(
                        gl.annotation, mvp, currentTime, annotations, viewportSize);

                    const math::Matrix4x4f orthoMatrix = math::ortho(
                        0.F, static_cast<float>(renderSize.w), 0.F,
                        static_cast<float>(renderSize.h), -1.F, 1.F);
                    _compositeAnnotations(gl.annotation, orthoMatrix, viewportSize);
                }

                if (p.dataWindow)
                    _drawDataWindow();
                if (p.displayWindow)
                    _drawDisplayWindow();

                if (p.safeAreas)
                    _drawSafeAreas();

                if (p.actionMode != ActionMode::kScrub &&
                    p.actionMode != ActionMode::kText &&
                    p.actionMode != ActionMode::kSelection &&
                    p.actionMode != ActionMode::kRotate && Fl::belowmouse() == this)
                {
                    _drawCursor(mvp);
                }

                if (p.hudActive && p.hud != HudDisplay::kNone)
                    _drawHUD(alpha);

                if (!p.helpText.empty())
                    _drawHelpText();
            }

#ifdef USE_OPENGL2

            if (!draw_opengl1)
            {
                Fl_Gl_Window::draw();
                return;
            }

            // Set up 1:1 projection
            Fl_Gl_Window::draw_begin();

            // Draw FLTK children
            Fl_Window::draw();

            glViewport(0, 0, GLsizei(viewportSize.w), GLsizei(viewportSize.h));
            if (p.showAnnotations)
            {
                // Draw the text shape annotations to the viewport.
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glDrawBuffer(GL_BACK_LEFT);

                float pixel_unit = pixels_per_unit();
                math::Vector2f pos;
                pos.x = p.viewPos.x / pixel_unit;
                pos.y = p.viewPos.y / pixel_unit;
                math::Matrix4x4f vm = math::translate(math::Vector3f(
                                                          p.viewPos.x / pixel_unit, p.viewPos.y / pixel_unit, 0.F));
                vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));

                _drawGL1TextShapes(vm, p.viewZoom);

                // Create a new image with to read the gl.overlay composited
                // results.
                // This is the image we send to the outputDevice.
                const image::PixelType pixelType = image::PixelType::RGBA_U8;
                auto overlayImage =
                    image::Image::create(renderSize.w, renderSize.h, pixelType);
                auto outputDevice = App::app->outputDevice();
                if (outputDevice)
                {
                    math::Matrix4x4f vm;
                    float viewZoom = 1.0;
                    float scale = 1.F;

#    ifndef __APPLE__
                    // Now, draw the text shape annotations to the overlay frame
                    // buffer.  This accumulates the OpenGL3 drawings with the
                    // OpenGL1 text.
                    // On Apple, we render to a new OpenGL1 context and
                    // composite manually, but we cannot handle auto fit properly.
                    glBindFramebuffer(GL_FRAMEBUFFER, gl.overlay->getID());

                    if (!p.frameView)
                    {
                        const math::Size2i& deviceSize = outputDevice->getSize();
                        if (viewportSize.isValid() && deviceSize.isValid())
                        {
                            scale *=
                                deviceSize.w / static_cast<float>(viewportSize.w);
                        }
                        viewZoom = p.viewZoom * scale;
                        vm = math::translate(
                            math::Vector3f(pos.x * scale, pos.y * scale, 0.F));
                        vm = vm *
                             math::scale(math::Vector3f(viewZoom, viewZoom, 1.F));
                    }
                    _drawGL1TextShapes(vm, viewZoom);
                    // On Windows, X11, Wayland we can let the gfx card handle it.
                    glReadPixels(
                        0, 0, renderSize.w, renderSize.h, GL_RGBA, GL_UNSIGNED_BYTE,
                        overlayImage->getData());
#    else
                    // On Apple, we must do the composite ourselves.
                    // As this is extremaly expensive, we will only do
                    // it when playback is stopped.
                    if (_isPlaybackStopped())
                    {
                        glReadBuffer(GL_BACK_LEFT);

                        math::Vector2f pos;
                        math::Size2i tmpSize(
                            renderSize.w * p.viewZoom, renderSize.h * p.viewZoom);
                        if (!p.frameView)
                        {
                            pos = math::Vector2f(
                                p.viewPos.x * scale, p.viewPos.y * scale);
                        }
                        pos = _getRasterf(pos.x, pos.y);

                        //
                        //
                        //
                        auto tmp =
                            image::Image::create(tmpSize.w, tmpSize.h, pixelType);
                        tmp->zero();
                        glReadPixels(
                            pos.x, pos.y, tmpSize.w, tmpSize.h, GL_RGBA,
                            GL_UNSIGNED_BYTE, tmp->getData());

                        resizeImage(
                            overlayImage->getData(), tmp->getData(), tmpSize.w,
                            tmpSize.h, renderSize.w, renderSize.h);

                        // Composite the OpenGL3 annotations and the OpenGL1 text
                        // image into overlayImage.
                        GLubyte* source = gl.annotationImage->getData();
                        GLubyte* result = overlayImage->getData();
                        for (int y = 0; y < renderSize.h; ++y)
                        {
                            for (int x = 0; x < renderSize.w; ++x)
                            {
                                const float alpha = result[3] / 255.F;
                                result[0] =
                                    source[0] * (1.F - alpha) + result[0] * alpha;
                                result[1] =
                                    source[1] * (1.F - alpha) + result[1] * alpha;
                                result[2] =
                                    source[2] * (1.F - alpha) + result[2] * alpha;
                                result[3] =
                                    source[3] * (1.F - alpha) + result[3] * alpha;
                                source += 4;
                                result += 4;
                            }
                        }
                    }
#    endif
                    outputDevice->setOverlay(overlayImage);
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                }
            }
            Fl_Gl_Window::draw_end(); // Restore GL state
#else
            Fl_Gl_Window::draw();
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
            
            const uint32_t W = info.box.w();
            const uint32_t H = info.box.h();
            const size_t dataSize = W * H;
            
            image::Color4f rgba, hsv;

            for (int Y = info.box.y(); Y <= maxY; ++Y)
            {
                for (int X = info.box.x(); X <= maxX; ++X)
                {
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
                gl.currentPBOIndex = (gl.currentPBOIndex + 1) % 2;
                gl.nextPBOIndex = (gl.currentPBOIndex + 1) % 2;

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
                    CHECK_GL;
                    return;
                }
                else
                {

                    // read pixels from framebuffer to PBO
                    // glReadPixels() should return immediately.
                    glBindBuffer(
                        GL_PIXEL_PACK_BUFFER, gl.pboIDs[gl.currentPBOIndex]);
                    CHECK_GL;

                    glReadPixels(0, 0, renderSize.w, renderSize.h, format, type, 0);
                    CHECK_GL;

                    // map the PBO to process its data by CPU
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIDs[gl.nextPBOIndex]);
                    CHECK_GL;

                    // We are stopped, read the first PBO.
                    if (stopped)
                    {
                        glBindBuffer(
                            GL_PIXEL_PACK_BUFFER, gl.pboIDs[gl.currentPBOIndex]);
                        CHECK_GL;
                    }
                }

                if (p.rawImage)
                {
                    free(p.image);
                    p.image = nullptr;
                }

                gl.nextPBOIndex = (gl.currentPBOIndex + 1) % 2;

                // Wait for the fence of the PBO you're about to use:
                glClientWaitSync(
                    gl.pboFences[gl.nextPBOIndex], GL_SYNC_FLUSH_COMMANDS_BIT,
                    GL_TIMEOUT_IGNORED);
                CHECK_GL;
                glDeleteSync(gl.pboFences[gl.nextPBOIndex]);
                CHECK_GL;

                p.image = (float*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
                CHECK_GL;
                p.rawImage = false;
            }
            else
            {
                _unmapBuffer();
                TimelineViewport::_mapBuffer();
            }
        }

        void Viewport::_unmapBuffer() const noexcept
        {
            MRV2_GL();
            TLRENDER_P();

            if (p.image)
            {
                if (!p.rawImage)
                {
                    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

                    // Create a new fence
                    gl.pboFences[gl.nextPBOIndex] =
                        glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
                    p.image = nullptr;
                    p.rawImage = true;
                }
            }

            // back to conventional pixel operation
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        }

        void Viewport::_readPixel(image::Color4f& rgba) const noexcept
        {

            TLRENDER_P();
            MRV2_GL();

            math::Vector2i pos = _getRaster();

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
                {
                    return;
                }

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
                        Viewport* self = const_cast<Viewport*>(this);
                        self->make_current();
                        gl::OffscreenBufferBinding binding(gl.buffer);
                        glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, type, &rgba);
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
            else if (dynamic_cast< GLFilledRectangleShape* >(shape))
            {
                auto s = dynamic_cast< GLFilledRectangleShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< GLRectangleShape* >(shape))
            {
                auto s = dynamic_cast< GLRectangleShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< GLFilledCircleShape* >(shape))
            {
                auto s = dynamic_cast< GLFilledCircleShape* >(shape);
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
            else if (dynamic_cast< GLFilledPolygonShape* >(shape))
            {
                auto s = dynamic_cast< GLFilledPolygonShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< GLPolygonShape* >(shape))
            {
                auto s = dynamic_cast< GLPolygonShape* >(shape);
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

    }
    
} // namespace mrv

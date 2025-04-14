// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cinttypes>

#include <tlCore/FontSystem.h>
#include <tlCore/StringFormat.h>

#include <tlDevice/IOutput.h>

#include <tlVk/OffscreenBuffer.h>
#include <tlTimelineVk/RenderPrivate.h>
#include <tlVk/Init.h>
#include <tlVk/Util.h>

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

#include "mrvVk/mrvVkViewportPrivate.h"
#include "mrvVk/mrvVkDefines.h"
// #include "mrvVk/mrvVkErrors.h"
#include "mrvVk/mrvVkUtil.h"
#include "mrvVk/mrvVkShaders.h"
#include "mrvVk/mrvVkShape.h"
#include "mrvVk/mrvTimelineViewport.h"
#include "mrvVk/mrvTimelineViewportPrivate.h"
#include "mrvVk/mrvVkViewport.h"

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
        _vk(new VKPrivate)
    {
        int stereo = 0;
        int fl_double = FL_DOUBLE; // needed on Linux and _WIN32
#if defined(__APPLE__)
        // \bug: Do not use FL_DOUBLE on APPLE as it makes
        // playback slow
        fl_double = 0;
#endif

        mode(FL_RGB | fl_double | FL_ALPHA | FL_STENCIL | stereo);
    }

    Viewport::~Viewport() {}

    void Viewport::setContext(const std::weak_ptr<system::Context>& context)
    {
        _vk->context = context;
    }

    void Viewport::prepare()
    {
    }
    
    void Viewport::destroy_resources()
    {
        Fl_Vk_Window::destroy_resources();
    }
    
    //! Refresh window by clearing the associated resources.
    void Viewport::refresh()
    {
        TLRENDER_P();
        MRV2_VK();
        vk.render.reset();
        vk.outline.reset();
        vk.lines.reset();
#ifdef USE_ONE_PIXEL_LINES
        vk.outline.reset();
#endif
        vk.buffer.reset();
        vk.annotation.reset();
        vk.overlay.reset();
        vk.shader.reset();
        vk.annotationShader.reset();
        vk.vbo.reset();
        vk.vao.reset();
        p.fontSystem.reset();
        vk.currentPBOIndex = 0;
        vk.nextPBOIndex = 1;
    }

    void Viewport::_initializeVKResources()
    {
        TLRENDER_P();
        MRV2_VK();

        if (auto context = vk.context.lock())
        {

            vk.render = timeline_tk::Render::create(context);
            p.fontSystem = image::FontSystem::create(context);

#ifdef USE_ONE_PIXEL_LINES
            vk.outline = std::make_shared<vulkan::Outline>();
#endif

            vk.lines = std::make_shared<vulkan::Lines>();

            try
            {
                const std::string& vertexSource = timeline_vk::vertexSource();
                vk.shader =
                    vk::Shader::create(vertexSource, textureFragmentSource());
                vk.annotationShader = vk::Shader::create(
                    vertexSource, annotationFragmentSource());
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }
        }
    }

    void Viewport::_initializeVK()
    {
        MRV2_VK();

        refresh();

        _initializeVKResources();
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
        MRV2_VK();

        if (!valid())
        {
            _initializeVK();
            

            if (p.ui->uiPrefs->uiPrefsOpenGLVsync->value() ==
                MonitorVSync::kVSyncNone)
                swap_interval(0);
            else
                swap_interval(1);

            valid(1);
        }
        

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

        
        try
        {
            if (renderSize.isValid())
            {
                vk.colorBufferType = image::PixelType::RGBA_U8;
                int accuracy = p.ui->uiPrefs->uiPrefsColorAccuracy->value();
                switch (accuracy)
                {
                case kAccuracyFloat32:
                    vk.colorBufferType = image::PixelType::RGBA_F32;
                    hasAlpha = true;
                    break;
                case kAccuracyFloat16:
                    vk.colorBufferType = image::PixelType::RGBA_F16;
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
                            vk.colorBufferType = image::PixelType::RGBA_F32;
                            break;
                        case image::PixelType::RGBA_F16:
                        case image::PixelType::LA_F16:
                            hasAlpha = true;
                        case image::PixelType::RGB_F16:
                        case image::PixelType::L_F16:
                            vk.colorBufferType = image::PixelType::RGBA_F16;
                            break;
                        case image::PixelType::RGBA_U16:
                        case image::PixelType::LA_U16:
                            hasAlpha = true;
                        case image::PixelType::RGB_U16:
                        case image::PixelType::L_U16:
                            vk.colorBufferType = image::PixelType::RGBA_U16;
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

                
                vk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = vk.colorBufferType;

                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        p.displayOptions[0].imageFilters;
                }
                offscreenBufferOptions.depth = vk::OffscreenDepth::_24;
                offscreenBufferOptions.stencil = vk::OffscreenStencil::_8;
                
                if (vk::doCreate(vk.buffer, renderSize, offscreenBufferOptions))
                {
                    
                    vk.buffer = vk::OffscreenBuffer::create(
                        renderSize, offscreenBufferOptions);
                    
                    _createPBOs(renderSize);
                    
                }
                

                if (can_do(FL_STEREO))
                {
                    if (vk::doCreate(
                            vk.stereoBuffer, renderSize,
                            offscreenBufferOptions))
                    {
                        vk.stereoBuffer = vk::OffscreenBuffer::create(
                            renderSize, offscreenBufferOptions);
                    }
                }
                
            }
            else
            {
                vk.buffer.reset();
                vk.stereoBuffer.reset();
                
            }
            

            if (vk.buffer && vk.render)
            {

                if (p.pixelAspectRatio > 0.F && !p.videoData.empty() &&
                    !p.videoData[0].layers.empty())
                {
                    auto image = p.videoData[0].layers[0].image;
                    p.videoData[0].size.pixelAspectRatio = p.pixelAspectRatio;
                    image->setPixelAspectRatio(p.pixelAspectRatio);
                }

                if (p.stereo3DOptions.output == Stereo3DOutput::Vulkan &&
                    p.stereo3DOptions.input == Stereo3DInput::Image &&
                    p.videoData.size() > 1 && p.showVideo)
                {
                    _drawStereoVulkan();
                }
                else
                {
                    vk::OffscreenBufferBinding binding(vk.buffer);

                    locale::SetAndRestore saved;
                    timeline::RenderOptions renderOptions;
                    renderOptions.colorBuffer = vk.colorBufferType;

                    vk.render->begin(renderSize, renderOptions);

                    if (p.showVideo)
                    {
                        int screen = this->screen_num();
                        if (screen >= 0 && !p.monitorOCIOOptions.empty() &&
                            screen < p.monitorOCIOOptions.size())
                        {
                            timeline::OCIOOptions o = p.ocioOptions;
                            o.display = p.monitorOCIOOptions[screen].display;
                            o.view = p.monitorOCIOOptions[screen].view;
                            vk.render->setOCIOOptions(o);

                            _updateMonitorDisplayView(screen, o);
                        }
                        else
                        {
                            vk.render->setOCIOOptions(p.ocioOptions);
                            _updateMonitorDisplayView(screen, p.ocioOptions);
                        }

                        vk.render->setLUTOptions(p.lutOptions);
                        vk.render->setHDROptions(p.hdrOptions);
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
                                vk.render->drawVideo(
                                    p.videoData,
                                    timeline::getBoxes(
                                        p.compareOptions.mode, p.videoData),
                                    p.imageOptions, p.displayOptions,
                                    p.compareOptions, getBackgroundOptions());
                            }
                        }
                        _drawOverlays(renderSize);
                        vk.render->end();
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
            vk.buffer.reset();
            vk.stereoBuffer.reset();
        }
        

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

        
        // glDrawBuffer(GL_BACK_LEFT);
        

        // glViewport(0, 0, VKsizei(viewportSize.w), VKsizei(viewportSize.h));
        // glClearStencil(0);
        // glClearColor(r, g, b, a);
        // glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        

        const auto& player = getTimelinePlayer();
        if (!player)
        {
#ifdef __APPLE__
            set_window_transparency(alpha);
#endif
            Fl_Vk_Window::draw();
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

#ifdef USE_OPENVK2
        for (const auto& annotation : annotations)
        {
            for (const auto& shape : annotation->shapes)
            {
                if (dynamic_cast<VK2TextShape*>(shape.get()))
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

        if (vk.buffer && vk.shader)
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
                    //glDisable(GL_BLEND);
                }

                vk.shader->bind();
                vk.shader->setUniform("transform.mvp", mvp);
#ifdef __APPLE__
                vk.shader->setUniform("opacity", 1.0F);
                set_window_transparency(alpha);
#else
                if (desktop::Wayland())
                    vk.shader->setUniform("opacity", alpha);
                else if (desktop::X11() || desktop::Windows())
                    vk.shader->setUniform("opacity", 1.0F);
#endif

                //glActiveTexture(GL_TEXTURE0);
                //glBindTexture(GL_TEXTURE_2D, vk.buffer->getColorID());

                if (vk.vao && vk.vbo)
                {
                    vk.vao->bind();
                    // vk.vao->draw(GL_TRIANGLES, 0, vk.vbo->getSize());
                }

                if (p.stereo3DOptions.output == Stereo3DOutput::Vulkan &&
                    p.stereo3DOptions.input == Stereo3DInput::Image)
                {
                    vk.shader->bind();
                    vk.shader->setUniform("transform.mvp", mvp);
                    vk.shader->setUniform("opacity", alpha);

                    // glActiveTexture(GL_TEXTURE0);
                    // glBindTexture(GL_TEXTURE_2D, vk.stereoBuffer->getColorID());

                    if (vk.vao && vk.vbo)
                    {
                        vk.vao->bind();
                        // vk.vao->draw(GL_TRIANGLES, 0, vk.vbo->getSize());
                    }
                }

                if (p.imageOptions[0].alphaBlend == timeline::AlphaBlend::kNone)
                {
                    // glEnable(GL_BLEND);
                }
            }
            else
            {
                if (annotations.empty())
                    mvp = _projectionMatrix();
                else
                    mvp = _createTexturedRectangle();

                const uint32_t viewportX = p.viewPos.x;
                const uint32_t viewportY = p.viewPos.y;
                const size_t sizeW = renderSize.w * p.viewZoom;
                const size_t sizeH = renderSize.h * p.viewZoom;
                if (sizeW > 0 && sizeH > 0)
                {
                    // glViewport(viewportX, viewportY, sizeW, sizeH);

                    // glBindFramebuffer(GL_READ_FRAMEBUFFER, vk.buffer->getID());
                    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // 0 is screen

                    // Blit the offscreen buffer contents to the viewport
                    VkFilter filter = VK_FILTER_NEAREST;
                    if (!p.displayOptions.empty())
                    {
                        const auto& filters = p.displayOptions[0].imageFilters;
                        if (p.viewZoom < 1.0f &&
                            filters.minify == timeline::ImageFilter::Linear)
                            filter = VK_FILTER_LINEAR;
                        else if (
                            p.viewZoom > 1.0f &&
                            filters.magnify == timeline::ImageFilter::Linear)
                            filter = VK_FILTER_LINEAR;
                    }
                    // glBlitFramebuffer(
                    //     0, 0, renderSize.w, renderSize.h, viewportX, viewportY,
                    //     sizeW + viewportX, sizeH + viewportY,
                    //     GL_COLOR_BUFFER_BIT, filter);

                    // if (p.stereo3DOptions.output == Stereo3DOutput::Vulkan &&
                    //     p.stereo3DOptions.input == Stereo3DInput::Image)
                    // {
                    //     glBindFramebuffer(
                    //         GL_READ_FRAMEBUFFER, vk.stereoBuffer->getID());
                    //     glBindFramebuffer(
                    //         GL_DRAW_FRAMEBUFFER, 0); // 0 is screen
                    //     glDrawBuffer(VK_BACK_RIGHT);
                    //     glBlitFramebuffer(
                    //         0, 0, renderSize.w, renderSize.h, viewportX,
                    //         viewportY, sizeW + viewportX, sizeH + viewportY,
                    //         GL_COLOR_BUFFER_BIT, filter);
                    // }

                    // glViewport(
                    //     0, 0, GLsizei(viewportSize.w), GLsizei(viewportSize.h));
                }
            }

            //
            // Draw annotations for output device in an overlay buffer.
            //
            auto outputDevice = App::app->outputDevice();
            if (outputDevice && vk.buffer && p.showAnnotations)
            {
                vk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        p.displayOptions[0].imageFilters;
                }
                offscreenBufferOptions.depth = vk::OffscreenDepth::kNone;
                offscreenBufferOptions.stencil = vk::OffscreenStencil::kNone;
                if (vk::doCreate(
                        vk.overlay, renderSize, offscreenBufferOptions))
                {
                    vk.overlay = vk::OffscreenBuffer::create(
                        renderSize, offscreenBufferOptions);
                    
                    _createOverlayPBO(renderSize);
                    
                }
                

                const math::Matrix4x4f& renderMVP = _renderProjectionMatrix();
                _drawAnnotations(
                    vk.overlay, renderMVP, currentTime, annotations,
                    renderSize);
                

                // // Copy data to PBO:
                // glBindBuffer(GL_PIXEL_PACK_BUFFER, vk.overlayPBO);
                
                // glReadPixels(
                //     0, 0, renderSize.w, renderSize.h, GL_RGBA, GL_UNSIGNED_BYTE,
                //     nullptr);
                
                // glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                

                // Create a fence for the overlay PBO
                // vk.overlayFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
                

                // Wait for the fence to complete before compositing
                // VKenum waitReturn =
                //     glClientWaitSync(vk.overlayFence, 0, GL_TIMEOUT_IGNORED);
                
                // if (waitReturn == GL_TIMEOUT_EXPIRED)
                // {
                //     LOG_ERROR("glClientWaitSync: Timeout occurred!");
                // }

                math::Matrix4x4f overlayMVP;
                _compositeOverlay(vk.overlay, overlayMVP, viewportSize);

                if (!draw_opengl1)
                {
                    outputDevice->setOverlay(vk.annotationImage);
                }
            }

            // math::Box2i selection = p.colorAreaInfo.box = p.selection;
            // if (selection.max.x >= 0)
            // {
            //     // Check min < max
            //     if (selection.min.x > selection.max.x)
            //     {
            //         int tmp = selection.max.x;
            //         selection.max.x = selection.min.x;
            //         selection.min.x = tmp;
            //     }
            //     if (selection.min.y > selection.max.y)
            //     {
            //         int tmp = selection.max.y;
            //         selection.max.y = selection.min.y;
            //         selection.min.y = tmp;
            //     }
            //     // Copy it again in case it changed
            //     p.colorAreaInfo.box = selection;

            //     if (panel::colorAreaPanel || panel::histogramPanel ||
            //         panel::vectorscopePanel)
            //     {
            //         _mapBuffer();

            //         if (panel::colorAreaPanel)
            //         {
            //             _calculateColorArea(p.colorAreaInfo);
            //             panel::colorAreaPanel->update(p.colorAreaInfo);
            //         }
            //         if (panel::histogramPanel)
            //         {
            //             panel::histogramPanel->update(p.colorAreaInfo);
            //         }
            //         if (panel::vectorscopePanel)
            //         {
            //             panel::vectorscopePanel->update(p.colorAreaInfo);
            //         }
            //     }
            //     else
            //     {
            //         p.image = nullptr;
            //     }
            // }
            // else
            // {
            //     p.image = nullptr;
            // }

            // // Update the pixel bar from here only if we are playing a movie
            // // and one that is not 1 frames long.
            // bool update = !_shouldUpdatePixelBar();
            // if (update)
            //     updatePixelBar();

            // _unmapBuffer();

            // update = _isPlaybackStopped() || _isSingleFrame();
            // if (update)
            //     updatePixelBar();

            // if (p.selection.max.x >= 0)
            // {
            //     Fl_Color c = p.ui->uiPrefs->uiPrefsViewSelection->color();
            //     uint8_t r, g, b;
            //     Fl::get_color(c, r, g, b);

            //     const image::Color4f color(r / 255.F, g / 255.F, b / 255.F);

            //     math::Box2i selection = p.selection;
            //     if (selection.min == selection.max)
            //     {
            //         selection.max.x++;
            //         selection.max.y++;
            //     }
            //     _drawRectangleOutline(selection, color, mvp);
            // }

            if (panel::annotationsPanel)
            {
                panel::annotationsPanel->notes->value("");
            }

            if (p.showAnnotations && !annotations.empty())
            {
                vk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        p.displayOptions[0].imageFilters;
                }
                offscreenBufferOptions.depth = vk::OffscreenDepth::kNone;
                offscreenBufferOptions.stencil = vk::OffscreenStencil::kNone;
                if (vk::doCreate(
                        vk.annotation, viewportSize, offscreenBufferOptions))
                {
                    vk.annotation = vk::OffscreenBuffer::create(
                        viewportSize, offscreenBufferOptions);
                }

                _drawAnnotations(
                    vk.annotation, mvp, currentTime, annotations, viewportSize);

                const math::Matrix4x4f orthoMatrix = math::ortho(
                    0.F, static_cast<float>(renderSize.w), 0.F,
                    static_cast<float>(renderSize.h), -1.F, 1.F);
                _compositeAnnotations(vk.annotation, orthoMatrix, viewportSize);
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

#ifdef USE_OPENVK2

        if (!draw_opengl1)
        {
            Fl_Gl_Window::draw();
            return;
        }

        // Set up 1:1 projection
        VkWindow::draw_begin();

        // Draw FLTK children
        Fl_Window::draw();

        // glViewport(0, 0, VKsizei(viewportSize.w), VKsizei(viewportSize.h));
        if (p.showAnnotations)
        {
            // Draw the text shape annotations to the viewport.
            // glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // glDrawBuffer(GL_BACK_LEFT);

            float pixel_unit = pixels_per_unit();
            math::Vector2f pos;
            pos.x = p.viewPos.x / pixel_unit;
            pos.y = p.viewPos.y / pixel_unit;
            math::Matrix4x4f vm = math::translate(math::Vector3f(
                p.viewPos.x / pixel_unit, p.viewPos.y / pixel_unit, 0.F));
            vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));

            // _drawGL1TextShapes(vm, p.viewZoom);

            // Create a new image with to read the vk.overlay composited
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
                // buffer.  This accumulates the OpenVK3 drawings with the
                // OpenVK1 text.
                // On Apple, we render to a new OpenVK1 context and
                // composite manually, but we cannot handle auto fit properly.
                // glBindFramebuffer(GL_FRAMEBUFFER, vk.overlay->getID());

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
                // _drawGL1TextShapes(vm, viewZoom);
                // On Windows, X11, Wayland we can let the gfx card handle it.
                // glReadPixels(
                //     0, 0, renderSize.w, renderSize.h, GL_RGBA, GL_UNSIGNED_BYTE,
                //     overlayImage->getData());
#    else
                // On Apple, we must do the composite ourselves.
                // As this is extremaly expensive, we will only do
                // it when playback is stopped.
                if (_isPlaybackStopped())
                {
                    // glReadBuffer(GL_BACK_LEFT);

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
                    // glReadPixels(
                    //     pos.x, pos.y, tmpSize.w, tmpSize.h, GL_RGBA,
                    //     GL_UNSIGNED_BYTE, tmp->getData());

                    resizeImage(
                        overlayImage->getData(), tmp->getData(), tmpSize.w,
                        tmpSize.h, renderSize.w, renderSize.h);

                    // Composite the OpenVK3 annotations and the OpenVK1 text
                    // image into overlayImage.
                    VKubyte* source = vk.annotationImage->getData();
                    VKubyte* result = overlayImage->getData();
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
                // glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
        }
        VkWindow::draw_end(); // Restore VK state
#else
        VkWindow::draw();
#endif
    }

    void Viewport::_calculateColorAreaFullValues(area::Info& info) noexcept
    {
        TLRENDER_P();
        MRV2_VK();

        PixelToolBarClass* c = p.ui->uiPixelWindow;
        BrightnessType brightness_type = (BrightnessType)c->uiLType->value();
        int hsv_colorspace = c->uiBColorType->value() + 1;

        const int maxX = info.box.max.x;
        const int maxY = info.box.max.y;
        const auto& renderSize = vk.buffer->getSize();

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
        MRV2_VK();

        if (!p.image || !vk.buffer)
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
        MRV2_VK();
        TLRENDER_P();

        if (p.ui->uiPixelWindow->uiPixelValue->value() == PixelValue::kFull)
        {

            // For faster access, we muse use BGRA.
            // constexpr VKenum format = GL_BGRA;
            // constexpr VKenum type = GL_FLOAT;


            vk::OffscreenBufferBinding binding(vk.buffer);
            const auto& renderSize = vk.buffer->getSize();

            // bool update = _shouldUpdatePixelBar();
            bool stopped = _isPlaybackStopped();
            bool single_frame = _isSingleFrame();

            // set the target framebuffer to read
            // "index" is used to read pixels from framebuffer to a PBO
            // "nextIndex" is used to update pixels in the other PBO
            vk.currentPBOIndex = (vk.currentPBOIndex + 1) % 2;
            vk.nextPBOIndex = (vk.currentPBOIndex + 1) % 2;

        }
        else
        {
            TimelineViewport::_mapBuffer();
        }
    }

    void Viewport::_unmapBuffer() const noexcept
    {
        MRV2_VK();
        TLRENDER_P();

        if (p.image)
        {
            if (!p.rawImage)
            {
                // glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

                // // Create a new fence
                // vk.pboFences[vk.nextPBOIndex] =
                //     glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
                p.image = nullptr;
                p.rawImage = true;
            }
        }

        // back to conventional pixel operation
        // glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }

    void Viewport::_readPixel(image::Color4f& rgba) const noexcept
    {

        TLRENDER_P();
        MRV2_VK();

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
            if (!vk.buffer || !valid())
            {
                return;
            }

            // glPixelStorei(GL_PACK_ALIGNMENT, 1);
            // glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);

            // We use ReadPixels when the movie is stopped or has only a
            // a single frame.
            bool update = _shouldUpdatePixelBar();

            if (_isEnvironmentMap())
            {
                update = true;
            }

            // const VKenum type = GL_FLOAT;

            if (update)
            {
                _unmapBuffer();
                if (_isEnvironmentMap())
                {
                    pos = _getFocus();
                    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    // glReadBuffer(GL_FRONT);
                    // glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, type, &rgba);
                    return;
                }
                else
                {
                    // Viewport* self = const_cast<Viewport*>(this);
                    // self->make_current();
                    vk::OffscreenBufferBinding binding(vk.buffer);
                    //glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, type, &rgba);
                    return;
                }
            }

            if (!p.image)
                _mapBuffer();

            if (p.image)
            {
                const auto& renderSize = vk.buffer->getSize();
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
        if (dynamic_cast< VKArrowShape* >(shape))
        {
            auto s = dynamic_cast< VKArrowShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< VKFilledRectangleShape* >(shape))
        {
            auto s = dynamic_cast< VKFilledRectangleShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< VKRectangleShape* >(shape))
        {
            auto s = dynamic_cast< VKRectangleShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< VKFilledCircleShape* >(shape))
        {
            auto s = dynamic_cast< VKFilledCircleShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< VKCircleShape* >(shape))
        {
            auto s = dynamic_cast< VKCircleShape* >(shape);
            value = *s;
        }
#ifdef USE_OPENVK2
        else if (dynamic_cast< VK2TextShape* >(shape))
        {
            auto s = dynamic_cast< VK2TextShape* >(shape);
            value = *s;
        }
#else
        else if (dynamic_cast< VKTextShape* >(shape))
        {
            auto s = dynamic_cast< VKTextShape* >(shape);
            value = *s;
        }
#endif
        else if (dynamic_cast< draw::NoteShape* >(shape))
        {
            auto s = dynamic_cast< draw::NoteShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< VKFilledPolygonShape* >(shape))
        {
            auto s = dynamic_cast< VKFilledPolygonShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< VKPolygonShape* >(shape))
        {
            auto s = dynamic_cast< VKPolygonShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< VKErasePathShape* >(shape))
        {
            auto s = dynamic_cast< VKErasePathShape* >(shape);
            value = *s;
        }
        else if (dynamic_cast< VKPathShape* >(shape))
        {
            auto s = dynamic_cast< VKPathShape* >(shape);
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

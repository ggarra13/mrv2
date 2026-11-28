// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvApp/mrvSettingsObject.h"


// mrViewer .fl includes
#include "mrvWidgets/mrViewer.h"

#include "mrvCore/mrvColor.h"
#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvSequence.h"
#include "mrvOS/mrvI8N.h"


#include "mrvFl/mrvOCIO.h"
#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvViewport/mrvTimelineViewport.h"

#include "mrvWGPU/mrvWGPUViewportPrivate.h"
// #include "mrvWGPU/mrvWGPUDefines.h"
// #include "mrvWGPU/mrvWGPUErrors.h"
// #include "mrvWGPU/mrvWGPUUtil.h"
// #include "mrvWGPU/mrvWGPUShaders.h"
// #include "mrvWGPU/mrvWGPUShape.h"
#include "mrvWGPU/mrvWGPUViewport.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include <tlTimelineWGPU/RenderPrivate.h>
#include <tlTimelineWGPU/RenderStructs.h>

#include <tlWGPU/OffscreenBuffer.h>
#include <tlWGPU/Init.h>

#include <tlIO/WebCodecs.h>

#include <tlCore/FontSystem.h>
#include <tlCore/StringFormat.h>


namespace
{
    const char* kModule = "view";
}

namespace mrv
{
    namespace webgpu
    {

        static void pollPickCB(void* d)
        {
            static_cast<Viewport*>(d)->pollPick();
        }

        using namespace tl;

        Viewport::Viewport(int X, int Y, int W, int H, const char* L) :
            TimelineViewport(X, Y, W, H, L),
            _wgpu(new WGPUPrivate)
        {
            int stereo = 0;
            mode(FL_RGB | FL_DOUBLE | FL_ALPHA | FL_STENCIL | stereo);

            // Clear to dark gray so the quad stands out.
            clearValues = {0.1, 0.1, 0.1, 1.0};
        }

        Viewport::~Viewport() {}

        void Viewport::setContext(const std::weak_ptr<system::Context>& context)
        {
            _wgpu->context = context;
        }

        //! Refresh window by clearing the associated resources.
        void Viewport::refresh()
        {
            TLRENDER_P();
            MRV2_WGPU();
        }

        void Viewport::draw()
        {
            TLRENDER_P();
            MRV2_WGPU();

            const math::Size2i viewportSize = getViewportSize();
            math::Size2i renderSize = getRenderSize();
            end_render_pass();

            bool hasAlpha = false;
            const float alpha = p.ui->uiMain->get_alpha() / 255.F;
            if (alpha < 1.0F)
            {
                hasAlpha = true;
            }

            const bool transparent =
                hasAlpha || getBackgroundOptions().type ==
                                timeline::Background::Transparent;

            if (!renderSize.isValid())
            {
                wgpu.buffer.reset();
                wgpu.stereoBuffer.reset();
                return;
            }

            wgpu.colorBufferType = image::PixelType::RGBA_U8;

            int accuracy = p.ui->uiPrefs->ColorAccuracy->value();
            switch (accuracy)
            {
            case kAccuracyFloat32:
                wgpu.colorBufferType = image::PixelType::RGBA_F32;
                hasAlpha = true;
                break;
            case kAccuracyFloat16:
                wgpu.colorBufferType = image::PixelType::RGBA_F16;
                hasAlpha = true;
                break;
            case kAccuracyAuto:
                image::PixelType pixelType = image::PixelType::RGBA_U8;
                static auto emptyVideo = timeline::VideoFrame();
                timeline::VideoFrame& video = emptyVideo;
                if (!p.videoData.empty())
                    video = p.videoData[0];

                if (p.missingFrame &&
                    p.missingFrameType != tl::io::MissingFrames::Black)
                {
                    video = p.lastVideoFrame;
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
                        wgpu.colorBufferType = image::PixelType::RGBA_F32;
                        break;
                    case image::PixelType::RGBA_F16:
                    case image::PixelType::LA_F16:
                        hasAlpha = true;
                    case image::PixelType::RGB_F16:
                    case image::PixelType::L_F16:
                        wgpu.colorBufferType = image::PixelType::RGBA_F16;
                        break;
                    case image::PixelType::RGBA_U16:
                    case image::PixelType::LA_U16:
                        hasAlpha = true;
                    case image::PixelType::YUV_420P_U10:
                    case image::PixelType::YUV_422P_U10:
                    case image::PixelType::YUV_444P_U10:
                    case image::PixelType::YUV_420P_U12:
                    case image::PixelType::YUV_422P_U12:
                    case image::PixelType::YUV_444P_U12:
                    case image::PixelType::YUV_420P_U16:
                    case image::PixelType::YUV_422P_U16:
                    case image::PixelType::YUV_444P_U16:
                    case image::PixelType::YUV_420SP_U16:
                    case image::PixelType::YUV_422SP_U16:
                    case image::PixelType::YUV_444SP_U16:
                    case image::PixelType::RGB_U16:
                    case image::PixelType::L_U16:
                        wgpu.colorBufferType = image::PixelType::RGBA_U16;
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

            if (p.pixelAspectRatio > 0.F && !p.videoData.empty() &&
                !p.videoData.front().layers.empty())
            {
                auto image = p.videoData.front().layers.front().image;
                p.videoData.front().size.pixelAspectRatio = p.pixelAspectRatio;
                if (image) image->setPixelAspectRatio(p.pixelAspectRatio);
                renderSize = getRenderSize();
            }

            wgpu::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = wgpu.colorBufferType;

            if (!p.displayOptions.empty())
            {
                offscreenBufferOptions.colorFilters =
                    p.displayOptions[0].imageFilters;
            }
            offscreenBufferOptions.depth = wgpu::OffscreenDepth::_24;
            offscreenBufferOptions.stencil = wgpu::OffscreenStencil::_8;
            offscreenBufferOptions.pbo = true;

            if (wgpu::doCreate(
                    wgpu.buffer, renderSize, offscreenBufferOptions))
            {
                wgpu.buffer = wgpu::OffscreenBuffer::create(
                    ctx, renderSize, offscreenBufferOptions);
                wgpu.readPixels = false;

                // As render resolution might have changed,
                // we need to reset the quad size.
                wgpu.vbo.reset();
            }

            const auto& player = getTimelinePlayer();
            if (!wgpu.buffer || !player || p.videoData.empty())
            {
                return;
            }

            float r, g, b, a = 1.F;
            if (!p.presentation)
            {
                Fl_Color c = p.ui->uiPrefs->ViewBG->color();

                uint8_t ur = 0, ug = 0, ub = 0, ua = 0;
                Fl::get_color(c, ur, ug, ub, ua);

                r = ur / 255.0f;
                g = ug / 255.0f;
                b = ub / 255.0f;
                a = alpha;
            }
            else
            {
                r = g = b = 0.F; a = 1.F;
            }


            timeline::RenderOptions renderOptions;
            renderOptions.colorBuffer = wgpu.colorBufferType;
            renderOptions.vaoSize = 16 * memory::megabyte;

            wgpu.render->begin(
                ctx.encoder, wgpu.buffer, frameIndex, renderSize,
                renderOptions);
            wgpu.render->applyTransforms();

            if (p.showVideo)
            {
                auto ocio = p.ocioOptions;
                p.ocio_disabled = true;


                timeline::BackgroundOptions backgroundOptions = getBackgroundOptions();
                if (backgroundOptions.type == timeline::Background::Transparent)
                {
                    backgroundOptions.color0 = image::Color4f(r, g, b, 1.F);
                }
                wgpu.render->setShaderOptions(p.shaderOptions);
                wgpu.render->setLUTOptions(p.lutOptions);
                wgpu.render->setHDROptions(p.hdrOptions);
                if (p.missingFrame)
                {
                    //_drawMissingFrame(renderSize);
                }
                else
                {
                    if (p.stereo3DOptions.input == Stereo3DInput::Image &&
                        p.videoData.size() > 1)
                    {
                        //_drawStereo3D();
                    }
                    else
                    {
                        wgpu.render->drawVideo(
                            p.videoData,
                            timeline::getBoxes(
                                p.compareOptions,
                                p.displayOptions,
                                p.videoData),
                            p.imageOptions, p.displayOptions,
                            p.compareOptions, backgroundOptions);
                    }
                }
            }
            //    _drawOverlays(renderSize);

            wgpu.render->end();

            if (1)
            {
                begin_render_pass();

                math::Matrix4x4f mvp;
                mvp = _createTexturedRectangle();

                wgpu.shader->bind(frameIndex);
                wgpu.vao->beginFrame();     // rewind the vertex bump allocator

                const std::size_t offset = wgpu.vao->upload(wgpu.vbo->getData());

                timeline_wgpu::PushConstants pc;
                pc.mvp = mvp;
                pc.color = image::Color4f(1.F, 1.F, 0.F, 1.F);
                wgpu.shader->setFBO("textureSampler", wgpu.buffer);
                wgpu.shader->bindToPass(ctx.render_pass, &pc);

                wgpuRenderPassEncoderSetPipeline(ctx.render_pass,
                                                 wgpu.pipeline);
                wgpu.vao->draw(ctx.render_pass, wgpu.vbo->getSize(), offset);

                end_render_pass();
            }

            // Update the pixel bar from here only if we are playing a movie
            // and one that is not 1 frames long.
            bool update = wgpu.readPixels && !_shouldUpdatePixelBar();
            if (update)
                updatePixelBar();

            // After first run, we can read pixels.
            wgpu.readPixels = true;
        }

        void Viewport::prepare_shaders()
        {
            TLRENDER_P();
            MRV2_WGPU();

            if (auto context = wgpu.context.lock())
            {
                // Add the thumbnail system if not present.
                if (!context->getSystem<TIMELINEUI::ThumbnailSystem>())
                {
                    TIMELINEUI::ThumbnailSystem::create(context, ctx);
                }

                // Set the renderers's max nits
                if (!wgpu.render)
                    wgpu.render = timeline_wgpu::Render::create(ctx, context);


                p.fontSystem = image::FontSystem::create(context);

                // wgpu.lines = std::make_shared<webgpu::Lines>(ctx, nullptr);
                // wgpu.viewport = std::make_shared<webgpu::Lines>(ctx, ctx.render_pass);


                const timeline_wgpu::PushConstants pc;
                const wgpu::ShaderFlags flags =
                    static_cast<wgpu::ShaderFlags>(wgpu::kShaderVertex | wgpu::kShaderFragment);
                if (!wgpu.shader)
                {
                    wgpu.shader = wgpu::Shader::create(
                        ctx,
                        timeline_wgpu::vertexSource(),
                        timeline_wgpu::textureFragmentSource(),
                        "wgpu.shader");
                    timeline_wgpu::PushConstants pc;
                    wgpu.shader->addPush("pc", pc, flags);
                    wgpu.shader->addTexture("textureSampler");

                    wgpu.bindingSet = wgpu.shader->createBindingSet();
                    wgpu.shader->useBindingSet(wgpu.bindingSet);
                }
            }
        }

        void Viewport::destroy()
        {
            MRV2_WGPU();

            if (wgpu.pipeline)
            {
                wgpuRenderPipelineRelease(wgpu.pipeline);
                wgpu.pipeline = nullptr;
            }
            if (wgpu.annotation_pipeline)
            {
                wgpuRenderPipelineRelease(wgpu.annotation_pipeline);
                wgpu.annotation_pipeline = nullptr;
            }
            wgpu.bindingSet.reset();
            wgpu.vao.reset();
            wgpu.vbo.reset();
            wgpu.shader.reset();
        }

        void Viewport::prepare()
        {
            prepare_shaders();
        }

        int Viewport::handle(int event)
        {
            return TimelineViewport::handle(event);
        }

        std::shared_ptr<image::FontSystem > Viewport::getFontSystem()
        {
            return _p->fontSystem;
        }

        void Viewport::_calculateColorAreaFullValues(area::Info& info) noexcept
        {
        }

        void Viewport::_calculateColorArea(area::Info& info)
        {
        }

        void Viewport::_mapBuffer() const noexcept
        {
        }

        void Viewport::_unmapBuffer() const noexcept
        {
        }

        void Viewport::_readPixel(image::Color4f& rgba)
        {
            TLRENDER_P();
            MRV2_WGPU();

            math::Vector2i pos = _getRaster();
            p.gotReadPixel = false;

            if (p.ui->uiPixelValue->value() == PixelValue::kOriginal)
            {
                if (_isEnvironmentMap())
                    return;

                rgba.r = rgba.g = rgba.b = rgba.a = 0.f;

                for (const auto& video : p.videoData)
                {
                    for (const auto& layer : video.layers)
                    {
                        image::Color4f pixel, pixelB;


                        const auto& image = layer.image;
                        if (image && image->isValid())
                        {
                            _getPixelValue(pixel, image, pos);
                        }

                        const auto& imageB = layer.image;
                        if (imageB && imageB->isValid())
                        {
                            _getPixelValue(pixelB, imageB, pos);

                            if (layer.transition ==
                                timeline::Transition::Dissolve)
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

                p.gotReadPixel = true;
            }
            else
            {
                // This is needed as the FL_MOVE of fltk would get called
                // before the draw routine
                if (!wgpu.buffer)
                {
                    return;
                }

                _unmapBuffer();

                if (_isEnvironmentMap())
                {
                    pos = _getFocus();
                    return;
                }
                else
                {
                    if (wgpu.pickPending)
                        return;

                    WGPUCommandEncoderDescriptor encoderDesc = {};
                    encoderDesc.label = {
                        "Pixel Readback Encoder",
                        WGPU_STRLEN };

                    WGPUCommandEncoder cmd =
                        wgpuDeviceCreateCommandEncoder(ctx.device,
                                                       &encoderDesc);
                    wgpu.buffer->readPixels(cmd, pos.x,
                                            wgpu.buffer->getHeight() -
                                            pos.y, 1, 1);
                    wgpu.buffer->submitReadback(cmd);

                    wgpuCommandEncoderRelease(cmd);

                    wgpu.pickPending = true;

                    // returns to the browser first
                    Fl::add_timeout(0.0, pollPickCB, this);
                    return;
                }
            }
        }

        void Viewport::pollPick()
        {
            TLRENDER_P();
            MRV2_WGPU();

            void* data = nullptr;
            if (wgpu.buffer->getLatestReadPixels(data) && data)
            {
                const auto options = wgpu.buffer->getOptions();
                image::Color4f rgba = color::fromVoidPtr(data,
                                                         options.colorType);
                p.gotReadPixel = false;
                wgpu.pickPending = false;

                _updatePixelBar(rgba);
                return;
            }
            // Not ready yet: try again next tick.
            Fl::repeat_timeout(1.0 / 60.0, pollPickCB, this);
        }

        void Viewport::prepare_pipeline()
        {
            MRV2_WGPU();

            if (wgpu.pipeline)
                return;

            try
            {
                // --- Pipeline ----------------------------------------------
                wgpu::PipelineCreationState state;
                state.label = "Main Viewport compositing pipeline";

                wgpu::ShaderStageInfo vs;
                vs.name = "quad vs";
                vs.module = wgpu.shader->getVertex();
                vs.entryPoint = wgpu.shader->getVertexEntryPoint();
                state.vertexStage = vs;

                wgpu::ShaderStageInfo fs;
                fs.name = "quad fs";
                fs.module = wgpu.shader->getFragment();
                fs.entryPoint = wgpu.shader->getFragmentEntryPoint();
                state.fragmentStage = fs;

                // Take the layout straight from the VBO so it always matches
                // what the VAO draws.
                const WGPUVertexBufferLayout& vl = wgpu.vbo->getVertexLayout();
                wgpu::VertexBufferLayoutInfo buf;
                buf.arrayStride = vl.arrayStride;
                buf.stepMode = vl.stepMode;
                buf.attributes = wgpu.vbo->getAttributes();
                wgpu::VertexInputStateInfo vin;
                vin.buffers.push_back(buf);
                state.vertexInputState = vin;

                wgpu::ColorTargetStateInfo target;
                target.format = ctx.surface_format;
                state.colorTargets.push_back(target);

                // The window's main pass has a depth attachment when the mode
                // includes FL_DEPTH/FL_STENCIL: the pipeline must match it.
                if (ctx.depth_format != WGPUTextureFormat_Undefined)
                {
                    wgpu::DepthStencilStateInfo ds;
                    ds.format = ctx.depth_format;
                    ds.depthWriteEnabled = WGPUOptionalBool_False;
                    ds.depthCompare = WGPUCompareFunction_Always;
                    state.depthStencilState = ds;
                }

                state.layout = wgpu.shader->getPipelineLayout();

                wgpu.pipeline = state.create(ctx.device);
            }
            catch (const std::exception& e)
            {
                std::fprintf(stderr, "FAIL: prepare(): %s\n", e.what());
                destroy();
            }
        }

        void Viewport::_pushAnnotationShape(const std::string& command) const
        {
#ifdef MRV2_NETWORK
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
            if (dynamic_cast< WGPUArrowShape* >(shape))
            {
                auto s = dynamic_cast< WGPUArrowShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< WGPUFilledRectanwgpueShape* >(shape))
            {
                auto s = dynamic_cast< WGPUFilledRectanwgpueShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< WGPURectanwgpueShape* >(shape))
            {
                auto s = dynamic_cast< WGPURectanwgpueShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< WGPUFilledCircleShape* >(shape))
            {
                auto s = dynamic_cast< WGPUFilledCircleShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< WGPUCircleShape* >(shape))
            {
                auto s = dynamic_cast< WGPUCircleShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< WGPUTextShape* >(shape))
            {
                auto s = dynamic_cast< WGPUTextShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< draw::NoteShape* >(shape))
            {
                auto s = dynamic_cast< draw::NoteShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< WGPUFilledPolygonShape* >(shape))
            {
                auto s = dynamic_cast< WGPUFilledPolygonShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< WGPUPolygonShape* >(shape))
            {
                auto s = dynamic_cast< WGPUPolygonShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< WGPUErasePathShape* >(shape))
            {
                auto s = dynamic_cast< WGPUErasePathShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< WGPUPathShape* >(shape))
            {
                auto s = dynamic_cast< WGPUPathShape* >(shape);
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
#endif

        }
    }
} // namespace mrv

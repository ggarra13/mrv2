// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvFl/mrvSaveOptions.h"
#include "mrvFl/mrvSave.h"
#include "mrvFl/mrvIO.h"

#include "mrvGL/mrvGLErrors.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvWidgets/mrvProgressReport.h"

#include "mrvCore/mrvImage.h"
#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvMath.h"
#include "mrvCore/mrvUtil.h"

#include <tlIO/System.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlVk/Init.h>
#include <tlVk/Util.h>
#include <tlTimelineVk/Render.h>
#include <FL/Fl_Vk_Utils.H>
#include <FL/vk_enum_string_helper.h>

#include <chrono>
#include <string>
#include <sstream>

namespace
{
    const char* kModule = "save";
}

namespace mrv
{
    
    int _save_single_frame(
        std::string file, const ViewerUI* ui, SaveOptions options,
        const int32_t frameIndex)
    {
        int ret = 0;
        std::string msg;
        MyViewport* view = ui->uiView;

        file::Path path(file);

        // Time range.
        auto player = view->getTimelinePlayer();
        if (!player)
            return -1; // should never happen

        // Stop the playback
        player->stop();
        
        std::shared_ptr<image::Image> tmpImage;

        auto currentTime = player->currentTime();

        const std::string& extension = path.getExtension();

        bool saveEXR = string::compare(
            extension, ".exr", string::Compare::CaseInsensitive);
        bool saveHDR = string::compare(
            extension, ".hdr", string::Compare::CaseInsensitive);

        try
        {

            tl::io::Options ioOptions;

            char buf[256];

#ifdef TLRENDER_EXR
            ioOptions["OpenEXR/Compression"] =
                tl::string::Format("{0}").arg(options.exrCompression);
            snprintf(buf, 256, "%d", options.zipCompressionLevel);
            ioOptions["OpenEXR/ZipCompressionLevel"] = buf;
            {
                std::stringstream s;
                s << options.dwaCompressionLevel;
                ioOptions["OpenEXR/DWACompressionLevel"] = s.str();
            }
#endif

            otime::TimeRange oneFrameTimeRange(
                currentTime, otime::RationalTime(1, currentTime.rate()));

            auto context = ui->app->getContext();
            auto timeline = player->timeline();

            // Render information.
            const auto& info = player->ioInfo();
            if (info.video.empty())
            {
                throw std::runtime_error("No video information");
            }

            vlk::OffscreenBufferOptions offscreenBufferOptions;
            std::shared_ptr<timeline_vlk::Render> render;

            image::Size renderSize;

            int layerId = ui->uiColorChannel->value();
            if (layerId < 0)
                layerId = 0;

            const SaveResolution resolution = options.resolution;
            {
                auto compareSize = ui->uiView->getRenderSize();
                if (!options.annotations || compareSize.w == 0 ||
                    compareSize.h == 0)
                {
                    renderSize = info.video[layerId].size;
                }
                else
                {
                    renderSize.w = compareSize.w;
                    renderSize.h = compareSize.h;
                }
                // \@todo: rotated images not yet supported.
                
                // auto rotation = view->getRotation();
                // if (options.annotations && rotationSign(rotation) != 0)
                // {
                //     size_t tmp = renderSize.w;
                //     renderSize.w = renderSize.h;
                //     renderSize.h = tmp;

                //     msg = tl::string::Format(_("Rotated image info: {0}"))
                //               .arg(renderSize);
                //     LOG_STATUS(msg);
                // }
                if (resolution == SaveResolution::kHalfSize)
                {
                    renderSize.w /= 2;
                    renderSize.h /= 2;
                    msg = tl::string::Format(_("Scaled image info: {0}"))
                              .arg(renderSize);
                    LOG_STATUS(msg);
                }
                else if (resolution == SaveResolution::kQuarterSize)
                {
                    renderSize.w /= 4;
                    renderSize.h /= 4;
                    msg = tl::string::Format(_("Scaled image info: {0}"))
                              .arg(renderSize);
                    LOG_STATUS(msg);
                }
                msg = tl::string::Format(_("Render size: {0}"))
                      .arg(renderSize);
                LOG_STATUS(msg);
            }

                    
            // Create the renderer.
            auto ctx = ui->uiView->getContext();
            render = timeline_vlk::Render::create(ctx, context);
            
            offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
            offscreenBufferOptions.pbo = true;

            // Create the writer.
            auto writerPlugin =
                context->getSystem<io::System>()->getPlugin(path);

            if (!writerPlugin)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open writer plugin.")
                        .arg(file));
            }
            
            auto tags = ui->uiView->getTags();

            io::Info ioInfo;
            image::Info outputInfo;

            outputInfo.size = renderSize;
            std::shared_ptr<image::Image> outputImage;
            std::shared_ptr<image::Image> annotationImage;

            outputInfo.pixelType = info.video[layerId].pixelType;
            outputInfo.size.pixelAspectRatio = 1.0;

            math::Box2i displayWindow(0, 0, renderSize.w, renderSize.h);
            math::Box2i dataWindow(0, 0, renderSize.w, renderSize.h);
            if (!options.annotations && saveEXR)
            {
                auto i = tags.find("Data Window");
                if (i != tags.end())
                {
                    std::stringstream s(i->second);
                    s >> dataWindow;
                }
                i = tags.find("Display Window");
                if (i != tags.end())
                {
                    std::stringstream s(i->second);
                    s >> displayWindow;
                }
                if (options.exrSaveContents == SaveContents::kDisplayWindow)
                    dataWindow = displayWindow;
                
                outputInfo.size.w = dataWindow.max.x - dataWindow.min.x + 1;
                outputInfo.size.h = dataWindow.max.y - dataWindow.min.y + 1;
            }

            {

                if (options.annotations)
                {
                    view->setShowVideo(options.video);
                    view->setActionMode(ActionMode::kScrub);
                    view->redraw();
                    // flush is needed
                    Fl::flush();
                    view->flush();

                    view->setFrameView(true);
                    view->centerView();
                    view->redraw();
                    // flush is needed
                    Fl::flush();
                }
            }

            if (!options.annotations)
            {
                std::string layerName = ui->uiColorChannel->label();
                outputInfo.pixelType = info.video[layerId].pixelType;
                msg = tl::string::Format(_("Image Layer '{0}' info: {1} {2}"))
                      .arg(layerName)
                      .arg(outputInfo.size)
                      .arg(outputInfo.pixelType);
                LOG_STATUS(msg);
            }
            else
            {
                auto buffer = view->getVideoFBO();
                outputInfo.pixelType = buffer->getOptions().colorType;
            }
            
            outputInfo = writerPlugin->getWriteInfo(outputInfo);
            if (image::PixelType::kNone == outputInfo.pixelType)
            {
                outputInfo.pixelType = image::PixelType::RGBA_U8;
                offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
#ifdef TLRENDER_EXR
                if (saveEXR)
                {
                    offscreenBufferOptions.colorType =
                        options.exrPixelType;
                }
#endif
                msg = tl::string::Format(
                          _("Writer plugin did not get output info.  "
                            "Defaulting to {0}"))
                          .arg(offscreenBufferOptions.colorType);
                LOG_STATUS(msg);
            }

            if (saveHDR)
            {
                outputInfo.pixelType = image::PixelType::RGB_F32;
                offscreenBufferOptions.colorType = image::PixelType::RGB_F32;
            }
            
#ifdef TLRENDER_EXR
            if (saveEXR)
            {
                outputInfo.pixelType = options.exrPixelType;
                offscreenBufferOptions.colorType =
                    outputInfo.pixelType;
            }
#endif

            std::string msg = tl::string::Format(_("Output info: {0} {1}"))
                                  .arg(outputInfo.size)
                                  .arg(outputInfo.pixelType);
            LOG_STATUS(msg);

#ifdef TLRENDER_EXR
            ioOptions["OpenEXR/PixelType"] = getLabel(outputInfo.pixelType);
#endif
            outputImage = image::Image::create(outputInfo);
            
            outputInfo.pixelType = image::PixelType::RGBA_U8;
            
            ioInfo.video.push_back(outputInfo);
            ioInfo.videoTime = oneFrameTimeRange;

            auto writer = writerPlugin->write(path, ioInfo, ioOptions);
            if (!writer)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(file));
            }

            // Don't send any tcp updates
            tcp->lock();

            // Turn off hud so it does not get captured by glReadPixels.
            view->setHudActive(false);

            if (options.annotations)
            {
                view->setSaveOverlay(true);
                
                // Refresh the view (so we turn off the HUD, for example).
                view->redraw();
                view->flush();

                // Flush the image
                Fl::flush();
            
                auto buffer = view->getVideoFBO();
                auto bufferOptions = buffer->getOptions();
                {
                    std::string msg =
                        tl::string::Format(_("Offscreen Buffer info: {0}"))
                        .arg(bufferOptions.colorType);
                    LOG_STATUS(msg);
                }
                
                auto overlayBuffer = view->getAnnotationFBO();

                VkDevice device = view->device();
                VkCommandPool commandPool = view->commandPool();

                // Read Main Image Viewport
                VkCommandBuffer cmd = beginSingleTimeCommands(device,
                                                              commandPool);
            
                size_t width = buffer->getWidth();
                size_t height = buffer->getHeight();
                
                auto tempImageInfo = outputInfo;
                tempImageInfo.size.w = width;
                tempImageInfo.size.h = height;

                if (width != outputInfo.size.w ||
                    height != outputInfo.size.h)
                {
                    tmpImage = image::Image::create(tempImageInfo);
                }
                else
                {
                    tmpImage = outputImage;
                }
                

                buffer->readPixels(cmd, 0, 0, width, height);
            
                vkEndCommandBuffer(cmd);
            
                buffer->submitReadback(cmd);

                view->wait_queue();
            
                const void* data = buffer->getLatestReadPixels();
                if (!data)
                {
                    LOG_ERROR(_("Could not read image data from view"));
                }
                
                std::memcpy(tmpImage->getData(), data,
                            tmpImage->getDataByteCount());

                //
                // Read Annotation Image
                //                
                switch(outputInfo.pixelType)
                {
                default:
                case image::PixelType::RGBA_U8:
                    flipImageInY<uint8_t>(
                        reinterpret_cast<uint8_t*>(
                            tmpImage->getData()), width, height, 4);
                    break;
                case image::PixelType::RGB_U16:
                    flipImageInY<uint16_t>(
                        reinterpret_cast<uint16_t*>(
                            tmpImage->getData()), width, height, 3);
                    break;
                case image::PixelType::RGBA_U16:
                    flipImageInY<uint16_t>(
                        reinterpret_cast<uint16_t*>(
                            tmpImage->getData()), width, height, 4);
                    break;
                case image::PixelType::RGB_F16:
                    flipImageInY<half>(
                        reinterpret_cast<half*>(
                            tmpImage->getData()), width, height, 3);
                    break;
                case image::PixelType::RGBA_F16:
                    flipImageInY<half>(
                        reinterpret_cast<half*>(
                            tmpImage->getData()), width, height, 4);
                    break;
                case image::PixelType::RGB_F32:
                    flipImageInY<float>(
                        reinterpret_cast<float*>(
                            tmpImage->getData()), width, height, 3);
                    break;
                case image::PixelType::RGBA_F32:
                    flipImageInY<float>(
                        reinterpret_cast<float*>(
                            tmpImage->getData()), width, height, 4);
                    break;
                }
                
                annotationImage = image::Image::create(tempImageInfo);
                        
                if (overlayBuffer)
                {
                    VkCommandBuffer cmd = beginSingleTimeCommands(device,
                                                                  commandPool);
                
                    width = overlayBuffer->getWidth();
                    height = overlayBuffer->getHeight();

                    overlayBuffer->readPixels(cmd, 0, 0, width, height);
            
                    vkEndCommandBuffer(cmd);
            
                    overlayBuffer->submitReadback(cmd);

                    view->wait_queue();
            
                    data = overlayBuffer->getLatestReadPixels();
                    if (!data)
                    {
                        LOG_ERROR(_("Could not read annotation image from view"));
                    }
                            
                    std::memcpy(annotationImage->getData(), data,
                                annotationImage->getDataByteCount());
                }
                else
                {
                    annotationImage->zero();
                }
                
                //
                // Composite output
                //
                switch(outputInfo.pixelType)
                {
                case image::PixelType::RGB_U8:
                    compositeImageOverNoAlpha<uint8_t>(
                        reinterpret_cast<uint8_t*>(
                            tmpImage->getData()),
                        annotationImage->getData(),
                        width, height);
                    break;
                case image::PixelType::RGB_U16:
                    compositeImageOverNoAlpha<uint16_t>(
                        reinterpret_cast<uint16_t*>(
                            tmpImage->getData()),
                        annotationImage->getData(),
                        width, height);
                    break;
                case image::PixelType::RGBA_U16:
                    compositeImageOver<uint16_t>(
                        reinterpret_cast<uint16_t*>(
                            tmpImage->getData()),
                        annotationImage->getData(),
                        width, height);
                    break;
                case image::PixelType::RGB_F16:
                    compositeImageOverNoAlpha<half>(
                        reinterpret_cast<half*>(
                            tmpImage->getData()),
                        annotationImage->getData(),
                        width, height);
                    break;
                case image::PixelType::RGBA_F16:
                    compositeImageOver<half>(
                        reinterpret_cast<half*>(
                            tmpImage->getData()),
                        annotationImage->getData(),
                        width, height);
                    break;
                case image::PixelType::RGB_F32:
                    compositeImageOverNoAlpha<float>(
                        reinterpret_cast<float*>(
                            tmpImage->getData()),
                        annotationImage->getData(),
                        width, height);
                    break;
                case image::PixelType::RGBA_F32:
                    compositeImageOver<float>(
                        reinterpret_cast<float*>(
                            tmpImage->getData()),
                        annotationImage->getData(),
                        width, height);
                    break;
                default:
                case image::PixelType::RGBA_U8:
                    compositeImageOver<uint8_t>(
                        reinterpret_cast<uint8_t*>(
                            tmpImage->getData()),
                        annotationImage->getData(),
                        width, height);
                    break;
                }
                
                vkFreeCommandBuffers(device, commandPool, 1, &cmd);
                        
                view->setSaveOverlay(false);
                
            }
            else
            {

                math::Size2i offscreenBufferSize(renderSize.w, renderSize.h);
                
                auto buffer = vlk::OffscreenBuffer::create(
                    ctx, offscreenBufferSize, offscreenBufferOptions);
            
                // Get the videoData
                auto videoData = timeline->getVideo(currentTime).future.get();
                videoData.layers[0].image->setPixelAspectRatio(1.F);

                VkDevice device = ctx.device;
                VkCommandPool commandPool = ctx.commandPool;
                
                VkCommandBuffer cmd = beginSingleTimeCommands(device, commandPool);
                buffer->transitionToColorAttachment(cmd);

                {
                    locale::SetAndRestore saved;
                    timeline::RenderOptions renderOptions;
                    renderOptions.clear = false;
                    render->begin(cmd, buffer, frameIndex, offscreenBufferSize,
                                  renderOptions);
                    const math::Matrix4x4f ortho = math::ortho(
                        0.F, static_cast<float>(renderSize.w),
                        static_cast<float>(renderSize.h), 0.F,
                        -1.F, 1.F);
                    render->setTransform(ortho);
                    render->setOCIOOptions(view->getOCIOOptions());
                    render->setLUTOptions(view->lutOptions());
                    
                    render->drawVideo(
                        {videoData},
                        {math::Box2i(0, 0, renderSize.w, renderSize.h)},
                        {timeline::ImageOptions()},
                        {timeline::DisplayOptions()},
                        timeline::CompareOptions(),
                        ui->uiView->getBackgroundOptions());
                    
                    render->end();
                }

                // Read back the image
                
                buffer->transitionToColorAttachment(cmd);
                
                buffer->readPixels(cmd, 0, 0, renderSize.w, renderSize.h);
                                    
                vkEndCommandBuffer(cmd);
                
                buffer->submitReadback(cmd);

                {
                    std::lock_guard<std::mutex> lock(ctx.queue_mutex());
                    vkQueueWaitIdle(ctx.queue());
                }
                                                    
                void* imageData = buffer->getLatestReadPixels();
                if (imageData)
                {
                    std::memcpy(outputImage->getData(), imageData,
                                outputImage->getDataByteCount());
                                    
                    vkFreeCommandBuffers(device, commandPool, 1, &cmd);    
                }

                // Scale down result.
                if (tmpImage && outputImage != tmpImage)
                {
                    scaleImageLinear(tmpImage->getData(), tmpImage->getWidth(),
                                     tmpImage->getHeight(),
                                     outputImage->getData(),
                                     outputImage->getWidth(),
                                     outputImage->getHeight(),
                                     image::getChannelCount(outputImage->getPixelType()));
                }
            }

            outputImage->setTags(tags);
            writer->writeVideo(currentTime, outputImage);            
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
            ret = -1;
        }
        return ret;
    }

    int save_multiple_frames(
        const std::string& file, const std::vector<otime::RationalTime>& times,
        const ViewerUI* ui, SaveOptions options)
    {
        int ret = 0;
        MyViewport* view = ui->uiView;
        bool presentation = view->getPresentationMode();
        bool hud = view->getHudActive();

        auto player = view->getTimelinePlayer();
        if (!player)
            return -1;

        file::Path path(file);

        if (file::isTemporaryEDL(path))
        {
            LOG_ERROR(_("Cannot save an NDI stream"));
            return -1;
        }

        int32_t frameIndex = 0;
        for (const auto& time : times)
        {
            player->seek(time);
            waitForFrame(player, time);

            _save_single_frame(file, ui, options, frameIndex);

            frameIndex = (frameIndex + 1) % vlk::MAX_FRAMES_IN_FLIGHT;
        }

        view->setFrameView(ui->uiPrefs->uiPrefsAutoFitImage->value());
        view->setHudActive(hud);
        view->setShowVideo(true);
        tcp->unlock();

        auto settings = ui->app->settings();
        if (file::isReadable(file))
        {
            settings->addRecentFile(file);
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }
        return ret;
    }

    int save_multiple_annotation_frames(
        const std::string& file, const std::vector<otime::RationalTime>& times,
        const ViewerUI* ui, SaveOptions options)
    {
        int ret = 0;
        MyViewport* view = ui->uiView;
        bool presentation = view->getPresentationMode();
        bool hud = view->getHudActive();

        auto player = view->getTimelinePlayer();
        if (!player)
            return -1;

        file::Path path(file);

        if (file::isTemporaryEDL(path))
        {
            LOG_ERROR(_("Cannot save an NDI stream"));
            return -1;
        }

        int32_t frameIndex = 0;
        for (const auto& time : times)
        {
            player->seek(time);
            waitForFrame(player, time);

            _save_single_frame(file, ui, options, frameIndex);
        }

        view->setFrameView(ui->uiPrefs->uiPrefsAutoFitImage->value());
        view->setHudActive(hud);
        view->setShowVideo(true);
        view->setPresentationMode(presentation);
        tcp->unlock();

        auto settings = ui->app->settings();
        if (file::isReadable(file))
        {
            settings->addRecentFile(file);
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }
        return ret;
    }

    int save_single_frame(
        const std::string& file, const ViewerUI* ui, SaveOptions options)
    {
        std::string msg;

        int ret = 0;
        MyViewport* view = ui->uiView;
        bool presentation = view->getPresentationMode();
        bool hud = view->getHudActive();

        file::Path path(file);

        if (file::isTemporaryEDL(path))
        {
            LOG_ERROR(_("Cannot save an NDI stream"));
            return -1;
        }

        // Time range.
        auto player = view->getTimelinePlayer();
        if (!player)
            return -1; // should never happen

        // Stop the playback
        player->stop();

        const auto& currentTime = player->currentTime(); 

        // Save this frame with the frame number
        player->seek(currentTime);
        waitForFrame(player, currentTime);
            
        _save_single_frame(file, ui, options, 0);

        Fl::wait(1);
        
        // Rename file name that got saved with a frame number to the actual
        // frame that the user set.
        int64_t currentFrame = currentTime.to_frames();
        char filename[4096];
        snprintf(
            filename, 4096, "%s%s%0*" PRId64 "%s", path.getDirectory().c_str(),
            path.getBaseName().c_str(), static_cast<int>(path.getPadding()),
            currentFrame, path.getExtension().c_str());

        if (filename != file && !options.noRename)
        {
            try
            {
                fs::rename(fs::path(filename), fs::path(file));
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(e.what());
            }
        }

        view->setFrameView(ui->uiPrefs->uiPrefsAutoFitImage->value());
        view->setHudActive(hud);
        view->setShowVideo(true);
        view->setPresentationMode(presentation);
        tcp->unlock();

        auto settings = ui->app->settings();
        if (file::isReadable(file))
        {
            settings->addRecentFile(file);
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }
        return ret;
    }

} // namespace mrv

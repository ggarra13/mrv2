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

#include "mrvCore/mrvImageOps.h"
#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvMath.h"
#include "mrvCore/mrvUtil.h"


#include <tlGL/Init.h>
#include <tlGL/Util.h>
#include <tlGL/GLFWWindow.h>
#include <tlTimelineGL/Render.h>

#include <tlIO/System.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

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

            gl::OffscreenBufferOptions offscreenBufferOptions;
            std::shared_ptr<timeline_gl::Render> render;

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
                auto rotation = ui->uiView->getRotation();
                if (options.annotations && rotationSign(rotation) != 0)
                {
                    size_t tmp = renderSize.w;
                    renderSize.w = renderSize.h;
                    renderSize.h = tmp;

                    msg = tl::string::Format(_("Rotated image info: {0}"))
                              .arg(renderSize);
                    LOG_STATUS(msg);
                }
                if (resolution == SaveResolution::kHalfSize)
                {
                    renderSize.w /= 2;
                    renderSize.h /= 2;
                }
                else if (resolution == SaveResolution::kQuarterSize)
                {
                    renderSize.w /= 4;
                    renderSize.h /= 4;
                }
                /* xgettext:c++-format */
                msg = tl::string::Format(_("Render size: {0}"))
                      .arg(renderSize);
                LOG_STATUS(msg);
            }

                    
            // Create the renderer.
            render = timeline_gl::Render::create(context);
            offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;

            // Create the writer.
            auto writerPlugin =
                context->getSystem<io::System>()->getPlugin(path);

            if (!writerPlugin)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open writer plugin.")
                        .arg(file));
            }

            int X = 0, Y = 0;

            io::Info ioInfo;
            image::Info outputInfo;
            outputInfo.size = renderSize;
            
            auto tags = ui->uiView->getTags();


            std::shared_ptr<image::Image> outputImage;

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
                    view->setPresentationMode(true);
                    view->redraw();
                    // flush is needed
                    Fl::flush();
                    view->flush();
                    Fl::check();

                    // returns pixel_w(), pixel_h()
                    auto viewportSize = view->getViewportSize();
                    const float pixels_unit = view->pixels_per_unit();
                    viewportSize.w /= pixels_unit;
                    viewportSize.h /= pixels_unit;

                    if (viewportSize.w >= renderSize.w &&
                        viewportSize.h >= renderSize.h)
                    {
                        view->setFrameView(false);
#ifdef __APPLE__
                        view->setViewZoom(pixels_unit);
#else
                        view->setViewZoom(1.0);
#endif
                        view->centerView();
                        view->redraw();
                        // flush is needed
                        Fl::flush();

                        outputInfo.size = renderSize;
                    }
                    else
                    {
                        LOG_WARNING(_("Image too big for Save Annotations.  "
                                      "Will scale to the viewport size."));

                        view->frameView();

                        float aspectImage =
                            static_cast<float>(renderSize.w) / renderSize.h;
                        float aspectViewport =
                            static_cast<float>(viewportSize.w) / viewportSize.h;

                        if (aspectImage > aspectViewport)
                        {
                            // Fit to width
                            outputInfo.size.w = viewportSize.w;
                            outputInfo.size.h = viewportSize.w / aspectImage;
                        }
                        else
                        {
                            // Fit to height
                            outputInfo.size.h = viewportSize.h;
                            outputInfo.size.w = viewportSize.h * aspectImage;
                        }
                    }

                    X = std::max(0, (viewportSize.w - outputInfo.size.w) / 2);
                    Y = std::max(0, (viewportSize.h - outputInfo.size.h) / 2);

                    outputInfo.size.w = std::round(outputInfo.size.w);
                    outputInfo.size.h = std::round(outputInfo.size.h);

                    /* xgettext:c++-format */
                    msg = tl::string::Format(_("Viewport Size: {0} - "
                                               "X={1}, Y={2}"))
                              .arg(viewportSize)
                              .arg(X)
                              .arg(Y);
                    LOG_STATUS(msg);
                }
            }

#ifdef __APPLE__
            if (options.annotations)
            {
                switch (outputInfo.pixelType)
                {
                case image::PixelType::RGB_F16:
                case image::PixelType::RGBA_F16:
                    outputInfo.pixelType = image::PixelType::RGBA_F16;
                    break;
                case image::PixelType::RGB_F32:
                case image::PixelType::RGBA_F32:
                    outputInfo.pixelType = image::PixelType::RGBA_F32;
                    break;
                default:
                    if (saveHDR)
                        outputInfo.pixelType = image::PixelType::RGBA_F32;
                    else if (saveEXR)
                        outputInfo.pixelType = image::PixelType::RGBA_F16;
                    else
                        outputInfo.pixelType = image::PixelType::RGBA_U8;
                    break;
                }
            }
#endif

            if (!options.annotations)
            {
                std::string layerName = ui->uiColorChannel->label();
                outputInfo.pixelType = info.video[layerId].pixelType;
                /* xgettext:c++-format */
                msg = tl::string::Format(_("Image Layer '{0}' info: {1} {2}"))
                      .arg(layerName)
                      .arg(outputInfo.size)
                      .arg(outputInfo.pixelType);
                LOG_STATUS(msg);
            }
            
            outputInfo = writerPlugin->getWriteInfo(outputInfo);
            if (image::PixelType::kNone == outputInfo.pixelType)
            {
                outputInfo.pixelType = image::PixelType::RGB_U8;
                offscreenBufferOptions.colorType = image::PixelType::RGB_U8;

#ifdef TLRENDER_EXR
                if (saveEXR)
                {
                    offscreenBufferOptions.colorType =
                        image::PixelType::RGB_F32;
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
                if (options.annotations)
                {
                    outputInfo.pixelType = options.exrPixelType;
                }
                offscreenBufferOptions.colorType = outputInfo.pixelType;
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

#ifdef OPENGL_BACKEND
            const GLenum format = gl::getReadPixelsFormat(outputInfo.pixelType);
            const GLenum type = gl::getReadPixelsType(outputInfo.pixelType);
            if (GL_NONE == format || GL_NONE == type)
            {
                /* xgettext:c++-format */
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(file));
            }
#endif
            {
                /* xgettext:c++-format */
                const std::string msg =
                    tl::string::Format(_("Offscreen Buffer info: {0}"))
                                      .arg(offscreenBufferOptions.colorType);
                LOG_STATUS(msg);
            }

            // Turn off hud so it does not get captured by glReadPixels.
            view->setHudActive(false);

            math::Size2i offscreenBufferSize(renderSize.w, renderSize.h);

            view->make_current();
            gl::initGLAD();
            auto buffer = gl::OffscreenBuffer::create(
                offscreenBufferSize, offscreenBufferOptions);

            if (options.annotations)
            {
                // Refresh the view (so we turn off the HUD, for example).
                view->redraw();
                view->flush();
                Fl::flush();

#ifdef __APPLE__
                Fl_RGB_Image* tmp = fl_capture_window(
                    view, X, Y, outputInfo.size.w, outputInfo.size.h);

                Fl_Image* rgb = tmp->copy(outputInfo.size.w, outputInfo.size.h);
                tmp->alloc_array = 1;
                delete tmp;

                // Access the first pointer in the data array
                const char* const* data = rgb->data();

                // Flip image in Y
                switch (outputImage->getPixelType())
                {
                case image::PixelType::RGBA_U8:
                    flipImageInY(
                        (uint8_t*)outputImage->getData(),
                        (const uint8_t*)data[0], rgb->w(), rgb->h(), rgb->d(),
                        4);
                    break;
                case image::PixelType::RGBA_F16:
                    flipImageInY(
                        (Imath::half*)outputImage->getData(),
                        (const uint8_t*)data[0], rgb->w(), rgb->h(), rgb->d(),
                        4);
                    break;
                case image::PixelType::RGBA_F32:
                    flipImageInY(
                        (float*)outputImage->getData(), (const uint8_t*)data[0],
                        rgb->w(), rgb->h(), rgb->d(), 4);
                    break;
                default:
                    LOG_ERROR(
                        _("Unsupported output format: ")
                        << outputImage->getPixelType());
                    break;
                }

                delete rgb;
#else

                GLenum imageBuffer = GL_FRONT;

                // @note: Wayland does not work like Windows, macOS or
                //        X11.  The compositor does not immediately
                //        swap buffers when calling view->flush().
                if (desktop::Wayland())
                {
                    imageBuffer = GL_BACK;
                }

                glReadBuffer(imageBuffer);

                glPixelStorei(GL_PACK_ALIGNMENT, 1);

                
                glReadPixels(
                    X, Y, outputInfo.size.w, outputInfo.size.h, format, type,
                    outputImage->getData());
#endif
                
            }
            else
            {
                // Get the videoData
                auto videoData = timeline->getVideo(currentTime).future.get();
                videoData.layers[0].image->setPixelAspectRatio(1.F);

                // Render the video.
                gl::OffscreenBufferBinding binding(buffer);
                {
                    locale::SetAndRestore saved;
                    render->begin(offscreenBufferSize);
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
                
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                

                glPixelStorei(GL_PACK_ALIGNMENT, outputInfo.layout.alignment);
                
                glPixelStorei(
                    GL_PACK_SWAP_BYTES,
                    outputInfo.layout.endian != memory::getEndian());

                X = dataWindow.min.x;

                //
                // OpenGL Framebuffer:
                // Origin (0,0) is at the bottom-left, and Y increases upwards.
                // However, when reading pixels with glReadPixels, the image
                // data is stored top-down.
                //
                Y = displayWindow.max.y - (dataWindow.min.y +
                                           outputImage->getHeight()) + 1;

                glReadPixels(X, Y, outputImage->getWidth(),
                             outputImage->getHeight(), format, type,
                             outputImage->getData());

                
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

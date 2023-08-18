// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <sstream>
#include <algorithm>

#include <tlIO/IOSystem.h>
#include <tlIO/FFmpeg.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlGL/Util.h>
#include <tlTimeline/GLRender.h>

#include <tlGlad/gl.h>

#include "mrvCore/mrvUtil.h"

#include "mrvWidgets/mrvProgressReport.h"

#include "mrvGL/mrvGLErrors.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvFl/mrvSaveOptions.h"
#include "mrvFl/mrvIO.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "save";
}

namespace mrv
{

    void
    save_movie(const std::string& file, const ViewerUI* ui, SaveOptions options)
    {
        try
        {

            tl::io::Options ioOptions;

            char buf[256];

            ioOptions["FFmpeg/WriteProfile"] = getLabel(options.ffmpegProfile);

            ioOptions["OpenEXR/Compression"] = getLabel(options.exrCompression);

            ioOptions["OpenEXR/PixelType"] = getLabel(options.exrPixelType);

            std::cerr << "openexr/pixeltype=" << options.exrPixelType
                      << std::endl;

            snprintf(buf, 256, "%d", options.zipCompressionLevel);
            ioOptions["OpenEXR/ZipCompressionLevel"] = buf;

            {
                std::stringstream s;
                s << options.dwaCompressionLevel;
                ioOptions["OpenEXR/DWACompressionLevel"] = s.str();
            }

            Viewport* view = ui->uiView;

            auto player = view->getTimelinePlayer();
            if (!player)
                return; // should never happen

            // Stop the playback
            player->stop();

            // Time range.
            auto timeRange = player->inOutRange();
            auto startTime = timeRange.start_time();
            auto endTime = timeRange.end_time_inclusive();
            auto currentTime = startTime;

            auto context = ui->app->getContext();
            auto timeline = player->timeline();

            // Render information.
            const auto& info = player->ioInfo();
            if (info.video.empty())
            {
                throw std::runtime_error("No video information");
            }

            auto renderSize = info.video[0].size;

            const std::string& originalFile = player->path().get();
            if (originalFile == file)
            {
                throw std::runtime_error(
                    string::Format("{0}: Saving over same file being played!")
                        .arg(file));
            }

            file::Path path(file);
            const std::string& extension = path.getExtension();

            // Create the renderer.
            auto render = timeline::GLRender::create(context);

            gl::OffscreenBufferOptions offscreenBufferOptions;

            offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;

            // Create the writer.
            auto writerPlugin =
                context->getSystem<io::System>()->getPlugin(path);

            if (!writerPlugin)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(file));
            }

            bool annotations = false;
            if (options.annotations)
            {
                annotations = true;
            }

            writerPlugin->setOptions(ioOptions);

            int X = 0, Y = 0;
            bool presentation = view->getPresentationMode();

            if (annotations)
            {
                view->setActionMode(ActionMode::kScrub);
                view->setPresentationMode(true);
                view->redraw();
                // flush is needed
                Fl::flush();
                view->flush();
                Fl::check();
                const auto& viewportSize = view->getViewportSize();
                if (viewportSize.w >= renderSize.w &&
                    viewportSize.h >= renderSize.h)
                {
                    view->setFrameView(false);
                    view->setViewZoom(1.0);
                    view->centerView();
                    view->redraw();
                    // flush is needed
                    Fl::flush();

                    X = (viewportSize.w - renderSize.w) / 2;
                    Y = (viewportSize.h - renderSize.h) / 2;
                }
                else
                {
                    view->frameView();
                    renderSize.w = viewportSize.w;
                    renderSize.h = viewportSize.h;
                    LOG_WARNING(_("Image too big.  "
                                  "Will save the viewport size."));
                }

                std::string msg = tl::string::Format(
                                      _("Viewport Size: {0}  Render Size: {1}"))
                                      .arg(viewportSize)
                                      .arg(renderSize);
                LOG_INFO(msg);

                msg = tl::string::Format("viewZoom: {2} X: {3} Y: {4}")
                          .arg(view->viewZoom())
                          .arg(X)
                          .arg(Y);
                LOG_INFO(msg);
            }

            image::Info outputInfo;
            outputInfo.size = renderSize;
            outputInfo.pixelType = info.video[0].pixelType;

            {
                std::string msg = tl::string::Format(_("Image info: {0} {1}"))
                                      .arg(outputInfo.size)
                                      .arg(outputInfo.pixelType);
                LOG_INFO(msg);
            }

            outputInfo = writerPlugin->getWriteInfo(outputInfo);
            if (image::PixelType::None == outputInfo.pixelType)
            {
                outputInfo.pixelType = image::PixelType::RGB_U8;
            }
            if (annotations &&
                string::compare(
                    extension, ".exr", string::Compare::CaseInsensitive))
            {
                outputInfo.pixelType = options.exrPixelType;
            }
            std::string msg = tl::string::Format(_("Output info: {0} {1}"))
                                  .arg(outputInfo.size)
                                  .arg(outputInfo.pixelType);
            LOG_INFO(msg);

            auto outputImage = image::Image::create(outputInfo);

            io::Info ioInfo;
            ioInfo.video.push_back(outputInfo);
            ioInfo.videoTime = timeRange;

            auto writer = writerPlugin->write(path, ioInfo);
            if (!writer)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(file));
            }

            int64_t startFrame = startTime.to_frames();
            int64_t endFrame = endTime.to_frames();

            ProgressReport progress(ui->uiMain, startFrame, endFrame);
            progress.show();

            bool running = true;

            // Don't send any tcp updates
            tcp->lock();

            const GLenum format = gl::getReadPixelsFormat(outputInfo.pixelType);
            const GLenum type = gl::getReadPixelsType(outputInfo.pixelType);
            if (GL_NONE == format || GL_NONE == type)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(file));
            }

            TimelineClass* c = ui->uiTimeWindow;

            player->start();

            // Use inner player
            auto innerPlayer = player->player();

            // Turn off hud so it does not get captured by glReadPixels.
            bool hud = view->getHudActive();
            view->setHudActive(false);

            auto buffer =
                gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);
            CHECK_GL;

            try
            {
                while (running)
                {

                    if (annotations)
                    {
                        view->redraw();
                        view->flush();
                        Fl::check();

                        // If progress window is closed, exit loop.
                        if (!progress.tick())
                            break;

                        glReadBuffer(GL_FRONT);
                        CHECK_GL;
                        glReadPixels(
                            X, Y, outputInfo.size.w, outputInfo.size.h, format,
                            type, outputImage->getData());
                        CHECK_GL;
                    }
                    else
                    {
                        // Get the videoData
                        const auto& videoData =
                            timeline->getVideo(currentTime).get();

                        // If progress window is closed, exit loop.
                        if (!progress.tick())
                            break;

                        // This updates Viewport display
                        view->make_current();
                        CHECK_GL;
                        view->currentVideoCallback(videoData, player);
                        CHECK_GL;
                        view->flush();
                        CHECK_GL;

                        // Render the video.
                        gl::OffscreenBufferBinding binding(buffer);
                        CHECK_GL;
                        const std::string savedLocale =
                            std::setlocale(LC_NUMERIC, nullptr);
                        setlocale(LC_NUMERIC, "C");
                        render->begin(
                            renderSize, view->getColorConfigOptions(),
                            view->lutOptions());
                        CHECK_GL;
                        render->drawVideo(
                            {videoData},
                            {math::Box2i(0, 0, renderSize.w, renderSize.h)});
                        CHECK_GL;
                        render->end();
                        std::setlocale(LC_NUMERIC, savedLocale.c_str());

                        // back to conventional pixel operation
                        // glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                        // CHECK_GL;
                        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                        CHECK_GL;

                        glPixelStorei(
                            GL_PACK_ALIGNMENT, outputInfo.layout.alignment);
                        CHECK_GL;
                        glPixelStorei(
                            GL_PACK_SWAP_BYTES,
                            outputInfo.layout.endian != memory::getEndian());
                        CHECK_GL;

                        glReadPixels(
                            0, 0, outputInfo.size.w, outputInfo.size.h, format,
                            type, outputImage->getData());
                        CHECK_GL;
                    }

                    writer->writeVideo(currentTime, outputImage);

                    // We need to use frameNext instead of seeking as large
                    // movies can lag behind the seek
                    if (annotations)
                        player->frameNext();

                    currentTime += otime::RationalTime(1, currentTime.rate());
                    if (currentTime > endTime)
                    {
                        running = false;
                    }
                }
            }
            catch (std::exception& e)
            {
                LOG_ERROR(e.what());
            }

            player->seek(currentTime);
            view->setFrameView(ui->uiPrefs->uiPrefsAutoFitImage->value());
            view->setHudActive(hud);
            view->setPresentationMode(presentation);
            tcp->unlock();
        }
        catch (std::exception& e)
        {
            LOG_ERROR(e.what());
        }
    }

} // namespace mrv

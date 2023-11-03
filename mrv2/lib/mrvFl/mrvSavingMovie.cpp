// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <sstream>

#include <tlIO/System.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlGL/Init.h>
#include <tlGL/Util.h>
#include <tlTimeline/GLRender.h>

#include "mrvCore/mrvUtil.h"
#include "mrvCore/mrvLocale.h"

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
        Viewport* view = ui->uiView;
        bool presentation = view->getPresentationMode();

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

        auto mute = player->isMuted();
        player->setMute(true);

        try
        {

            tl::io::Options ioOptions;

#ifdef TLRENDER_FFMPEG
            ioOptions["FFmpeg/WriteProfile"] = getLabel(options.ffmpegProfile);
            ioOptions["FFmpeg/AudioCodec"] = getLabel(options.ffmpegAudioCodec);
#endif

#ifdef TLRENDER_EXR
            ioOptions["OpenEXR/Compression"] = getLabel(options.exrCompression);
            ioOptions["OpenEXR/PixelType"] = getLabel(options.exrPixelType);
            {
                std::stringstream s;
                s << options.zipCompressionLevel;
                ioOptions["OpenEXR/ZipCompressionLevel"] = s.str();
            }
            {
                std::stringstream s;
                s << options.dwaCompressionLevel;
                ioOptions["OpenEXR/DWACompressionLevel"] = s.str();
            }
#endif

            auto context = ui->app->getContext();
            auto timeline = player->timeline();

            auto startTimeOpt = timeline->getTimeline()->global_start_time();
            if (startTime.value() > 0.0 || startTimeOpt.has_value())
            {
                std::stringstream s;
                std::string timecode = startTime.to_timecode();
                if (timecode.empty() && startTimeOpt.has_value())
                {
                    timecode = startTimeOpt.value().to_timecode();
                }
                if (!timecode.empty())
                {
                    s << timecode;
                    ioOptions["timecode"] = s.str();
                }
            }

            // Render information.
            const auto& info = player->ioInfo();

            auto videoTime = info.videoTime;

            const bool hasVideo = !info.video.empty();

            if (player->timeRange() != timeRange)
            {
                double videoRate = info.videoTime.duration().rate();
                videoTime = otime::TimeRange(
                    timeRange.start_time().rescaled_to(videoRate),
                    timeRange.duration().rescaled_to(videoRate));
            }

            auto audioTime = time::invalidTimeRange;
            const double sampleRate = info.audio.sampleRate;
            const bool hasAudio = info.audio.isValid();
            if (hasAudio)
            {
                audioTime = info.audioTime;
                if (player->timeRange() != timeRange)
                {
                    audioTime = otime::TimeRange(
                        timeRange.start_time().rescaled_to(sampleRate),
                        timeRange.duration().rescaled_to(sampleRate));
                }
            }

            if (time::compareExact(videoTime, time::invalidTimeRange))
                videoTime = audioTime;

            const size_t endAudioSampleCount =
                endTime.rescaled_to(sampleRate).value();
            const size_t maxAudioSampleCount =
                timeRange.duration().rescaled_to(sampleRate).value();

            const std::string& originalFile = player->path().get();
            if (originalFile == file)
            {
                throw std::runtime_error(
                    string::Format("{0}: Saving over same file being played!")
                        .arg(file));
            }

            file::Path path(file);
            const std::string& extension = path.getExtension();

            bool annotations = false;

            gl::OffscreenBufferOptions offscreenBufferOptions;
            std::shared_ptr<timeline::GLRender> render;
            image::Size renderSize;
            int layerId = 0;
            if (options.annotations)
            {
                annotations = true;
                layerId = ui->uiColorChannel->value();
            }

            if (hasVideo)
                renderSize = info.video[layerId].size;
            else
                renderSize = image::Size(1, 1);

            // Create the renderer.
            render = timeline::GLRender::create(context);
            offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;

            // Create the writer.
            auto writerPlugin =
                context->getSystem<io::System>()->getPlugin(path);

            if (!writerPlugin)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(file));
            }

            int X = 0, Y = 0;

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

            io::Info ioInfo;
            image::Info outputInfo;

            outputInfo.size = renderSize;
            if (hasVideo)
                outputInfo.pixelType = info.video[layerId].pixelType;
            else
                outputInfo.pixelType = image::PixelType::RGB_U8;

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

#ifdef TLRENDER_EXR
            if (annotations &&
                string::compare(
                    extension, ".exr", string::Compare::CaseInsensitive))
            {
                outputInfo.pixelType = options.exrPixelType;
            }
#endif
            if (string::compare(
                    extension, ".hdr", string::Compare::CaseInsensitive))
            {
                outputInfo.pixelType = image::PixelType::RGB_F32;
                offscreenBufferOptions.colorType = image::PixelType::RGB_F32;
            }

            std::string msg = tl::string::Format(_("Output info: {0} {1}"))
                                  .arg(outputInfo.size)
                                  .arg(outputInfo.pixelType);
            LOG_INFO(msg);

            auto outputImage = image::Image::create(outputInfo);

            ioInfo.videoTime = videoTime;
            ioInfo.video.push_back(outputInfo);

            if (hasAudio)
            {
                ioInfo.audio = info.audio;
                ioInfo.audioTime = audioTime;
            }

            auto writer = writerPlugin->write(path, ioInfo, ioOptions);
            if (!writer)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(file));
            }

            int64_t startFrame = startTime.to_frames();
            int64_t endFrame = endTime.to_frames();

            char title[1024];

            if (hasVideo)
            {
                if (static_cast<ffmpeg::AudioCodec>(options.ffmpegAudioCodec) ==
                    ffmpeg::AudioCodec::None)
                    snprintf(
                        title, 1024, _("Saving Movie %" PRId64 " - %" PRId64),
                        startFrame, endFrame);
                else
                    snprintf(
                        title, 1024,
                        _("Saving Movie with Audio %" PRId64 " - %" PRId64),
                        startFrame, endFrame);
            }
            else
            {
                snprintf(
                    title, 1024, _("Saving Audio %" PRId64 " - %" PRId64),
                    startFrame, endFrame);
            }

            ProgressReport progress(ui->uiMain, startFrame, endFrame, title);
            progress.show();

            bool running = true;

            // Don't send any tcp updates
            tcp->lock();

            GLenum format = gl::getReadPixelsFormat(outputInfo.pixelType);
            GLenum type = gl::getReadPixelsType(outputInfo.pixelType);
            if (GL_NONE == format || GL_NONE == type)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(file));
            }

            player->start();

            // Turn off hud so it does not get captured by glReadPixels.
            bool hud = view->getHudActive();
            view->setHudActive(false);

            std::shared_ptr<gl::OffscreenBuffer> buffer;

            math::Size2i offscreenBufferSize(renderSize.w, renderSize.h);
            view->make_current();
            gl::initGLAD();
            buffer = gl::OffscreenBuffer::create(
                offscreenBufferSize, offscreenBufferOptions);

            size_t totalSamples = 0;
            size_t currentSampleCount =
                startTime.rescaled_to(sampleRate).value();

            while (running)
            {
                // If progress window is closed, exit loop.
                if (!progress.tick())
                    break;

                if (hasAudio)
                {
                    const double seconds = currentTime.to_seconds();
                    const auto audioData = timeline->getAudio(seconds).get();
                    if (!audioData.layers.empty())
                    {
                        bool skip = false;
                        otime::TimeRange range;

                        if (hasVideo)
                            range = otime::TimeRange(
                                currentTime,
                                otime::RationalTime(1.0, currentTime.rate()));
                        else
                            range = otime::TimeRange(
                                currentTime,
                                otime::RationalTime(
                                    currentTime.rate(), currentTime.rate()));
                        auto audio = audioData.layers[0].audio;

                        // \todo mix audio layers (or have a function in
                        // timeline to do it).
                        const auto currentAudioTime =
                            currentTime.rescaled_to(sampleRate);

                        // timeline->getAudio() returns one second of audio.
                        // Clamp to end of the timeRange/inOutRange.
                        if (currentAudioTime.value() >= currentSampleCount)
                        {
                            const size_t sampleCount = audio->getSampleCount();
                            if (currentSampleCount + sampleCount >=
                                endAudioSampleCount)
                            {
                                int64_t clampedSamples =
                                    maxAudioSampleCount - totalSamples;
                                if (clampedSamples > 0)
                                {
                                    clampedSamples = std::min(
                                        static_cast<size_t>(clampedSamples),
                                        sampleCount);
                                    auto tmp = audio::Audio::create(
                                        audio->getInfo(), clampedSamples);
                                    memcpy(
                                        tmp->getData(), audio->getData(),
                                        tmp->getByteCount());
                                    audio = tmp;
                                }
                                else
                                {
                                    skip = true;
                                }
                            }
                        }
                        else
                        {
                            skip = true;
                        }

                        if (audioTime.contains(currentAudioTime))
                        {
                            if (!skip)
                            {
                                writer->writeAudio(range, audio);

                                const size_t sampleCount =
                                    audio->getSampleCount();
                                currentSampleCount += sampleCount;
                                totalSamples += sampleCount;
                            }
                        }
                    }
                }

                if (hasVideo)
                {
                    if (annotations)
                    {
                        view->redraw();
                        view->flush();
                        Fl::check();

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

                        view->make_current();
                        CHECK_GL;
                        gl::initGLAD();

                        // Render the video.
                        gl::OffscreenBufferBinding binding(buffer);
                        CHECK_GL;
                        {
                            locale::SetAndRestore saved;
                            render->begin(
                                offscreenBufferSize,
                                view->getColorConfigOptions(),
                                view->lutOptions());
                            CHECK_GL;
                            render->drawVideo(
                                {videoData},
                                {math::Box2i(
                                    0, 0, renderSize.w, renderSize.h)});
                            CHECK_GL;
                            render->end();
                        }

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

                    if (videoTime.contains(currentTime))
                        writer->writeVideo(currentTime, outputImage);
                }

                if (hasVideo)
                    currentTime += otime::RationalTime(1, currentTime.rate());
                else
                    currentTime += otime::RationalTime(
                        currentTime.rate(), currentTime.rate());

                if (currentTime > endTime)
                {
                    running = false;
                }
                else
                {
                    // We need to use frameNext instead of seeking as large
                    // movies can lag behind the seek
                    if (annotations && hasVideo)
                        player->frameNext();
                    else
                        player->seek(currentTime);
                }
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }

        view->setFrameView(ui->uiPrefs->uiPrefsAutoFitImage->value());
        view->setHudActive(hud);
        view->setPresentationMode(presentation);
        player->seek(currentTime);
        player->setMute(mute);
        ui->uiTimeline->valid(0); // needed
        ui->uiTimeline->redraw();
        tcp->unlock();
    }

} // namespace mrv

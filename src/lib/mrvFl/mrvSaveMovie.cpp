// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvFl/mrvSaveOptions.h"
#include "mrvFl/mrvIO.h"

#include "mrvGL/mrvGLErrors.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvWidgets/mrvProgressReport.h"

#include "mrvCore/mrvImageOps.h"
#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvMath.h"
#include "mrvCore/mrvUtil.h"

#include <tlIO/System.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlGL/Init.h>
#include <tlGL/Util.h>
#include <tlGL/GLFWWindow.h>
#include <tlTimelineGL/Render.h>

#include <FL/Fl.H>

#include <chrono>
#include <string>
#include <sstream>





namespace
{
    const char* kModule = "save";
}

namespace mrv
{
    void waitForFrame(
        const mrv::TimelinePlayer* player, const otime::RationalTime& startTime)
    {
        using namespace tl;

        bool found = false;

        auto cacheInfoObserver =
            observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                player->player()->observeCacheInfo(),
                [&startTime, &found](const timeline::PlayerCacheInfo& value)
                {
                    for (const auto& t : value.videoFrames)
                    {
                        if (startTime >= t.start_time() &&
                            startTime <= t.end_time_exclusive())
                        {
                            found = true;
                            break;
                        }
                    }
                });

        while (!found)
        {
            Fl::check();
        }
    }

    void
    save_movie(const std::string& file, const ViewerUI* ui, SaveOptions options)
    {
        std::string msg;
        MyViewport* view = ui->uiView;
        bool presentation = view->getPresentationMode();
        bool hud = view->getHudActive();

        auto player = view->getTimelinePlayer();
        if (!player)
            return; // should never happen

        file::Path path(file);

        if (file::isTemporaryNDI(path))
        {
            LOG_ERROR(_("Cannot save an NDI stream"));
            return;
        }

        // Stop the playback
        player->stop();

        // Time range.
        auto timeRange = player->inOutRange();
        auto speed = player->speed();
        auto startTime = timeRange.start_time();
        auto endTime = timeRange.end_time_inclusive();
        auto currentTime = startTime;

        auto mute = player->isMuted();
        player->setMute(true);

        auto context = ui->app->getContext();

        // Get I/O cache and store its size.
        auto ioSystem = context->getSystem<io::System>();
        auto cache = ioSystem->getCache();

        size_t oldCacheSize = cache->getMax();

        const std::string& directory = path.getDirectory();
        const std::string& baseName = path.getBaseName();
        const std::string& number = path.getNumber();
        const std::string& extension = path.getExtension();

        std::string newFile = directory + baseName + number + extension;

        try
        {

            tl::io::Options ioOptions;

#ifdef TLRENDER_FFMPEG
            ioOptions["FFmpeg/WriteProfile"] = getLabel(options.ffmpegProfile);
            ioOptions["FFmpeg/AudioCodec"] = getLabel(options.ffmpegAudioCodec);
            ioOptions["FFmpeg/ThreadCount"] =
                string::Format("{0}").arg(ffmpeg::threadCount);

            // If we are not saving a movie, take speed from the player's
            // current speed.
            {
                const auto& model = ui->app->filesModel();
                const auto& Aitem = model->observeA()->get();
                const auto& extension = Aitem->path.getExtension();
                if (!file::isMovie(extension))
                {
                    ioOptions["FFmpeg/Speed"] =
                        string::Format("{0}").arg(speed);
                }
            }

            // If we have a preset, send it over.
            if (!options.ffmpegPreset.empty())
            {
                ioOptions["FFmpeg/PresetFile"] = options.ffmpegPreset;
            }

            ioOptions["FFmpeg/PixelFormat"] = options.ffmpegPixelFormat;
            ioOptions["FFmpeg/HardwareEncode"] =
                string::Format("{0}").arg(options.ffmpegHardwareEncode);
            if (options.ffmpegOverride)
            {
                ioOptions["FFmpeg/ColorRange"] = options.ffmpegColorRange;
                ioOptions["FFmpeg/ColorSpace"] = options.ffmpegColorSpace;
                ioOptions["FFmpeg/ColorPrimaries"] =
                    options.ffmpegColorPrimaries;
                ioOptions["FFmpeg/ColorTRC"] = options.ffmpegColorTRC;
            }
#endif

#ifdef TLRENDER_EXR
            ioOptions["OpenEXR/Compression"] =
                tl::string::Format("{0}").arg(options.exrCompression);
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

            {
                std::stringstream s;
                s << speed;
                ioOptions["OpenEXR/Speed"] = s.str();
            }
#endif

            auto model = ui->app->filesModel();
            auto Aitem = model->observeA()->get();
            std::string inputFile = Aitem->path.get();

            // Make I/O cache be 1Gb to deal with long movies fine.
            size_t bytes = memory::gigabyte;
            cache->setMax(bytes);

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

            const bool savingMovie = file::isMovie(extension);
            const bool savingAudio = file::isAudio(extension);
            if (savingMovie)
            {
                msg = string::Format(_("Saving movie to {0}.")).arg(newFile);
            }
            else if (savingAudio)
            {
                msg = string::Format(_("Saving audio to {0}.")).arg(newFile);
            }
            else
            {
                msg = string::Format(_("Saving pictures to {0}.")).arg(newFile);
            }
            LOG_STATUS(msg);

            // Render information.
            const auto& info = player->ioInfo();

            auto videoTime = info.videoTime;

            const bool hasVideo = (!info.video.empty()) && options.saveVideo;

            if (player->timeRange() != timeRange ||
                info.videoTime.start_time() != timeRange.start_time() ||
                info.videoTime.duration() != timeRange.duration())
            {
                double videoRate = info.videoTime.duration().rate();
                videoTime = otime::TimeRange(
                    timeRange.start_time().rescaled_to(videoRate),
                    timeRange.duration().rescaled_to(videoRate));
            }

            auto audioTime = time::invalidTimeRange;
            const double sampleRate = info.audio.sampleRate;
            bool hasAudio = info.audio.isValid();
            if (hasAudio)
            {
                audioTime = info.audioTime;
                if (player->timeRange() != timeRange ||
                    audioTime.start_time() !=
                        timeRange.start_time().rescaled_to(sampleRate))
                {
                    audioTime = otime::TimeRange(
                        timeRange.start_time().rescaled_to(sampleRate),
                        timeRange.duration().rescaled_to(sampleRate));
                }
            }

#ifdef TLRENDER_FFMPEG
            const std::string& profile = getLabel(options.ffmpegProfile);

            std::string newExtension = extension;
            if (profile.substr(0, 6) == "ProRes")
            {
                if (!string::compare(
                        extension, ".mov", string::Compare::CaseInsensitive))
                {
                    LOG_WARNING(_("ProRes profiles need a .mov movie "
                                  "extension.  Changing it to .mov."));
                    newExtension = ".mov";
                }
            }
            else if (profile == "VP9")
            {
                if (!string::compare(
                        extension, ".mp4", string::Compare::CaseInsensitive) &&
                    !string::compare(
                        extension, ".webm", string::Compare::CaseInsensitive) &&
                    !string::compare(
                        extension, ".mkv", string::Compare::CaseInsensitive))
                {
                    LOG_WARNING(
                        _("VP9 profile needs a .mp4, .mkv or .webm movie "
                          "extension.  Changing it to .mp4"));
                    newExtension = ".mp4";
                }
            }
            else if (profile == "AV1")
            {
                if (!string::compare(
                        extension, ".mp4", string::Compare::CaseInsensitive) &&
                    !string::compare(
                        extension, ".mkv", string::Compare::CaseInsensitive))
                {
                    LOG_WARNING(_("AV1 profile needs a .mp4 or .mkv movie "
                                  "extension.  Changing it to .mp4"));
                    newExtension = ".mp4";
                }
            }
            else if (profile == "Cineform")
            {
                if (!string::compare(
                        extension, ".mkv", string::Compare::CaseInsensitive))
                {
                    LOG_WARNING(_("GoPro Cineform profile needs a .mkv movie "
                                  "extension.  Changing it to .mkv"));
                    newExtension = ".mkv";
                }
            }
            else if (profile == "HAP")
            {
                if (!string::compare(
                        extension, ".mov", string::Compare::CaseInsensitive))
                {
                    LOG_WARNING(
                        _("HAP profile needs a .mov extension.  Changing "
                          "it to .mov"));
                    newExtension = ".mov";
                }
            }

            newFile = directory + baseName + number + newExtension;

            if (newFile != file)
            {
                if (fs::exists(newFile))
                {
                    /* xgettext:c++-format */
                    throw std::runtime_error(
                        string::Format(_("New file {0} already exist!  "
                                         "Cannot overwrite it."))
                            .arg(newFile));
                }
            }

            path = file::Path(newFile);
#endif

            bool saveEXR = string::compare(
                extension, ".exr", string::Compare::CaseInsensitive);
            bool saveHDR = string::compare(
                extension, ".hdr", string::Compare::CaseInsensitive);

            if (time::compareExact(videoTime, time::invalidTimeRange))
                videoTime = audioTime;

            const size_t endAudioSampleCount =
                endTime.rescaled_to(sampleRate).value();
            const size_t maxAudioSampleCount =
                timeRange.duration().rescaled_to(sampleRate).value();

            const std::string& originalFile = player->path().get();
            if (originalFile == file)
            {
                /* xgettext:c++-format */
                throw std::runtime_error(
                    string::Format("{0}: Saving over same file being played!")
                        .arg(file));
            }
            

            gl::OffscreenBufferOptions offscreenBufferOptions;
            std::shared_ptr<timeline_gl::Render> render;

            image::Size renderSize;
            int layerId = ui->uiColorChannel->value();
            if (layerId < 0)
                layerId = 0;

            const SaveResolution resolution = options.resolution;
            if (hasVideo)
            {
                auto compareSize = view->getRenderSize();
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
                auto rotation = view->getRotation();
                if (options.annotations && rotationSign(rotation) != 0)
                {
                    size_t tmp = renderSize.w;
                    renderSize.w = renderSize.h;
                    renderSize.h = tmp;

                    /* xgettext:c++-format */
                    msg = tl::string::Format(_("Rotated image info: {0}"))
                              .arg(renderSize);
                    LOG_STATUS(msg);
                }
                if (resolution == SaveResolution::kHalfSize)
                {
                    renderSize.w /= 2;
                    renderSize.h /= 2;
                    /* xgettext:c++-format */
                    msg = tl::string::Format(_("Scaled image info: {0}"))
                              .arg(renderSize);
                    LOG_STATUS(msg);
                }
                else if (resolution == SaveResolution::kQuarterSize)
                {
                    renderSize.w /= 4;
                    renderSize.h /= 4;
                    /* xgettext:c++-format */
                    msg = tl::string::Format(_("Scaled image info: {0}"))
                              .arg(renderSize);
                    LOG_STATUS(msg);
                }
            }

            bool interactive = view->visible_r();
            
            std::shared_ptr<gl::GLFWWindow> window;
            if (!interactive)
            {
                // Create the window.
                window = gl::GLFWWindow::create(
                    "bake", math::Size2i(1, 1), context,
                    static_cast<int>(gl::GLFWWindowOptions::MakeCurrent));
            }

            // Create the renderer.
            render = timeline_gl::Render::create(context);
            offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;

            // Create the writer.
            auto writerPlugin = ioSystem->getPlugin(path);

            if (!writerPlugin)
            {
                /* xgettext:c++-format */
                throw std::runtime_error(
                    string::Format(_("{0}: Cannot open writer plugin."))
                        .arg(file));
            }

            int X = 0, Y = 0;

            io::Info ioInfo;
            image::Info outputInfo;

            outputInfo.size = renderSize;
            std::shared_ptr<image::Image> outputImage;

            outputInfo.pixelType = info.video[layerId].pixelType;

            player->start();

            if (hasVideo)
            {
                /* xgettext:c++-format */
                msg = tl::string::Format(_("Image info: {0} {1}"))
                          .arg(outputInfo.size)
                          .arg(outputInfo.pixelType);
                LOG_STATUS(msg);

                if (options.annotations)
                {
                    view->setShowVideo(options.video);
                    view->setActionMode(ActionMode::kScrub);
                    view->setPresentationMode(true);
                    view->redraw();
                    // flush is needed
                    Fl::flush();
                    view->flush();

                    // Wait 2 seconds (needed on Apple for presentation mode
                    //                 visible resizing)
                    Fl::wait(2.0);

                    // returns pixel_w(), pixel_h()
                    auto viewportSize = view->getViewportSize();
                    float pixels_unit = view->pixels_per_unit();
                    viewportSize.w /= pixels_unit;
                    viewportSize.h /= pixels_unit;

                    math::Size2i outputSize;
                    if (viewportSize.w >= renderSize.w &&
                        viewportSize.h >= renderSize.h)
                    {
                        view->setFrameView(false);
#ifndef __APPLE__
                        pixels_unit = 1.0F;
#endif
                        if (resolution == SaveResolution::kHalfSize)
                            view->setViewZoom(0.5 * pixels_unit);
                        else if (resolution == SaveResolution::kQuarterSize)
                            view->setViewZoom(0.25 * pixels_unit);
                        else
                            view->setViewZoom(pixels_unit);
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

#ifdef __APPLE__
                if (options.annotations)
                {
                    switch (outputInfo.pixelType)
                    {
                    case image::PixelType::RGBA_F16:
                        outputInfo.pixelType = image::PixelType::RGBA_F16;
                        break;
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

                outputInfo = writerPlugin->getWriteInfo(outputInfo);
                if (image::PixelType::kNone == outputInfo.pixelType)
                {
#ifdef OPENGL_BACKEND
                    outputInfo.pixelType = image::PixelType::RGB_U8;
                    offscreenBufferOptions.colorType = image::PixelType::RGB_U8;
#endif
#ifdef TLRENDER_EXR
                    if (saveEXR)
                    {
                        offscreenBufferOptions.colorType =
                            image::PixelType::RGB_F32;
                    }
#endif
                    /* xgettext:c++-format */
                    msg = tl::string::Format(
                              _("Writer plugin did not get output info.  "
                                "Defaulting to {0}"))
                              .arg(offscreenBufferOptions.colorType);
                    LOG_STATUS(msg);
                }

#ifdef TLRENDER_EXR
                if (saveEXR)
                {
                    if (!options.annotations)
                    {
                        outputInfo.pixelType = options.exrPixelType;
                    }
                }
#endif
                if (saveHDR)
                {
                    outputInfo.pixelType = image::PixelType::RGB_F32;
                    offscreenBufferOptions.colorType =
                        image::PixelType::RGB_F32;
                }

                msg = tl::string::Format(_("Output info: {0} {1}"))
                          .arg(outputInfo.size)
                          .arg(outputInfo.pixelType);
                LOG_STATUS(msg);

#ifdef TLRENDER_EXR
                ioOptions["OpenEXR/PixelType"] = getLabel(outputInfo.pixelType);
#endif
                outputImage = image::Image::create(outputInfo);
                ioInfo.videoTime = videoTime;
                ioInfo.video.push_back(outputInfo);

#ifdef TLRENDER_FFMPEG
                if (hasVideo && savingMovie)
                {
                    auto entries = tl::ffmpeg::getProfileLabels();
                    std::string profileName =
                        entries[(int)options.ffmpegProfile];
                    
                    /* xgettext:c++-format */
                    msg = tl::string::Format(
                              _("Using profile {0}, pixel format {1}."))
                              .arg(profileName)
                              .arg(options.ffmpegPixelFormat);
                    LOG_STATUS(msg);
                    if (!options.ffmpegPreset.empty())
                    {
                        /* xgettext:c++-format */
                        msg = tl::string::Format(_("Using preset {0}."))
                                  .arg(options.ffmpegPreset);
                        LOG_STATUS(msg);
                    }
                }
#endif
            }

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

#ifdef TLRENDER_FFMPEG
            if (hasVideo && savingMovie)
            {
                if (static_cast<ffmpeg::AudioCodec>(options.ffmpegAudioCodec) ==
                        ffmpeg::AudioCodec::kNone ||
                    !hasAudio)
                    /* xgettext:c-format */
                    snprintf(
                        title, 1024,
                        _("Saving Movie without Audio %" PRId64 " - %" PRId64),
                        startFrame, endFrame);
                else
                    /* xgettext:c-format */
                    snprintf(
                        title, 1024,
                        _("Saving Movie with Audio %" PRId64 " - %" PRId64),
                        startFrame, endFrame);
            }
            else if (hasAudio && savingAudio)
            {
                /* xgettext:c-format */
                snprintf(
                    title, 1024, _("Saving Audio %" PRId64 " - %" PRId64),
                    startFrame, endFrame);
            }
            else
#endif
                if (hasVideo && !savingMovie && !savingAudio)
            {
                /* xgettext:c-format */
                snprintf(
                    title, 1024,
                    _("Saving Pictures without Audio %" PRId64 " - %" PRId64),
                    startFrame, endFrame);
                hasAudio = false;
            }
            else
            {
                LOG_ERROR(
                    _("Audio only in timeline, but not trying to save audio."));
                return;
            }

            ProgressReport progress(ui->uiMain, startFrame, endFrame, title);
            if (interactive)
                progress.show();

            bool running = true;

            // Don't send any tcp updates
            tcp->lock();

            GLenum format = gl::getReadPixelsFormat(outputInfo.pixelType);
            GLenum type = gl::getReadPixelsType(outputInfo.pixelType);
            if (hasVideo)
            {
                if (GL_NONE == format || GL_NONE == type)
                {
                    /* xgettext:c++-format */
                    throw std::runtime_error(
                        string::Format(_("{0}: Invalid OpenGL format and type"))
                            .arg(file));
                }

                /* xgettext:c++-format */
                msg = tl::string::Format(_("OpenGL info: {0}"))
                          .arg(offscreenBufferOptions.colorType);
                LOG_STATUS(msg);
            }

            // Turn off hud so it does not get captured by glReadPixels.
            view->setHudActive(false);

            math::Size2i offscreenBufferSize(renderSize.w, renderSize.h);

            std::shared_ptr<gl::OffscreenBuffer> buffer;
            if (hasVideo)
            {
                if (interactive)
                {
                    view->make_current();
                    gl::initGLAD();
                }

                buffer = gl::OffscreenBuffer::create(
                    offscreenBufferSize, offscreenBufferOptions);
            }

            size_t totalSamples = 0;
            size_t currentSampleCount =
                startTime.rescaled_to(sampleRate).value();

            waitForFrame(player, startTime);

            int32_t frameIndex = 0;
            
            while (running)
            {
                context->tick();

                // If progress window is closed, exit loop.
                if (interactive)
                {
                    if (!progress.tick())
                        break;
                }
                else
                {
                    /* xgettext:c++-format */
                    msg = string::Format(_("Saving... {0}")).arg(currentTime);
                    LOG_STATUS(msg);
                }

                if (hasAudio)
                {
                    const double seconds = currentTime.to_seconds();
                    const auto audioData =
                        timeline->getAudio(seconds).future.get();
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
                                otime::RationalTime(seconds, 1.0),
                                otime::RationalTime(1.0, 1.0));
                        auto audio = audioData.layers[0].audio;
                        if (!audio)
                        {
                            // Create silence
                            audio =
                                audio::Audio::create(info.audio, sampleRate);
                            audio->zero();
                        }

                        // \todo mix audio layers (or have a function in
                        // timeline to do it).
                        const auto currentAudioTime =
                            currentTime.rescaled_to(sampleRate);

                        // timeline->getAudio() returns one second of audio.
                        // Clamp to end of the timeRange/inOutRange.
                        if (!skip && std::round(currentAudioTime.value()) >=
                                         currentSampleCount)
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
                    if (options.annotations)
                    {
                        view->redraw();
                        view->flush();
                        Fl::flush();

#ifdef __APPLE__
                        Fl_RGB_Image* tmp = fl_capture_window(
                            view, X, Y, outputInfo.size.w, outputInfo.size.h);
                        Fl_Image* rgb =
                            tmp->copy(outputInfo.size.w, outputInfo.size.h);
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
                                (const uint8_t*)data[0], rgb->w(), rgb->h(),
                                rgb->d(), 4);
                            break;
                        case image::PixelType::RGBA_F16:
                            flipImageInY(
                                (Imath::half*)outputImage->getData(),
                                (const uint8_t*)data[0], rgb->w(), rgb->h(),
                                rgb->d(), 4);
                            break;
                        case image::PixelType::RGBA_F32:
                            flipImageInY(
                                (float*)outputImage->getData(),
                                (const uint8_t*)data[0], rgb->w(), rgb->h(),
                                rgb->d(), 4);
                            break;
                        default:
                            LOG_ERROR(_("Unsupported output format"));
                            break;
                        }

                        delete rgb;
#else
                        
#  ifdef VULKAN_BACKEND
#  else
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
                            X, Y, outputInfo.size.w, outputInfo.size.h, format,
                            type, outputImage->getData());
#  endif
                        
#endif
                    }
                    else
                    {
                        // Get the videoData
                        auto videoData =
                            timeline->getVideo(currentTime).future.get();

                        if (videoData.layers.empty() ||
                            !videoData.layers[0].image)
                        {
                            /* xgettext:c++-format */
                            const std::string err =
                                string::Format(_("Empty video data at time "
                                                 "{0}.  Repeating frame."))
                                    .arg(currentTime);
                            LOG_WARNING(err);
                        }
                        else
                        {

                            // This refreshes the view window
                            if (interactive)
                            {
                                view->make_current();
                                view->currentVideoCallback({videoData});
                                view->flush();
                            }

#ifdef VULKAN_BACKEND
                            VkDevice device = ctx.device;
                            VkCommandPool commandPool = ctx.commandPool;
                
                            VkCommandBuffer cmd = beginSingleTimeCommands(device, commandPool);
                            buffer->transitionToColorAttachment(cmd);

                            {
                                locale::SetAndRestore saved;
                                timeline::RenderOptions renderOptions;
                                renderOptions.clear = false;
                                render->begin(cmd, buffer, frameIndex,
                                              offscreenBufferSize,
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
                                    {math::Box2i(0, 0,
                                                 renderSize.w, renderSize.h)},
                                    {timeline::ImageOptions()},
                                    {timeline::DisplayOptions()},
                                    timeline::CompareOptions(),
                                    ui->uiView->getBackgroundOptions());
                    
                                render->end();
                            }

                            buffer->transitionToColorAttachment(cmd);
                
                            buffer->readPixels(cmd, 0, 0,
                                               renderSize.w, renderSize.h);
                                    
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
                                    
                                vkFreeCommandBuffers(device, commandPool, 1,
                                                     &cmd);    
                            }
                                    
#else
                            // back to conventional pixel operation
                            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                            // CHECK_GL;
                            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

                            if (interactive)
                                gl::initGLAD();

                            // Render the video to an offscreen buffer.
                            gl::OffscreenBufferBinding binding(buffer);
                            {
                                locale::SetAndRestore saved;
                                render->begin(offscreenBufferSize);
                                render->setOCIOOptions(view->getOCIOOptions());
                                render->setLUTOptions(view->lutOptions());
                                render->drawVideo(
                                    {videoData},
                                    {math::Box2i(
                                        0, 0, renderSize.w, renderSize.h)},
                                    {timeline::ImageOptions()},
                                    {timeline::DisplayOptions()},
                                    timeline::CompareOptions(),
                                    view->getBackgroundOptions());
                                render->end();
                            }

                            glPixelStorei(
                                GL_PACK_ALIGNMENT, outputInfo.layout.alignment);
#if defined(TLRENDER_API_GL_4_1)
                            glPixelStorei(
                                GL_PACK_SWAP_BYTES, outputInfo.layout.endian !=
                                                        memory::getEndian());
#endif // TLRENDER_API_GL_4_1

                            glReadPixels(
                                0, 0, outputInfo.size.w, outputInfo.size.h,
                                format, type, outputImage->getData());
#endif
                        }
                    }

                    if (videoTime.contains(currentTime))
                    {
                        const auto& tags = ui->uiView->getTags();
                        outputImage->setTags(tags);
                        writer->writeVideo(currentTime, outputImage);
                    }
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
                    // When saving video and not options.annotations, we cannot
                    // use seek as it corrupts the timeline.
                    if (options.annotations && hasVideo)
                        player->frameNext();
                    else if (!hasVideo)
                        player->seek(currentTime);
                }

#ifdef VULKAN_BACKEND
                frameIndex = (frameIndex + 1) % vlk::MAX_FRAMES_IN_FLIGHT;
#endif
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }

        view->setFrameView(ui->uiPrefs->uiPrefsAutoFitImage->value());
        view->setHudActive(hud);
        view->setPresentationMode(presentation);
        view->setShowVideo(true);
        player->seek(currentTime);
        player->setMute(mute);
        ui->uiTimeline->valid(0); // needed
        ui->uiTimeline->redraw();
        tcp->unlock();

        auto settings = ui->app->settings();
        if (file::isReadable(newFile))
        {
            settings->addRecentFile(path.get());
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }

        App::unsaved_annotations = false;

        cache->setMax(oldCacheSize);
    }

} // namespace mrv

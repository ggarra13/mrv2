// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvFl/mrvConvertImage.h"
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

#include <tlVk/Init.h>
#include <tlVk/Util.h>
#include <tlTimelineVk/Render.h>
#include <FL/Fl_Vk_Utils.H>
#include <FL/vk_enum_string_helper.h>

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
        
        std::string number = path.getNumber();
        if (!number.empty()) number = std::to_string(startTime.to_frames());

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
                throw std::runtime_error(
                    string::Format("{0}: Saving over same file being played!")
                        .arg(file));
            }
            
            vlk::OffscreenBufferOptions offscreenBufferOptions;
            std::shared_ptr<timeline_vlk::Render> render;

            image::Size renderSize;
            int layerId = ui->uiColorChannel->value();
            if (layerId < 0)
                layerId = 0;

            const SaveResolution resolution = options.resolution;
            if (hasVideo)
            {
                auto compareSize = view->getRenderSize();
                renderSize.w = compareSize.w;
                renderSize.h = compareSize.h;

                
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
            }

            bool interactive = view->visible_r();
            
            auto ctx = ui->uiView->getContext();
            render = timeline_vlk::Render::create(ctx, context);

            offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
            offscreenBufferOptions.pbo = true;

            // Create the writer.
            auto writerPlugin = ioSystem->getPlugin(path);

            if (!writerPlugin)
            {
                throw std::runtime_error(
                    string::Format(_("{0}: Cannot open writer plugin."))
                        .arg(file));
            }

            auto tags = ui->uiView->getTags();
            
            io::Info ioInfo;
            image::Info outputInfo, scaleInfo, bufferInfo;

            outputInfo.pixelType = info.video[layerId].pixelType;
            outputInfo.size.pixelAspectRatio = 1.0;

            // Images that may be created
            std::shared_ptr<image::Image> outputImage;
            std::shared_ptr<image::Image> annotationImage;
            std::shared_ptr<image::Image> bufferImage;
            std::shared_ptr<image::Image> scaleImage;

            if (hasVideo)
            {
                // Create scaleImage if resolution is not the same.
                if (resolution != SaveResolution::kSameSize)
                {
                    scaleInfo.size = renderSize;
                    scaleInfo.pixelType = outputInfo.pixelType;
                    scaleImage = image::Image::create(scaleInfo);

                    msg = tl::string::Format(_("Image info: {0} {1}"))
                          .arg(scaleInfo.size)
                          .arg(scaleInfo.pixelType);
                    LOG_STATUS(msg);
                }
            
                else if (resolution == SaveResolution::kHalfSize)
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

                outputInfo.size = renderSize;
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

#ifdef TLRENDER_EXR
                if (saveEXR)
                {
                    outputInfo.pixelType = options.exrPixelType;
                    offscreenBufferOptions.colorType =
                        outputInfo.pixelType;
                }
#endif
                if (saveHDR)
                {
                    outputInfo.pixelType = image::PixelType::RGB_F32;
                    offscreenBufferOptions.colorType =
                        image::PixelType::RGB_F32;
                }

#ifdef TLRENDER_EXR
                ioOptions["OpenEXR/PixelType"] = getLabel(outputInfo.pixelType);
#endif
                //
                // Create output image
                //
                outputImage = image::Image::create(outputInfo);
            
                ioInfo.videoTime = videoTime;
                ioInfo.video.push_back(outputInfo);

#ifdef TLRENDER_FFMPEG
                if (hasVideo && savingMovie)
                {
                    auto entries = tl::ffmpeg::getProfileLabels();
                    std::string profileName =
                        entries[(int)options.ffmpegProfile];

                    msg = tl::string::Format(
                              _("Using profile {0}, pixel format {1}."))
                              .arg(profileName)
                              .arg(options.ffmpegPixelFormat);
                    LOG_STATUS(msg);
                    if (!options.ffmpegPreset.empty())
                    {
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


            //
            // Create image buffer (main FBO).
            // 
            math::Size2i offscreenBufferSize(renderSize.w, renderSize.h);
            std::shared_ptr<vlk::OffscreenBuffer> buffer;
            if (hasVideo)
            {
                buffer = view->getVideoFBO();
                offscreenBufferOptions = buffer->getOptions();
                    
                if (options.annotations)
                {
                    
                    if (!annotationImage)
                    {
                        image::Info annotationInfo = outputInfo;
                        annotationInfo.pixelType = image::PixelType::RGBA_U8;
                        annotationImage = image::Image::create(annotationInfo);
                    }
                }

                const size_t width = buffer->getWidth();
                const size_t height = buffer->getHeight();
                
                bufferInfo = outputInfo;
                bufferInfo.pixelType = offscreenBufferOptions.colorType;
                bufferInfo.size.w = width;
                bufferInfo.size.h = height;
                bufferImage = image::Image::create(bufferInfo);
                        
                msg = tl::string::Format(_("Offscreen Buffer info: {0}"))
                      .arg(offscreenBufferOptions.colorType);
                LOG_STATUS(msg);
            }


            msg = tl::string::Format(_("Output info: {0} {1}"))
                  .arg(outputInfo.size)
                  .arg(outputInfo.pixelType);
            LOG_STATUS(msg);

                
            // Turn off hud so it does not get captured by readPixels.
            view->setHudActive(false);

            // Prepare annotations without HUD, cursors, and overlay with
            // a centered and frame image for easier checking.
            if (options.annotations)
            {
                view->setSaveOverlay(true);
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


            size_t totalSamples = 0;
            size_t currentSampleCount =
                startTime.rescaled_to(sampleRate).value();


            player->start();
            
            waitForFrame(player, startTime);

            int32_t frameIndex = 0;

            //
            // Create and show Progress window
            //
            char title[1024];

            int64_t startFrame = startTime.to_frames();
            int64_t endFrame = endTime.to_frames();

#ifdef TLRENDER_FFMPEG
            if (hasVideo && savingMovie)
            {
                if (static_cast<ffmpeg::AudioCodec>(options.ffmpegAudioCodec) ==
                        ffmpeg::AudioCodec::kNone ||
                    !hasAudio)
                    snprintf(
                        title, 1024,
                        _("Saving Movie without Audio %" PRId64 " - %" PRId64),
                        startFrame, endFrame);
                else
                    snprintf(
                        title, 1024,
                        _("Saving Movie with Audio %" PRId64 " - %" PRId64),
                        startFrame, endFrame);
            }
            else if (hasAudio && savingAudio)
            {
                snprintf(
                    title, 1024, _("Saving Audio %" PRId64 " - %" PRId64),
                    startFrame, endFrame);
            }
            else
#endif
                if (hasVideo && !savingMovie && !savingAudio)
            {
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


            // Don't send any tcp updates
            tcp->lock();

            // Start running...
            bool running = true;

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
                        view->setSaveOverlay(true);
                    }
                    else
                    {
                        view->setSaveOverlay(false);
                    }
                        
                    view->redraw();
                    view->flush(); // needed
                    Fl::flush();
                        
                    auto buffer = view->getVideoFBO();
                    auto bufferOptions = buffer->getOptions();
                    
                    VkDevice device = view->device();
                    VkCommandPool commandPool = view->commandPool();

                    // Read Main Image Viewport
                    VkCommandBuffer cmd = beginSingleTimeCommands(device,
                                                                  commandPool);
            
                    const size_t width = buffer->getWidth();
                    const size_t height = buffer->getHeight();
                    
                    buffer->readPixels(cmd, 0, 0, width, height);
                    
                    vkEndCommandBuffer(cmd);
                    
                    buffer->submitReadback(cmd);
                    
                    view->wait_queue();
                    
                    const void* imageData = buffer->getLatestReadPixels();
                    if (imageData)
                    {                            
                        std::memcpy(bufferImage->getData(), imageData,
                                    bufferImage->getDataByteCount());
                    }
                    else
                    {
                        LOG_ERROR(_("Could not read image data from view"));
                    }

                    vkFreeCommandBuffers(device, commandPool, 1, &cmd);

                    flipImageInY(bufferImage);
                        
                        
                    //
                    // Read Annotation Image
                    //
                    if (options.annotations)
                    {    
                        auto overlayBuffer = view->getAnnotationFBO();
                        if (overlayBuffer)
                        {
                            VkCommandBuffer cmd = beginSingleTimeCommands(device,
                                                                          commandPool);

                            overlayBuffer->readPixels(cmd, 0, 0, width, height);
            
                            vkEndCommandBuffer(cmd);
            
                            overlayBuffer->submitReadback(cmd);

                            view->wait_queue();
            
                            imageData = overlayBuffer->getLatestReadPixels();
                            if (imageData)
                            {
                                std::memcpy(annotationImage->getData(), imageData,
                                            annotationImage->getDataByteCount());
                            }
                            else
                            {
                                annotationImage->zero();
                            }

                            //
                            // Composite annotation image over buffer image.
                            //
                            composite_RGBA_U8(bufferImage, annotationImage);
                        
                            vkFreeCommandBuffers(device, commandPool, 1, &cmd);
                        }
                        else
                        {
                            annotationImage->zero();
                        }
                        
                        view->setSaveOverlay(false);
                    }
                
                
                    if (bufferImage != scaleImage)
                    {
                        if (!scaleImage)
                        {
                            scaleImage = outputImage;
                        }

                        convertImage(scaleImage, bufferImage);
                    }
                    
                    //
                    // Scale down result.
                    //
                    if (scaleImage)
                    {
                        if (outputImage != scaleImage &&
                            (scaleImage->getWidth() != outputImage->getWidth() ||
                             scaleImage->getHeight() != outputImage->getHeight()))
                        {
                            int numChannels = image::getChannelCount(outputImage->getPixelType());
                            scaleImageLinear(scaleImage->getData(),
                                             scaleImage->getWidth(),
                                             scaleImage->getHeight(),
                                             outputImage->getData(),
                                             outputImage->getWidth(),
                                             outputImage->getHeight(),
                                             numChannels);
                        }
                        else
                        {
                            outputImage = scaleImage;
                        }
                    }
                    else
                    {
                        outputImage = bufferImage;
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
                    if (hasVideo)
                        player->frameNext();
                    else
                        player->seek(currentTime);
                }

                frameIndex = (frameIndex + 1) % vlk::MAX_FRAMES_IN_FLIGHT;
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }

        view->setFrameView(ui->uiPrefs->uiPrefsAutoFitImage->value());
        view->setHudActive(hud);
        view->setShowVideo(true);
        view->setSaveOverlay(false);
                        
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

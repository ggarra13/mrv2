// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <algorithm>
#include <chrono>
#include <thread>

#include <tlIO/IOSystem.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlGL/Util.h>
#include <tlGL/Render.h>

#include <tlGlad/gl.h>

#include "mrvCore/mrvUtil.h"

#include "mrvWidgets/mrvProgressReport.h"

#include "mrvFl/mrvIO.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "save";
}

namespace mrv
{

    void save_movie(const std::string& file, ViewerUI* ui)
    {
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

        // Create the renderer.
        auto render = gl::Render::create(context);
        gl::OffscreenBufferOptions offscreenBufferOptions;
        offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
        auto buffer =
            gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);

        // Create the writer.
        auto writerPlugin =
            context->getSystem<io::System>()->getPlugin(file::Path(file));

        if (!writerPlugin)
        {
            throw std::runtime_error(
                string::Format("{0}: Cannot open").arg(file));
        }

        imaging::Info outputInfo;
        outputInfo.size = renderSize;
        outputInfo.pixelType = info.video[0].pixelType;

        outputInfo = writerPlugin->getWriteInfo(outputInfo);
        if (imaging::PixelType::None == outputInfo.pixelType)
        {
            outputInfo.pixelType = imaging::PixelType::RGB_U8;
        }

        auto outputImage = imaging::Image::create(outputInfo);

        io::Info ioInfo;
        ioInfo.video.push_back(outputInfo);
        ioInfo.videoTime = timeRange;

        auto writer = writerPlugin->write(file::Path(file), ioInfo);
        if (!writer)
        {
            throw std::runtime_error(
                string::Format("{0}: Cannot open").arg(file));
        }

        int64_t startFrame = startTime.to_frames();
        int64_t endFrame = endTime.to_frames();

        ProgressReport progress(ui->uiMain, startFrame, endFrame);
        progress.show();

        TimelineClass* c = ui->uiTimeWindow;
        c->uiTimeline->setTimelinePlayer(nullptr);

        bool running = true;
        while (running)
        {
            // Get the videoData
            const auto& videoData = timeline->getVideo(currentTime).get();

            c->uiTimeline->value(currentTime.value());

            // This works!
            view->currentVideoCallback(videoData, player);
            view->flush();

            // If progress window is closed, exit loop.
            if (!progress.tick())
                break;

            // Render the video.
            gl::OffscreenBufferBinding binding(buffer);
            char* saved_locale = strdup(setlocale(LC_NUMERIC, NULL));
            setlocale(LC_NUMERIC, "C");
            render->begin(
                renderSize, view->getColorConfigOptions(), view->lutOptions());
            render->drawVideo(
                {videoData}, {math::BBox2i(0, 0, renderSize.w, renderSize.h)});
            render->end();
            setlocale(LC_NUMERIC, saved_locale);
            free(saved_locale);

            glPixelStorei(GL_PACK_ALIGNMENT, outputInfo.layout.alignment);
            glPixelStorei(
                GL_PACK_SWAP_BYTES,
                outputInfo.layout.endian != memory::getEndian());
            const GLenum format = gl::getReadPixelsFormat(outputInfo.pixelType);
            const GLenum type = gl::getReadPixelsType(outputInfo.pixelType);
            if (GL_NONE == format || GL_NONE == type)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(file));
            }
            glReadPixels(
                0, 0, outputInfo.size.w, outputInfo.size.h, format, type,
                outputImage->getData());

            writer->writeVideo(currentTime, outputImage);

            currentTime += otime::RationalTime(1, currentTime.rate());
            if (currentTime > endTime)
            {
                running = false;
            }
        }

        c->uiTimeline->setTimelinePlayer(player);
        player->seek(currentTime);
    }

} // namespace mrv

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include "TestPatterns.h"

#include <tlTimelineGL/Render.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWWindow.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Util.h>

#include <tlIO/System.h>

#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/track.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace examples
    {
        namespace test_patterns
        {
            void App::_init(
                const std::vector<std::string>& argv,
                const std::shared_ptr<system::Context>& context)
            {
                BaseApp::_init(
                    argv, context, "test-patterns",
                    "Example test patterns application.");

                _window = gl::GLFWWindow::create(
                    "test-patterns", math::Size2i(1, 1), context,
                    static_cast<int>(gl::GLFWWindowOptions::MakeCurrent));
            }

            App::App() {}

            App::~App() {}

            std::shared_ptr<App> App::create(
                const std::vector<std::string>& argv,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(argv, context);
                return out;
            }

            int App::run()
            {
                if (0 == _exit)
                {
                    for (const auto& size :
                         {math::Size2i(1920, 1080), math::Size2i(3840, 2160),
                          math::Size2i(4096, 2160)})
                    {
                        otio::SerializableObject::Retainer<otio::Timeline>
                            otioTimeline(new otio::Timeline);
                        otio::SerializableObject::Retainer<otio::Track>
                            otioTrack(new otio::Track);
                        otioTimeline->tracks()->append_child(otioTrack);

                        for (const auto& name :
                             {CountTestPattern::getClassName(),
                              SwatchesTestPattern::getClassName(),
                              GridTestPattern::getClassName()})
                        {
                            // const std::string output =
                            // string::Format("{0}_{1}_pattern.mp4").arg(name).arg(size);
                            const std::string output =
                                string::Format("{0}_{1}.0.dpx")
                                    .arg(name)
                                    .arg(size);
                            std::cout << "Output: " << output << std::endl;
                            otio::SerializableObject::Retainer<otio::Clip>
                                otioClip(new otio::Clip);
                            // otio::SerializableObject::Retainer<otio::MediaReference>
                            // mediaReference(
                            //     new
                            //     otio::ExternalReference(string::Format("{0}").arg(output)));
                            otio::SerializableObject::Retainer<
                                otio::ImageSequenceReference>
                                mediaReference(new otio::ImageSequenceReference(
                                    "file://", file::Path(output).getBaseName(),
                                    file::Path(output).getExtension(), 0, 1,
                                    24));
                            const otime::TimeRange timeRange(
                                otime::RationalTime(0.0, 24.0),
                                otime::RationalTime(24 * 3, 24.0));
                            mediaReference->set_available_range(timeRange);
                            otioClip->set_media_reference(mediaReference);
                            otioTrack->append_child(otioClip);

                            // Create the I/O plugin.
                            auto writerPlugin =
                                _context->getSystem<io::System>()->getPlugin(
                                    file::Path(output));
                            if (!writerPlugin)
                            {
                                throw std::runtime_error(
                                    string::Format("{0}: Cannot open")
                                        .arg(output));
                            }
                            image::Info info;
                            info.size.w = size.w;
                            info.size.h = size.h;
                            info.pixelType = image::PixelType::RGB_U10;
                            info = writerPlugin->getWriteInfo(info);
                            if (image::PixelType::None == info.pixelType)
                            {
                                throw std::runtime_error(
                                    string::Format("{0}: Cannot open")
                                        .arg(output));
                            }
                            io::Info ioInfo;
                            ioInfo.video.push_back(info);
                            ioInfo.videoTime = timeRange;
                            auto writer =
                                writerPlugin->write(file::Path(output), ioInfo);
                            if (!writer)
                            {
                                throw std::runtime_error(
                                    string::Format("{0}: Cannot open")
                                        .arg(output));
                            }

                            // Create the offscreen buffer.
                            gl::OffscreenBufferOptions offscreenBufferOptions;
                            offscreenBufferOptions.colorType =
                                image::PixelType::RGBA_F32;
                            auto buffer = gl::OffscreenBuffer::create(
                                size, offscreenBufferOptions);
                            gl::OffscreenBufferBinding binding(buffer);
                            auto image = image::Image::create(info);

                            // Render the test pattern.
                            auto render = timeline_gl::Render::create(_context);
                            auto pattern = TestPatternFactory::create(
                                name, size, _context);
                            for (double i =
                                     ioInfo.videoTime.start_time().value();
                                 i < ioInfo.videoTime.duration().value();
                                 i = i + 1.0)
                            {
                                const otime::RationalTime time(i, 24.0);

                                render->begin(size);
                                pattern->render(render, time);
                                render->end();

                                // Write the image.
                                glPixelStorei(
                                    GL_PACK_ALIGNMENT, info.layout.alignment);
#if defined(TLRENDER_API_GL_4_1)
                                glPixelStorei(
                                    GL_PACK_SWAP_BYTES,
                                    info.layout.endian != memory::getEndian());
#endif // TLRENDER_API_GL_4_1
                                const GLenum format =
                                    gl::getReadPixelsFormat(info.pixelType);
                                const GLenum type =
                                    gl::getReadPixelsType(info.pixelType);
                                if (GL_NONE == format || GL_NONE == type)
                                {
                                    throw std::runtime_error(
                                        string::Format("{0}: Cannot open")
                                            .arg(output));
                                }
                                glReadPixels(
                                    0, 0, info.size.w, info.size.h, format,
                                    type, image->getData());
                                writer->writeVideo(time, image);
                            }
                        }

                        otioTimeline->to_json_file(
                            string::Format("{0}.otio").arg(size));
                    }
                }
                return _exit;
            }
        } // namespace test_patterns
    } // namespace examples
} // namespace tl

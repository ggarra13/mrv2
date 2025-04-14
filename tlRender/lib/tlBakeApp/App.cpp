// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlBakeApp/App.h>

#include <tlTimelineGL/Render.h>

#include <tlIO/System.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWWindow.h>
#include <tlGL/Util.h>

#include <tlCore/File.h>
#include <tlCore/Math.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

namespace tl
{
    namespace bake
    {
        void App::_init(
            const std::vector<std::string>& argv,
            const std::shared_ptr<system::Context>& context)
        {
            BaseApp::_init(
                argv, context, "tlbake",
                "Render a timeline to a movie or image sequence.",
                {app::CmdLineValueArg<std::string>::create(
                     _input, "input", "The input timeline."),
                 app::CmdLineValueArg<std::string>::create(
                     _output, "output", "The output file.")},
                {
                    app::CmdLineValueOption<otime::TimeRange>::create(
                        _options.inOutRange, {"-inOutRange"},
                        "Set the in/out points range."),
                    app::CmdLineValueOption<math::Size2i>::create(
                        _options.renderSize, {"-renderSize", "-rs"},
                        "Render size."),
                    app::CmdLineValueOption<image::PixelType>::create(
                        _options.outputPixelType, {"-outputPixelType", "-op"},
                        "Output pixel type.", std::string(),
                        string::join(image::getPixelTypeLabels(), ", ")),
                    app::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.fileName, {"-ocio"},
                        "OpenColorIO configuration file name (e.g., "
                        "config.ocio)."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.input, {"-ocioInput"},
                        "OpenColorIO input name."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.display, {"-ocioDisplay"},
                        "OpenColorIO display name."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.view, {"-ocioView"},
                        "OpenColorIO view name."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.look, {"-ocioLook"},
                        "OpenColorIO look name."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.lutOptions.fileName, {"-lut"},
                        "LUT file name."),
                    app::CmdLineValueOption<timeline::LUTOrder>::create(
                        _options.lutOptions.order, {"-lutOrder"},
                        "LUT operation order.",
                        string::Format("{0}").arg(_options.lutOptions.order),
                        string::join(timeline::getLUTOrderLabels(), ", ")),
                    app::CmdLineValueOption<float>::create(
                        _options.sequenceDefaultSpeed,
                        {"-sequenceDefaultSpeed"},
                        "Default speed for image sequences.",
                        string::Format("{0}").arg(
                            _options.sequenceDefaultSpeed)),
                    app::CmdLineValueOption<int>::create(
                        _options.sequenceThreadCount, {"-sequenceThreadCount"},
                        "Number of threads for image sequence I/O.",
                        string::Format("{0}").arg(
                            _options.sequenceThreadCount)),
#if defined(TLRENDER_EXR)
                    app::CmdLineValueOption<float>::create(
                        _options.exrDWACompressionLevel,
                        {"-exrDWACompressionLevel"},
                        "OpenEXR DWA compression level.",
                        string::Format("{0}").arg(
                            _options.exrDWACompressionLevel)),
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
                    app::CmdLineValueOption<std::string>::create(
                        _options.ffmpegWriteProfile, {"-ffmpegProfile", "-ffp"},
                        "FFmpeg output profile.", std::string(),
                        string::join(ffmpeg::getProfileLabels(), ", ")),
                    app::CmdLineValueOption<int>::create(
                        _options.ffmpegThreadCount, {"-ffmpegThreadCount"},
                        "Number of threads for FFmpeg I/O.",
                        string::Format("{0}").arg(_options.ffmpegThreadCount)),
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
                    app::CmdLineValueOption<int>::create(
                        _options.usdRenderWidth, {"-usdRenderWidth"},
                        "USD render width.",
                        string::Format("{0}").arg(_options.usdRenderWidth)),
                    app::CmdLineValueOption<float>::create(
                        _options.usdComplexity, {"-usdComplexity"},
                        "USD render complexity setting.",
                        string::Format("{0}").arg(_options.usdComplexity)),
                    app::CmdLineValueOption<usd::DrawMode>::create(
                        _options.usdDrawMode, {"-usdDrawMode"},
                        "USD draw mode.",
                        string::Format("{0}").arg(_options.usdDrawMode),
                        string::join(usd::getDrawModeLabels(), ", ")),
                    app::CmdLineValueOption<bool>::create(
                        _options.usdEnableLighting, {"-usdEnableLighting"},
                        "USD enable lighting.",
                        string::Format("{0}").arg(_options.usdEnableLighting)),
                    app::CmdLineValueOption<bool>::create(
                        _options.usdSRGB, {"-usdSRGB"},
                        "USD enable sRGB color space.",
                        string::Format("{0}").arg(_options.usdSRGB)),
                    app::CmdLineValueOption<size_t>::create(
                        _options.usdStageCache, {"-usdStageCache"},
                        "USD stage cache size.",
                        string::Format("{0}").arg(_options.usdStageCache)),
                    app::CmdLineValueOption<size_t>::create(
                        _options.usdDiskCache, {"-usdDiskCache"},
                        "USD disk cache size in gigabytes. A size of zero "
                        "disables the cache.",
                        string::Format("{0}").arg(_options.usdDiskCache)),
#endif // TLRENDER_USD
                });
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
                _startTime = std::chrono::steady_clock::now();

                // Create the window.
                _window = gl::GLFWWindow::create(
                    "test-patterns", math::Size2i(1, 1), _context,
                    static_cast<int>(gl::GLFWWindowOptions::MakeCurrent));

                // Read the timeline.
                timeline::Options options;
                options.ioOptions = _getIOOptions();
                _timeline =
                    timeline::Timeline::create(_input, _context, options);
                _timeRange = _timeline->getTimeRange();
                _print(string::Format("Timeline range: {0}-{1}")
                           .arg(_timeRange.start_time().value())
                           .arg(_timeRange.end_time_inclusive().value()));
                _print(string::Format("Timeline speed: {0}")
                           .arg(_timeRange.duration().rate()));

                // Time range.
                if (time::isValid(_options.inOutRange))
                {
                    _timeRange = _options.inOutRange;
                }
                _print(string::Format("In/out range: {0}-{1}")
                           .arg(_timeRange.start_time().value())
                           .arg(_timeRange.end_time_inclusive().value()));
                _inputTime = _timeRange.start_time();
                _outputTime =
                    otime::RationalTime(0.0, _timeRange.duration().rate());

                // Render information.
                const auto& info = _timeline->getIOInfo();
                if (info.video.empty())
                {
                    throw std::runtime_error("No video information");
                }
                _renderSize =
                    _options.renderSize.isValid()
                        ? _options.renderSize
                        : math::Size2i(
                              info.video[0].size.w, info.video[0].size.h);
                _print(string::Format("Render size: {0}").arg(_renderSize));

                // Create the renderer.
                _render = timeline_gl::Render::create(_context);
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = gl::offscreenColorDefault;
                _buffer = gl::OffscreenBuffer::create(
                    _renderSize, offscreenBufferOptions);

                // Create the writer.
                _writerPlugin = _context->getSystem<io::System>()->getPlugin(
                    file::Path(_output));
                if (!_writerPlugin)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot open").arg(_output));
                }
                io::Info ioInfo;
                _outputInfo.size.w = _renderSize.w;
                _outputInfo.size.h = _renderSize.h;
                _outputInfo.pixelType =
                    _options.outputPixelType != image::PixelType::kNone
                        ? _options.outputPixelType
                        : info.video[0].pixelType;
                _outputInfo = _writerPlugin->getWriteInfo(_outputInfo);
                if (image::PixelType::kNone == _outputInfo.pixelType)
                {
                    _outputInfo.pixelType = image::PixelType::RGB_U8;
                }
                _print(string::Format("Output info: {0} {1}")
                           .arg(_outputInfo.size)
                           .arg(_outputInfo.pixelType));
                _outputImage = image::Image::create(_outputInfo);
                ioInfo.video.push_back(_outputInfo);
                ioInfo.videoTime = _timeRange;
                _writer = _writerPlugin->write(file::Path(_output), ioInfo);
                if (!_writer)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot open").arg(_output));
                }

                // Start the main loop.
                gl::OffscreenBufferBinding binding(_buffer);
                while (_running)
                {
                    _tick();
                }

                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = now - _startTime;
                _print(
                    string::Format("Seconds elapsed: {0}").arg(diff.count()));
                _print(string::Format("Average FPS: {0}")
                           .arg(_timeRange.duration().value() / diff.count()));
            }

            return _exit;
        }

        io::Options App::_getIOOptions() const
        {
            io::Options out;
            {
                std::stringstream ss;
                ss << _options.sequenceDefaultSpeed;
                out["SequenceIO/DefaultSpeed"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.sequenceThreadCount;
                out["SequenceIO/ThreadCount"] = ss.str();
            }

#if defined(TLRENDER_EXR)
            {
                std::stringstream ss;
                ss << _options.exrCompression;
                out["OpenEXR/Compression"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.exrDWACompressionLevel;
                out["OpenEXR/DWACompressionLevel"] = ss.str();
            }
#endif // TLRENDER_EXR

#if defined(TLRENDER_FFMPEG)
            if (!_options.ffmpegWriteProfile.empty())
            {
                out["FFmpeg/WriteProfile"] = _options.ffmpegWriteProfile;
            }
            {
                std::stringstream ss;
                ss << _options.ffmpegThreadCount;
                out["FFmpeg/ThreadCount"] = ss.str();
            }
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            {
                std::stringstream ss;
                ss << _options.usdRenderWidth;
                out["USD/renderWidth"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdComplexity;
                out["USD/complexity"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdDrawMode;
                out["USD/drawMode"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdEnableLighting;
                out["USD/enableLighting"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdSRGB;
                out["USD/sRGB"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdStageCache;
                out["USD/stageCacheCount"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdDiskCache * memory::gigabyte;
                out["USD/diskCacheByteCount"] = ss.str();
            }
#endif // TLRENDER_USD

            return out;
        }

        void App::_tick()
        {
            _context->tick();

            _printProgress();

            // Render the video.
            _render->begin(_renderSize);
            _render->setOCIOOptions(_options.ocioOptions);
            _render->setLUTOptions(_options.lutOptions);
            const auto videoData = _timeline->getVideo(_inputTime).future.get();
            _render->drawVideo(
                {videoData}, {math::Box2i(0, 0, _renderSize.w, _renderSize.h)});
            _render->end();

            // Write the frame.
            glPixelStorei(GL_PACK_ALIGNMENT, _outputInfo.layout.alignment);
#if defined(TLRENDER_API_GL_4_1)
            glPixelStorei(
                GL_PACK_SWAP_BYTES,
                _outputInfo.layout.endian != memory::getEndian());
#endif // TLRENDER_API_GL_4_1
            const GLenum format =
                gl::getReadPixelsFormat(_outputInfo.pixelType);
            const GLenum type = gl::getReadPixelsType(_outputInfo.pixelType);
            if (GL_NONE == format || GL_NONE == type)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(_output));
            }
            glReadPixels(
                0, 0, _outputInfo.size.w, _outputInfo.size.h, format, type,
                _outputImage->getData());
            _writer->writeVideo(_outputTime, _outputImage);

            // Advance the time.
            _inputTime += otime::RationalTime(1, _inputTime.rate());
            if (_inputTime > _timeRange.end_time_inclusive())
            {
                _running = false;
            }
            _outputTime += otime::RationalTime(1, _outputTime.rate());
        }

        void App::_printProgress()
        {
            const int64_t c = static_cast<int64_t>(
                _inputTime.value() - _timeRange.start_time().value());
            const int64_t d =
                static_cast<int64_t>(_timeRange.duration().value());
            if (d >= 100 && c % (d / 100) == 0)
            {
                _print(string::Format("Complete: {0}%")
                           .arg(static_cast<int>(
                               c / static_cast<float>(d) * 100)));
            }
        }
    } // namespace bake
} // namespace tl

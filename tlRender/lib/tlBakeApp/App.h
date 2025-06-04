// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlBaseApp/BaseApp.h>

#include <tlGL/OffscreenBuffer.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Timeline.h>

#include <tlIO/SequenceIO.h>
#if defined(TLRENDER_EXR)
#    include <tlIO/OpenEXR.h>
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
#    include <tlIO/FFmpeg.h>
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
#    include <tlIO/USD.h>
#endif // TLRENDER_USD

namespace tl
{
    namespace gl
    {
        class GLFWWindow;
    }

    //! tlbake application
    namespace bake
    {
        //! Application options.
        struct Options
        {
            otime::TimeRange inOutRange = time::invalidTimeRange;
            math::Size2i renderSize;
            image::PixelType outputPixelType = image::PixelType::kNone;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            float sequenceDefaultSpeed = io::sequenceDefaultSpeed;
            int sequenceThreadCount = io::sequenceThreadCount;

#if defined(TLRENDER_EXR)
            exr::Compression exrCompression = exr::Compression::ZIP;
            float exrDWACompressionLevel = 45.F;
#endif // TLRENDER_EXR

#if defined(TLRENDER_FFMPEG)
            std::string ffmpegWriteProfile;
            int ffmpegThreadCount = ffmpeg::threadCount;
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            int usdRenderWidth = 1920;
            float usdComplexity = 1.F;
            usd::DrawMode usdDrawMode = usd::DrawMode::ShadedSmooth;
            bool usdEnableLighting = true;
            bool usdSRGB = true;
            size_t usdStageCache = 10;
            size_t usdDiskCache = 0;
#endif // TLRENDER_USD
        };

        //! Application.
        class App : public app::BaseApp
        {
            TLRENDER_NON_COPYABLE(App);

        protected:
            void _init(
                const std::vector<std::string>&,
                const std::shared_ptr<system::Context>&);
            App();

        public:
            ~App();

            //! Create a new application.
            static std::shared_ptr<App> create(
                const std::vector<std::string>&,
                const std::shared_ptr<system::Context>&);

            //! Run the application.
            int run();

        private:
            io::Options _getIOOptions() const;

            void _tick();
            void _printProgress();

            std::string _input;
            std::string _output;
            Options _options;

            std::shared_ptr<timeline::Timeline> _timeline;
            math::Size2i _renderSize;
            image::Info _outputInfo;
            otime::TimeRange _timeRange = time::invalidTimeRange;
            otime::RationalTime _inputTime = time::invalidTime;
            otime::RationalTime _outputTime = time::invalidTime;

            std::shared_ptr<gl::GLFWWindow> _window;
            std::shared_ptr<io::IPlugin> _usdPlugin;
            std::shared_ptr<timeline::IRender> _render;
            std::shared_ptr<gl::OffscreenBuffer> _buffer;

            std::shared_ptr<io::IPlugin> _writerPlugin;
            std::shared_ptr<io::IWrite> _writer;
            std::shared_ptr<image::Image> _outputImage;

            bool _running = true;
            std::chrono::steady_clock::time_point _startTime;
        };
    } // namespace bake
} // namespace tl

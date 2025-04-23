// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Gonzalo Garramuño
// All rights reserved.

#include <tlDevice/NDI/NDIOutputDevice.h>
#include <tlDevice/NDI/NDIUtil.h>
#include <tlDevice/GLUtil.h>

#include <tlTimelineGL/Render.h>

#include <tlGL/GLFWWindow.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Texture.h>
#include <tlGL/Util.h>

#include <tlTimeline/Util.h>

#include <tlIO/NDI.h>

#include <tlCore/AudioResample.h>
#include <tlCore/Context.h>
#include <tlCore/Locale.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <list>
#include <mutex>
#include <tuple>

namespace tl
{
    namespace ndi
    {
        namespace
        {
            const std::chrono::milliseconds timeout(8);
            const char* kModule = "ndi ";
        } // namespace

        namespace
        {
            inline bool is_equal(float x, float y)
            {
                int intNum1 = (int)(x * 1000 + 0.5);
                int intNum2 = (int)(y * 1000 + 0.5);

                // Compare the resulting integers.
                return intNum1 == intNum2;
            }

            inline float fadeValue(double sample, double in, double out)
            {
                return (sample - in) / (out - in);
            }

            // Function to escape quotes in JSON for XML compatibility
            std::string escape_quotes_for_xml(const std::string& json_str)
            {
                std::string escaped;
                for (char c : json_str)
                {
                    if (c == '"')
                    {
                        escaped += "&quot;"; // Replace " with &quot;
                    }
                    else
                    {
                        escaped += c;
                    }
                }
                return escaped;
            }
        } // namespace

        struct OutputDevice::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<observer::Value<device::DeviceConfig> > config;
            std::shared_ptr<observer::Value<bool> > enabled;
            std::shared_ptr<observer::Value<bool> > active;
            std::shared_ptr<observer::Value<math::Size2i> > size;
            std::shared_ptr<observer::Value<otime::RationalTime> > frameRate;
            std::shared_ptr<observer::Value<otime::RationalTime> > currentTime;

            std::shared_ptr<timeline::Player> player;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> >
                playbackObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> >
                currentTimeObserver;
            std::shared_ptr<observer::ListObserver<timeline::VideoData> >
                videoObserver;
            std::shared_ptr<observer::ListObserver<timeline::AudioData> >
                audioObserver;
            std::shared_ptr<observer::ValueObserver<double> > speedObserver;

            std::shared_ptr<gl::GLFWWindow> window;

            struct Mutex
            {
                device::DeviceConfig config;
                bool enabled = false;
                bool active = false;
                math::Size2i size;
                otime::RationalTime frameRate = time::invalidTime;
                timeline::OCIOOptions ocioOptions;
                timeline::LUTOptions lutOptions;
                std::vector<timeline::ImageOptions> imageOptions;
                std::vector<timeline::DisplayOptions> displayOptions;
                device::HDRMode hdrMode = device::HDRMode::FromFile;
                image::HDRData hdrData;
                timeline::CompareOptions compareOptions;
                timeline::BackgroundOptions backgroundOptions;
                math::Size2i sourceViewportSize;
                math::Vector2i viewPos;
                double viewZoom = 1.0;
                bool frameView = true;
                float rotateZ = 0.F;
                otime::TimeRange timeRange = time::invalidTimeRange;
                timeline::Playback playback = timeline::Playback::Stop;
                otime::RationalTime currentTime = time::invalidTime;
                otime::RationalTime playbackStartTime = time::invalidTime;
                double speed = 24.F;
                double defaultSpeed = 24.F;
                std::vector<timeline::VideoData> videoData;
                std::shared_ptr<image::Image> overlay;
                float volume = 1.F;
                bool mute = false;
                std::vector<int> channelMute;
                std::chrono::steady_clock::time_point muteTimeout;
                double audioOffset = 0.0;
                std::vector<timeline::AudioData> audioData;
                std::map<int64_t, timeline::AudioData> audioDataCache;

                bool reset = false;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                // Video variables
                math::Size2i size;
                device::PixelType outputPixelType = device::PixelType::kNone;
                device::HDRMode hdrMode = device::HDRMode::FromFile;
                image::HDRData hdrData;
                math::Size2i sourceViewportSize;
                math::Vector2i viewPos;
                double viewZoom = 1.0;
                float rotateZ = 0.F;
                bool frameView = true;
                otime::TimeRange timeRange = time::invalidTimeRange;
                std::vector<timeline::VideoData> videoData;
                std::shared_ptr<image::Image> overlay;

                std::shared_ptr<timeline::IRender> render;
                std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;
                GLuint pbo = 0;

                // NDI variables
                NDIlib_send_instance_t NDI_send = nullptr;
                NDIlib_video_frame_t NDI_video_frame;
                NDIlib_audio_frame_interleaved_32f_t NDI_audio_frame;
                std::string metadata;

                // Audio variables
                bool backwards = false;
                bool reset = true;
                std::shared_ptr<audio::AudioResample> resample;
                std::list<std::shared_ptr<audio::Audio> > buffer;
                std::shared_ptr<audio::Audio> silence;
                size_t rtAudioCurrentFrame = 0;
                size_t backwardsSamples = std::numeric_limits<size_t>::max();
                size_t outSampleRate = 0;
                size_t outSamples = 0;
                size_t outChannelCount = 2;

                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        void
        OutputDevice::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;
            p.config = observer::Value<device::DeviceConfig>::create();
            p.enabled = observer::Value<bool>::create(false);
            p.active = observer::Value<bool>::create(false);
            p.size = observer::Value<math::Size2i>::create();
            p.frameRate =
                observer::Value<otime::RationalTime>::create(time::invalidTime);
            p.currentTime =
                observer::Value<otime::RationalTime>::create(time::invalidTime);

            p.mutex.reset = true;

            p.window = gl::GLFWWindow::create(
                "tl::ndi::OutputDevice", math::Size2i(1, 1), context,
                static_cast<int>(gl::GLFWWindowOptions::kNone));
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    p.window->makeCurrent();
                    _run();
                    p.window->doneCurrent();
                });
        }

        OutputDevice::OutputDevice() :
            _p(new Private)
        {
        }

        OutputDevice::~OutputDevice()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<OutputDevice>
        OutputDevice::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<OutputDevice>(new OutputDevice);
            out->_init(context);
            return out;
        }

        device::DeviceConfig OutputDevice::getConfig() const
        {
            return _p->config->get();
        }

        std::shared_ptr<observer::IValue<device::DeviceConfig> >
        OutputDevice::observeConfig() const
        {
            return _p->config;
        }

        void OutputDevice::setConfig(const device::DeviceConfig& value)
        {
            TLRENDER_P();
            if (p.config->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.config = value;
                }
                p.thread.cv.notify_one();
            }
        }

        bool OutputDevice::isEnabled() const
        {
            return _p->enabled->get();
        }

        std::shared_ptr<observer::IValue<bool> >
        OutputDevice::observeEnabled() const
        {
            return _p->enabled;
        }

        void OutputDevice::setEnabled(bool value)
        {
            TLRENDER_P();
            if (p.enabled->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.enabled = value;
                }
                p.thread.cv.notify_one();
            }
        }

        bool OutputDevice::isActive() const
        {
            return _p->active->get();
        }

        std::shared_ptr<observer::IValue<bool> >
        OutputDevice::observeActive() const
        {
            return _p->active;
        }

        const math::Size2i& OutputDevice::getSize() const
        {
            return _p->size->get();
        }

        std::shared_ptr<observer::IValue<math::Size2i> >
        OutputDevice::observeSize() const
        {
            return _p->size;
        }

        const otime::RationalTime& OutputDevice::getFrameRate() const
        {
            return _p->frameRate->get();
        }

        std::shared_ptr<observer::IValue<otime::RationalTime> >
        OutputDevice::observeFrameRate() const
        {
            return _p->frameRate;
        }

        void OutputDevice::setView(
            const tl::math::Size2i& sourceViewportSize,
            const tl::math::Vector2i& position, double zoom, float rotateZ,
            bool frame)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.sourceViewportSize = sourceViewportSize;
                p.mutex.viewPos = position;
                p.mutex.viewZoom = zoom;
                p.mutex.rotateZ = rotateZ;
                p.mutex.frameView = frame;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.ocioOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.lutOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setImageOptions(
            const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.imageOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setDisplayOptions(
            const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.displayOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setHDR(
            device::HDRMode hdrMode, const image::HDRData& hdrData)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.hdrMode = hdrMode;
                p.mutex.hdrData = hdrData;
            }
            p.thread.cv.notify_one();
        }

        void
        OutputDevice::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.compareOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setBackgroundOptions(
            const timeline::BackgroundOptions& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.backgroundOptions = value;
            }
            p.thread.cv.notify_one();
        }

        void
        OutputDevice::setOverlay(const std::shared_ptr<image::Image>& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.overlay = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setVolume(float value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.volume = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setMute(bool value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.mute = value;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setAudioOffset(double value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.audioOffset = value;
            }
            p.thread.cv.notify_one();
        }

        void
        OutputDevice::setPlayer(const std::shared_ptr<timeline::Player>& value)
        {
            TLRENDER_P();
            if (value == p.player)
                return;

            p.playbackObserver.reset();
            p.currentTimeObserver.reset();
            p.speedObserver.reset();
            p.videoObserver.reset();
            p.audioObserver.reset();

            p.player = value;

            if (p.player)
            {
                auto weak = std::weak_ptr<OutputDevice>(
                    std::dynamic_pointer_cast<OutputDevice>(
                        shared_from_this()));
                p.playbackObserver =
                    observer::ValueObserver<timeline::Playback>::create(
                        p.player->observePlayback(),
                        [weak](timeline::Playback value)
                        {
                            if (auto device = weak.lock())
                            {
                                {
                                    std::unique_lock<std::mutex> lock(
                                        device->_p->mutex.mutex);
                                    device->_p->mutex.reset = true;
                                    device->_p->mutex.playback = value;
                                    device->_p->mutex.playbackStartTime =
                                        device->_p->player->getCurrentTime();
                                }
                                device->_p->thread.cv.notify_one();
                            }
                        },
                        observer::CallbackAction::Suppress);
                p.currentTimeObserver =
                    observer::ValueObserver<otime::RationalTime>::create(
                        p.player->observeCurrentTime(),
                        [weak](const otime::RationalTime& value)
                        {
                            if (auto device = weak.lock())
                            {
                                {
                                    std::unique_lock<std::mutex> lock(
                                        device->_p->mutex.mutex);
                                    double diff = std::abs(
                                        value.value() -
                                        device->_p->mutex.currentTime.value());
                                    if (diff > 1.0)
                                    {
                                        device->_p->mutex.reset = true;
                                        device->_p->mutex.playbackStartTime =
                                            value;
                                    }
                                    device->_p->mutex.currentTime = value;
                                }
                                device->_p->thread.cv.notify_one();
                            }
                        },
                        observer::CallbackAction::Suppress);
                p.videoObserver =
                    observer::ListObserver<timeline::VideoData>::create(
                        p.player->observeCurrentVideo(),
                        [weak](const std::vector<timeline::VideoData>& value)
                        {
                            if (auto device = weak.lock())
                            {
                                {
                                    std::unique_lock<std::mutex> lock(
                                        device->_p->mutex.mutex);
                                    device->_p->mutex.videoData = value;
                                }
                                device->_p->thread.cv.notify_one();
                            }
                        },
                        observer::CallbackAction::Suppress);
                p.audioObserver =
                    observer::ListObserver<timeline::AudioData>::create(
                        p.player->observeCurrentAudio(),
                        [weak](const std::vector<timeline::AudioData>& value)
                        {
                            if (auto device = weak.lock())
                            {
                                {
                                    std::unique_lock<std::mutex> lock(
                                        device->_p->mutex.mutex);
                                    device->_p->mutex.audioData = value;
                                }
                                device->_p->thread.cv.notify_one();
                            }
                        },
                        observer::CallbackAction::Suppress);
                p.speedObserver = observer::ValueObserver<double>::create(
                    p.player->observeSpeed(),
                    [weak](double value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(
                                    device->_p->mutex.mutex);
                                device->_p->mutex.speed = value;
                                device->_p->mutex.reset = true;
                                device->_p->mutex.playbackStartTime =
                                    device->_p->player->getCurrentTime();
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    },
                    observer::CallbackAction::Suppress);
            }

            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (p.player)
                {
                    p.mutex.timeRange = p.player->getTimeRange();
                    p.mutex.playback = p.player->getPlayback();
                    p.mutex.currentTime = p.player->getCurrentTime();
                    p.mutex.speed = p.player->getSpeed();
                    p.mutex.defaultSpeed = p.player->getDefaultSpeed();
                    p.mutex.playbackStartTime = p.mutex.currentTime;
                    p.mutex.reset = true;
                }
                else
                {
                    p.mutex.timeRange = time::invalidTimeRange;
                    p.mutex.playback = timeline::Playback::Count;
                    p.mutex.currentTime = time::invalidTime;
                    p.mutex.speed = p.mutex.defaultSpeed = 24.F;
                }
                p.mutex.videoData.clear();
                p.mutex.audioData.clear();
                if (p.player)
                {
                    p.mutex.videoData = p.player->getCurrentVideo();
                    p.mutex.audioData = p.player->getCurrentAudio();
                }
            }
        }

        void OutputDevice::tick()
        {
            TLRENDER_P();
            bool active = false;
            math::Size2i size = p.size->get();
            otime::RationalTime frameRate = p.frameRate->get();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                active = p.mutex.active;
                size = p.mutex.size;
                frameRate = p.mutex.frameRate;
            }
            p.active->setIfChanged(active);
            p.size->setIfChanged(size);
            p.frameRate->setIfChanged(frameRate);
        }

        timeline::AudioData OutputDevice::findAudioData(double seconds)
        {
            TLRENDER_P();
            timeline::AudioData audioData;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                const auto j = p.mutex.audioDataCache.find(seconds);
                if (j != p.mutex.audioDataCache.end())
                {
                    audioData = j->second;
                }
            }

            // if (audioData.layers.empty())
            // {
            //     std::cerr << "NO AUDIO DATA FOUND FOR " << seconds <<
            //     std::endl; for (const auto& j : p.mutex.audioDataCache)
            //     {
            //         std::cerr << "\tcache.seconds=" << j.first << std::endl;
            //     }
            // }

            return audioData;
        }

        void OutputDevice::_run()
        {
            TLRENDER_P();

            device::DeviceConfig config;
            bool enabled = false;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            timeline::BackgroundOptions backgroundOptions;
            timeline::Playback playback = timeline::Playback::Stop;
            otime::RationalTime currentTime = time::invalidTime;
            math::Size2i size;
            float volume = 1.F;
            bool mute = false;
            bool reset = true;
            otime::RationalTime frameRate = time::invalidTime;
            double audioOffset = 0.0;
            double speed = 24.0F;
            std::vector<timeline::AudioData> audioData;
            std::shared_ptr<image::Image> overlay;

            if (auto context = p.context.lock())
            {
                p.thread.render = timeline_gl::Render::create(context);
            }

            auto t = std::chrono::steady_clock::now();

            // Clear the NDI structs
            auto& video_frame = p.thread.NDI_video_frame;
            auto& audio_frame = p.thread.NDI_audio_frame;

            while (p.thread.running)
            {
                bool createDevice = false;
                bool doRender = false;
                bool audioDataChanged = false;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                            lock, timeout,
                            [this, config, enabled, ocioOptions, lutOptions,
                             imageOptions, displayOptions, compareOptions,
                             backgroundOptions, playback, currentTime,
                             frameRate, speed, volume, mute, audioOffset,
                             audioData, reset]
                            {
                                return config != _p->mutex.config ||
                                       enabled != _p->mutex.enabled ||
                                       ocioOptions != _p->mutex.ocioOptions ||
                                       lutOptions != _p->mutex.lutOptions ||
                                       imageOptions != _p->mutex.imageOptions ||
                                       displayOptions !=
                                           _p->mutex.displayOptions ||
                                       _p->thread.hdrMode !=
                                           _p->mutex.hdrMode ||
                                       _p->thread.hdrData !=
                                           _p->mutex.hdrData ||
                                       compareOptions !=
                                           _p->mutex.compareOptions ||
                                       backgroundOptions !=
                                           _p->mutex.backgroundOptions ||
                                       _p->thread.viewPos !=
                                           _p->mutex.viewPos ||
                                       _p->thread.viewZoom !=
                                           _p->mutex.viewZoom ||
                                       _p->thread.rotateZ !=
                                           _p->mutex.rotateZ ||
                                       _p->thread.frameView !=
                                           _p->mutex.frameView ||
                                       _p->thread.timeRange !=
                                           _p->mutex.timeRange ||
                                       playback != _p->mutex.playback ||
                                       currentTime != _p->mutex.currentTime ||
                                       _p->thread.videoData !=
                                           _p->mutex.videoData ||
                                       _p->thread.overlay !=
                                           _p->mutex.overlay ||
                                       volume != _p->mutex.volume ||
                                       mute != _p->mutex.mute ||
                                       audioOffset != _p->mutex.audioOffset ||
                                       audioData != _p->mutex.audioData ||
                                       speed != _p->mutex.speed;
                            }))
                    {
                        p.thread.reset = createDevice ||
                                         playback != p.mutex.playback ||
                                         frameRate != p.mutex.frameRate;

                        size = timeline::getRenderSize(
                            compareOptions.mode, p.thread.videoData);

                        createDevice = p.mutex.config != config ||
                                       p.mutex.enabled != enabled ||
                                       p.mutex.frameRate != frameRate ||
                                       p.mutex.speed != speed ||
                                       p.mutex.size != size;

                        audioDataChanged = createDevice ||
                                           audioData != p.mutex.audioData ||
                                           currentTime != p.mutex.currentTime;

                        config = p.mutex.config;
                        enabled = p.mutex.enabled;

                        p.thread.timeRange = p.mutex.timeRange;
                        playback = p.mutex.playback;
                        currentTime = p.mutex.currentTime;

                        doRender =
                            createDevice ||
                            ocioOptions != p.mutex.ocioOptions ||
                            lutOptions != p.mutex.lutOptions ||
                            imageOptions != p.mutex.imageOptions ||
                            displayOptions != p.mutex.displayOptions ||
                            p.thread.hdrMode != p.mutex.hdrMode ||
                            p.thread.hdrData != p.mutex.hdrData ||
                            compareOptions != p.mutex.compareOptions ||
                            backgroundOptions != p.mutex.backgroundOptions ||
                            p.thread.sourceViewportSize !=
                                p.mutex.sourceViewportSize ||
                            p.thread.viewPos != p.mutex.viewPos ||
                            p.thread.viewZoom != p.mutex.viewZoom ||
                            p.thread.rotateZ != p.mutex.rotateZ ||
                            p.thread.frameView != p.mutex.frameView ||
                            p.thread.videoData != p.mutex.videoData ||
                            p.thread.overlay != p.mutex.overlay;
                        ocioOptions = p.mutex.ocioOptions;
                        lutOptions = p.mutex.lutOptions;
                        imageOptions = p.mutex.imageOptions;
                        displayOptions = p.mutex.displayOptions;
                        p.thread.hdrMode = p.mutex.hdrMode;
                        p.thread.hdrData = p.mutex.hdrData;
                        compareOptions = p.mutex.compareOptions;
                        backgroundOptions = p.mutex.backgroundOptions;
                        p.thread.sourceViewportSize =
                            p.mutex.sourceViewportSize;
                        p.thread.viewPos = p.mutex.viewPos;
                        p.thread.viewZoom = p.mutex.viewZoom;
                        p.thread.rotateZ = p.mutex.rotateZ;
                        p.thread.frameView = p.mutex.frameView;
                        p.thread.videoData = p.mutex.videoData;
                        p.thread.overlay = p.mutex.overlay;

                        volume = p.mutex.volume;
                        mute = p.mutex.mute;
                        audioOffset = p.mutex.audioOffset;
                        audioData = p.mutex.audioData;
                    }
                }

                if (createDevice)
                {
                    if (p.thread.pbo != 0)
                    {
                        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                        glDeleteBuffers(1, &p.thread.pbo);
                        p.thread.pbo = 0;
                    }
                    p.thread.offscreenBuffer.reset();

                    bool active = false;

                    p.thread.size = size;

                    if (enabled)
                    {
                        if (!p.thread.videoData.empty())
                        {
                            double rate = 0;
                            for (const auto& videoData : p.thread.videoData)
                            {
                                const double videoRate = videoData.time.rate();
                                if (videoRate > rate)
                                    rate = videoRate;
                            }
                            frameRate = otime::RationalTime(1.0, rate);
                        }

                        try
                        {
                            _createDevice(config, active, size, frameRate);
                        }
                        catch (const std::exception& e)
                        {
                            if (auto context = p.context.lock())
                            {
                                context->log(
                                    "tl::ndi::OutputDevice", e.what(),
                                    log::Type::Error);
                            }
                        }
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.active = active;
                        p.mutex.speed = speed;
                        p.mutex.size = size;
                        p.mutex.frameRate = frameRate;
                    }

                    glGenBuffers(1, &p.thread.pbo);
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo);
                    glBufferData(
                        GL_PIXEL_PACK_BUFFER,
                        getPackPixelsSize(
                            p.thread.size, p.thread.outputPixelType),
                        NULL, GL_STREAM_READ);
                }

                if (audioDataChanged && p.thread.render && !config.noAudio)
                {
                    _cacheUpdate(audioData);
                    _audio();
                }

                if (doRender && p.thread.render)
                {
                    try
                    {
                        _render(
                            config, ocioOptions, lutOptions, imageOptions,
                            displayOptions, compareOptions, backgroundOptions);
                    }
                    catch (const std::exception& e)
                    {
                        if (auto context = p.context.lock())
                        {
                            context->log(
                                "tl::ndi::OutputDevice", e.what(),
                                log::Type::Error);
                        }
                    }
                }

                _read(config);

                const auto t1 = std::chrono::steady_clock::now();
                const std::chrono::duration<double> diff = t1 - t;
                // std::cerr << "diff: " << diff.count() * 1000 << std::endl;
                t = t1;
            }

            if (p.thread.pbo != 0)
            {
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                glDeleteBuffers(1, &p.thread.pbo);
                p.thread.pbo = 0;
            }
            p.thread.offscreenBuffer.reset();
            p.thread.render.reset();

            // Free the video frame
            free(p.thread.NDI_video_frame.p_data);
            p.thread.NDI_video_frame.p_data = nullptr;

            // Free the audio frame
            free(p.thread.NDI_audio_frame.p_data);
            p.thread.NDI_audio_frame.p_data = nullptr;

            // Destroy the NDI sender
            NDIlib_send_destroy(p.thread.NDI_send);
            p.thread.NDI_send = nullptr;
        }

        void OutputDevice::_createDevice(
            const device::DeviceConfig& config, bool& active,
            math::Size2i& size, otime::RationalTime& frameRate)
        {
            TLRENDER_P();
            if (config.deviceIndex != -1 && config.displayModeIndex != -1 &&
                config.pixelType != device::PixelType::kNone)
            {
                if (size.w == 0 || size.h == 0 || !p.player)
                    return;

                if (!p.thread.NDI_send)
                {
                    NDIlib_send_create_t send_create;
                    send_create.p_groups = NULL;
                    send_create.clock_video = true;
                    send_create.clock_audio = false;

                    p.thread.NDI_send = NDIlib_send_create();
                    if (!p.thread.NDI_send)
                    {
                        throw std::runtime_error("NDIlib_send_create failed");
                    }
                }

                auto& video_frame = p.thread.NDI_video_frame;

                p.thread.outputPixelType = getOutputType(config.pixelType);

                video_frame.xres = size.w;
                video_frame.yres = size.h;

                const double tolerance = 1.0e-03;
                const double speedMultiplier =
                    p.mutex.speed / p.player->getDefaultSpeed();

                double frame_rate = frameRate.rate() * speedMultiplier;
                const auto& oldFrameRate = frameRate;
                frameRate = otime::RationalTime(1.0, frame_rate);

                int denominator = 1000;
                int numerator = static_cast<int>(frame_rate * 1000);
                if (std::abs(frame_rate - 120.0) < tolerance)
                {
                    numerator = 120000;
                    denominator = 1000;
                }
                else if (std::abs(frame_rate - 60.0) < tolerance)
                {
                    numerator = 60000;
                    denominator = 1000;
                }
                else if (std::abs(frame_rate - 59.94) < tolerance)
                {
                    numerator = 60000;
                    denominator = 1001;
                }
                else if (std::abs(frame_rate - 50.0) < tolerance)
                {
                    numerator = 60000;
                    denominator = 1200;
                }
                else if (std::abs(frame_rate - 48.0) < tolerance)
                {
                    numerator = 48;
                    denominator = 1;
                }
                else if (std::abs(frame_rate - 47.952) < tolerance)
                {
                    numerator = 48000;
                    denominator = 1001;
                }
                else if (std::abs(frame_rate - 30.0) < tolerance)
                {
                    numerator = 30;
                    denominator = 1;
                }
                else if (std::abs(frame_rate - 29.976) < tolerance)
                {
                    numerator = 30000;
                    denominator = 1001;
                }
                else if (std::abs(frame_rate - 25.0) < tolerance)
                {
                    numerator = 30000;
                    denominator = 1200;
                }
                else if (std::abs(frame_rate - 24.0) < tolerance)
                {
                    numerator = 24;
                    denominator = 1;
                }
                else if (std::abs(frame_rate - 23.976) < tolerance)
                {
                    numerator = 24000;
                    denominator = 1001;
                }
                else if (std::abs(frame_rate - 15.0) < tolerance)
                {
                    numerator = 15;
                    denominator = 1;
                }
                else if (std::abs(frame_rate - 14.988) < tolerance)
                {
                    numerator = 15000;
                    denominator = 1001;
                }
                else if (std::abs(frame_rate - 12.5) < tolerance)
                {
                    numerator = 12500;
                    denominator = 1000;
                }
                else if (std::abs(frame_rate - 12.0) < tolerance)
                {
                    numerator = 12;
                    denominator = 1;
                }
                else if (std::abs(frame_rate - 11.988) < tolerance)
                {
                    numerator = 12000;
                    denominator = 1001;
                }
                else
                {
                    numerator = 1000;
                    denominator = 1000 * std::round(frame_rate);
                    frameRate = otime::RationalTime(numerator, denominator);
                }

                if (oldFrameRate != frameRate)
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.reset = true;
                }

                video_frame.frame_rate_N = numerator;
                video_frame.frame_rate_D = denominator;
                video_frame.picture_aspect_ratio = size.getAspect();
                video_frame.frame_format_type =
                    NDIlib_frame_format_type_progressive;

                video_frame.FourCC = toNDI(p.thread.outputPixelType);
                if (video_frame.FourCC == NDIlib_FourCC_video_type_max)
                {
                    throw std::runtime_error("Invalid pixel type for NDI!");
                }

                size_t dataSize =
                    getPackPixelsSize(size, p.thread.outputPixelType);
                if (dataSize == 0)
                {
                    throw std::runtime_error(
                        "Invalid data size for output pixel type");
                }
                free(video_frame.p_data);
                video_frame.p_data = (uint8_t*)malloc(dataSize);
                memset(video_frame.p_data, 128, dataSize);
                if (!video_frame.p_data)
                {
                    throw std::runtime_error("Out of memory allocating p_data");
                }
                video_frame.frame_format_type =
                    NDIlib_frame_format_type_progressive;
                if (auto context = p.context.lock())
                {
                    context->log(
                        "tl::ndi::OutputDevice",
                        string::Format(
                            "\n"
                            "    #{0} {1}\n"
                            "    video: {2} {3}\n"
                            "    audio: {4} {5} {6}")
                            .arg(config.deviceIndex)
                            .arg(config.displayModeIndex)
                            .arg(p.thread.size)
                            .arg(frameRate));
                }

                active = true;
            }
        }

        void OutputDevice::_cacheUpdate(
            const std::vector<timeline::AudioData>& audioDataList)
        {
            TLRENDER_P();
            if (!p.player)
                return;

            auto& thread = p.thread;

            const double outSampleRate = 1.0F; // 48000.0F;

            // Get the pl<yer ranges to be cached.
            const otime::TimeRange& timeRange = p.player->getTimeRange();
            const otime::TimeRange& inOutRange = p.player->getInOutRange();

            const otime::TimeRange audioTimeRange = otime::TimeRange(
                timeRange.start_time().rescaled_to(outSampleRate),
                timeRange.duration().rescaled_to(outSampleRate));

            // Set inOutRange in outSampleRate units.
            const otime::RationalTime& currentTime =
                p.mutex.currentTime.rescaled_to(outSampleRate);
            const auto cacheOptions = p.player->getCacheOptions();

            timeline::CacheDirection cacheDirection =
                timeline::CacheDirection::Count;
            switch (p.mutex.playback)
            {
            case timeline::Playback::Reverse:
                cacheDirection = timeline::CacheDirection::Reverse;
                break;
            case timeline::Playback::Forward:
            default:
                cacheDirection = timeline::CacheDirection::Forward;
                break;
            }

            // std::cerr << "time range: " << timeRange << std::endl;
            // std::cerr << "inOutRange range: " << inOutRange << std::endl;

            // Get the CacheOptions
            // we need to look up one frame for buffer
            const otime::RationalTime readAheadDivided(
                cacheOptions.readAhead.value() + 1.0,
                cacheOptions.readAhead.rate());
            const otime::RationalTime readBehindDivided(
                cacheOptions.readBehind.value() + 1.0,
                cacheOptions.readBehind.rate());

            // std::cerr << "read ahead divided " << readAheadDivided <<
            // std::endl; std::cerr << "read behind divided " <<
            // readBehindDivided << std::endl;

            // Rescale read aheads to times
            const otime::RationalTime readAheadRescaled =
                readAheadDivided.rescaled_to(outSampleRate).floor();
            const otime::RationalTime readBehindRescaled =
                readBehindDivided.rescaled_to(outSampleRate).floor();
            // Get the audio ranges to be cached.
            const otime::RationalTime audioOffsetTime =
                otime::RationalTime(p.mutex.audioOffset, 1.0)
                    .rescaled_to(outSampleRate);
            const otime::RationalTime audioOffsetAhead =
                otime::RationalTime(
                    audioOffsetTime.value() < 0.0
                        ? -audioOffsetTime
                        : otime::RationalTime(0.0, outSampleRate))
                    .round();
            const otime::RationalTime audioOffsetBehind =
                otime::RationalTime(
                    audioOffsetTime.value() > 0.0
                        ? audioOffsetTime
                        : otime::RationalTime(0.0, outSampleRate))
                    .round();
            // std::cerr << "currentTime: " << currentTime << std::endl;
            otime::TimeRange audioRange = time::invalidTimeRange;
            switch (cacheDirection)
            {
            case timeline::CacheDirection::Forward:
                audioRange =
                    otime::TimeRange::range_from_start_end_time_inclusive(
                        currentTime - readBehindRescaled - audioOffsetBehind,
                        currentTime + readAheadRescaled + audioOffsetAhead);
                break;
            case timeline::CacheDirection::Reverse:
                audioRange =
                    otime::TimeRange::range_from_start_end_time_inclusive(
                        currentTime - readAheadRescaled - audioOffsetAhead,
                        currentTime + readBehindRescaled + audioOffsetBehind);
                break;
            default:
                break;
            }
            const otime::TimeRange inOutAudioRange =
                otime::TimeRange::range_from_start_end_time_inclusive(
                    inOutRange.start_time() - audioOffsetBehind,
                    inOutRange.end_time_inclusive() + audioOffsetAhead)
                    .clamped(audioTimeRange);
            // std::cerr << "in out audio range: " << inOutAudioRange <<
            // std::endl;
            const auto audioRanges = timeline::loopCache(
                audioTimeRange, inOutAudioRange, cacheDirection);

            // Remove old audio from the cache.
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                auto audioCacheIt = p.mutex.audioDataCache.begin();
                while (audioCacheIt != p.mutex.audioDataCache.end())
                {
                    const otime::TimeRange cacheRange(
                        otime::RationalTime(
                            timeRange.start_time().rescaled_to(1.0).value() +
                                audioCacheIt->first,
                            1.0),
                        otime::RationalTime(1.0, 1.0));
                    const auto j = std::find_if(
                        audioRanges.begin(), audioRanges.end(),
                        [cacheRange](const otime::TimeRange& value)
                        { return cacheRange.intersects(value); });
                    if (j == audioRanges.end())
                    {
                        audioCacheIt =
                            p.mutex.audioDataCache.erase(audioCacheIt);
                    }
                    else
                    {
                        ++audioCacheIt;
                    }
                }
            }

            // Now, add the audios from the cacheList
            for (const auto& audioData : audioDataList)
            {
                p.mutex
                    .audioDataCache[static_cast<int64_t>(audioData.seconds)] =
                    audioData;
            }
        }

        void OutputDevice::_audio()
        {
            TLRENDER_P();
            if (!p.player)
                return;

            const otime::TimeRange& timeRange = p.player->getTimeRange();
            const otime::RationalTime& currentTime = p.player->getCurrentTime();
            const auto& currentLocalTime = currentTime - timeRange.start_time();
            double currentSeconds = currentLocalTime.to_seconds();
            double secondsD = std::floor(currentSeconds);

            auto& thread = p.thread;

            const timeline::AudioData& audioData = findAudioData(secondsD);

            std::shared_ptr<audio::Audio> inputAudio;
            for (const auto& layer : audioData.layers)
            {
                if (layer.audio)
                {
                    inputAudio = layer.audio;
                    break;
                }
            }

            if (!inputAudio)
            {
                return;
            }

            const auto& inputInfo = inputAudio->getInfo();
            const int channelCount = inputInfo.channelCount;
            const size_t inSampleRate = inputInfo.sampleRate;

            // One frame
            const size_t outSampleRate = 48000;
            const otime::RationalTime kOneFrame(1.0, currentTime.rate());
            const size_t outSamples =
                kOneFrame.rescaled_to(outSampleRate).value();
            const auto& inSampleDuration = kOneFrame.rescaled_to(inSampleRate);

            // auto outStartSample  =
            // otime::RationalTime(secondsD, 1.0).rescaled_to(outSampleRate);
            // auto outCurrentSample  =
            // otime::RationalTime(currentSeconds, 1.0).rescaled_to(outSampleRate);
            // size_t outSampleStart = outStartSample.value();
            // size_t outSampleCurrent = outCurrentSample.value();

            timeline::Playback playback = timeline::Playback::Stop;
            otime::RationalTime playbackStartTime = time::invalidTime;
            double audioOffset = 0.0;
            double speed = 1.0;
            double defaultSpeed = 1.0;
            double speedMultiplier = 1.0F;
            float volume = 1.F;
            bool mute = false;
            std::vector<int> channelMute;
            std::chrono::steady_clock::time_point muteTimeout;
            bool reset = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                playbackStartTime = p.mutex.playbackStartTime;
                audioOffset = p.mutex.audioOffset;
                speed = p.mutex.speed;
                defaultSpeed = p.player->getDefaultSpeed();
                speedMultiplier = speed / defaultSpeed;
                volume = p.mutex.volume;
                mute = p.mutex.mute;
                channelMute = p.mutex.channelMute;
                muteTimeout = p.mutex.muteTimeout;
                playback = p.player->getPlayback();
                reset = p.mutex.reset;
                p.mutex.reset = false;
            }

            switch (playback)
            {
            case timeline::Playback::Forward:
            case timeline::Playback::Reverse:
            {
                // Flush the audio resampler and buffer when the RtAudio
                // playback is reset.
                if (reset)
                {
                    if (thread.resample)
                    {
                        thread.resample->flush();
                    }
                    thread.silence.reset();
                    thread.buffer.clear();
                    thread.rtAudioCurrentFrame = 0;
                    thread.backwardsSamples =
                        std::numeric_limits<size_t>::max();
                    thread.reset = false;
                }

                const auto outputInfo =
                    audio::Info(channelCount, audio::DataType::F32, 48000);
                const size_t inSampleRate = inputInfo.sampleRate;
                const size_t outSampleRate =
                    outputInfo.sampleRate * speedMultiplier;

                // Create the audio resampler.
                if (!thread.resample ||
                    (thread.resample &&
                     thread.resample->getInputInfo() != inputInfo) ||
                    (thread.resample &&
                     thread.resample->getOutputInfo() != outputInfo))
                {
                    thread.resample =
                        audio::AudioResample::create(inputInfo, outputInfo);
                }

                // Fill the audio buffer.
                if (inputInfo.sampleRate <= 0 ||
                    playbackStartTime == time::invalidTime)
                    return;

                const bool backwards = playback == timeline::Playback::Reverse;
                if (!thread.silence)
                {
                    thread.silence =
                        audio::Audio::create(inputInfo, inSampleRate);
                    thread.silence->zero();
                }

                const int64_t playbackStartFrame =
                    playbackStartTime.rescaled_to(inSampleRate).value() -
                    p.player->getTimeRange()
                        .start_time()
                        .rescaled_to(inSampleRate)
                        .value() -
                    otime::RationalTime(audioOffset, 1.0)
                        .rescaled_to(inSampleRate)
                        .value();
                const auto bufferSampleCount =
                    audio::getSampleCount(thread.buffer);
                const auto& timeOffset =
                    otime::RationalTime(
                        thread.rtAudioCurrentFrame + bufferSampleCount,
                        outSampleRate)
                        .rescaled_to(inSampleRate);

                const int64_t frameOffset = timeOffset.value();
                int64_t frame = playbackStartFrame;

                if (backwards)
                {
                    frame -= frameOffset;
                }
                else
                {
                    frame += frameOffset;
                }

                // Works but minor pops
                int64_t seconds = inSampleRate > 0 ? (frame / inSampleRate) : 0;
                int64_t inOffsetSamples = frame - seconds * inSampleRate;

                while (audio::getSampleCount(thread.buffer) < outSamples)
                {
                    const timeline::AudioData& audioData =
                        findAudioData(seconds);

                    std::vector<float> volumeScale;
                    volumeScale.reserve(audioData.layers.size());
                    std::vector<std::shared_ptr<audio::Audio> > audios;
                    std::vector<const uint8_t*> audioDataP;
                    const int64_t dataByteOffset =
                        inOffsetSamples * inputInfo.getByteCount();
                    const size_t sampleGlobal =
                        seconds * inSampleRate + inOffsetSamples;

                    int audioIndex = 0;
                    std::shared_ptr<audio::Audio> audio;
                    for (const auto& layer : audioData.layers)
                    {
                        float volumeMultiplier = 1.F;
                        if (layer.audio && layer.audio->getInfo() == inputInfo)
                        {
                            audio = layer.audio;

                            if (layer.inTransition)
                            {
                                const auto& clipTimeRange = layer.clipTimeRange;
                                const auto& range = otime::TimeRange(
                                    clipTimeRange.start_time().rescaled_to(
                                        inSampleRate),
                                    clipTimeRange.duration().rescaled_to(
                                        inSampleRate));
                                const auto inOffset =
                                    layer.inTransition->in_offset().value();
                                const auto outOffset =
                                    layer.inTransition->out_offset().value();
                                const auto fadeInBegin =
                                    range.start_time().value() - inOffset;
                                const auto fadeInEnd =
                                    range.start_time().value() + outOffset;
                                if (sampleGlobal > fadeInBegin)
                                {
                                    if (sampleGlobal < fadeInEnd)
                                    {
                                        volumeMultiplier = fadeValue(
                                            sampleGlobal,
                                            range.start_time().value() -
                                                inOffset - 1.0,
                                            range.start_time().value() +
                                                outOffset);
                                        volumeMultiplier =
                                            std::min(1.F, volumeMultiplier);
                                    }
                                }
                                else
                                {
                                    volumeMultiplier = 0.F;
                                }
                            }

                            if (layer.outTransition)
                            {
                                const auto& clipTimeRange = layer.clipTimeRange;
                                const auto& range = otime::TimeRange(
                                    clipTimeRange.start_time().rescaled_to(
                                        inSampleRate),
                                    clipTimeRange.duration().rescaled_to(
                                        inSampleRate));

                                const auto inOffset =
                                    layer.outTransition->in_offset().value();
                                const auto outOffset =
                                    layer.outTransition->out_offset().value();
                                const auto fadeOutBegin =
                                    range.end_time_inclusive().value() -
                                    inOffset;
                                const auto fadeOutEnd =
                                    range.end_time_inclusive().value() +
                                    outOffset;
                                if (sampleGlobal > fadeOutBegin)
                                {
                                    if (sampleGlobal < fadeOutEnd)
                                    {
                                        volumeMultiplier =
                                            1.F - fadeValue(
                                                      sampleGlobal,
                                                      range.end_time_inclusive()
                                                              .value() -
                                                          inOffset,
                                                      range.end_time_inclusive()
                                                              .value() +
                                                          outOffset + 1.0);
                                    }
                                    else
                                    {
                                        volumeMultiplier = 0.F;
                                    }
                                }
                            }

                            if (audioIndex < channelMute.size() &&
                                channelMute[audioIndex])
                            {
                                volumeMultiplier = 0.F;
                            }

                            if (backwards)
                            {
                                auto tmp = audio::Audio::create(
                                    inputInfo, inSampleRate);
                                tmp->zero();

                                std::memcpy(
                                    tmp->getData(), audio->getData(),
                                    audio->getByteCount());
                                audio = tmp;
                                audios.push_back(audio);
                            }
                            audioDataP.push_back(
                                audio->getData() + dataByteOffset);
                            volumeScale.push_back(volumeMultiplier);
                            ++audioIndex;
                        }
                    }
                    if (audioDataP.empty())
                    {
                        volumeScale.push_back(0.F);
                        audioDataP.push_back(thread.silence->getData());
                    }

                    size_t inSamples = std::min(
                        p.player->getPlayerOptions().audioBufferFrameCount,
                        static_cast<size_t>(inSampleRate - inOffsetSamples));

                    if (backwards)
                    {
                        if (thread.backwardsSamples < inSamples)
                        {
                            inSamples = thread.backwardsSamples;
                        }

                        audio::reverse(
                            const_cast<uint8_t**>(audioDataP.data()),
                            audioDataP.size(), inSamples,
                            inputInfo.channelCount, inputInfo.dataType);
                    }

                    auto tmp = audio::Audio::create(inputInfo, inSamples);
                    tmp->zero();
                    audio::mix(
                        audioDataP.data(), audioDataP.size(), tmp->getData(),
                        volume, volumeScale, inSamples, inputInfo.channelCount,
                        inputInfo.dataType);

                    if (thread.resample)
                    {
                        thread.buffer.push_back(thread.resample->process(tmp));
                    }

                    if (backwards)
                    {
                        inOffsetSamples -= inSamples;
                        if (inOffsetSamples < 0)
                        {
                            seconds -= 1;
                            if (speedMultiplier < 1.0)
                                inOffsetSamples +=
                                    (inSampleRate * speedMultiplier);
                            else
                                inOffsetSamples += inSampleRate;
                            thread.backwardsSamples = static_cast<size_t>(
                                inSampleRate - inOffsetSamples);
                        }
                        else
                        {
                            thread.backwardsSamples = inSamples;
                        }
                    }
                    else
                    {
                        inOffsetSamples += inSamples;
                        if (inOffsetSamples >= inSampleRate)
                        {
                            inOffsetSamples -= inSampleRate;
                            seconds += 1;
                        }
                    }
                }

                const auto now = std::chrono::steady_clock::now();
                if (defaultSpeed ==
                        p.player->getTimeRange().duration().rate() &&
                    !mute && now >= muteTimeout &&
                    outSamples <= getSampleCount(thread.buffer))
                {
                    auto& NDI_audio_frame = p.thread.NDI_audio_frame;

                    // Send the output audio buffer to NDI.
                    // Submit the audio buffer
                    NDI_audio_frame.sample_rate = outSampleRate;
                    NDI_audio_frame.no_channels = channelCount;
                    NDI_audio_frame.no_samples = outSamples;

                    if (p.thread.outSamples != outSamples ||
                        p.thread.outChannelCount != channelCount ||
                        p.thread.outSampleRate != outSampleRate)
                    {
                        p.thread.outSamples = outSamples;
                        p.thread.outChannelCount = channelCount;
                        p.thread.outSampleRate = outSampleRate;

                        auto& NDI_audio_frame = p.thread.NDI_audio_frame;
                        free(NDI_audio_frame.p_data);
                        NDI_audio_frame.p_data = (float*)malloc(
                            sizeof(float) * channelCount * outSamples);
                    }

                    audio::move(
                        p.thread.buffer,
                        reinterpret_cast<uint8_t*>(NDI_audio_frame.p_data),
                        outSamples);

                    NDIlib_util_send_send_audio_interleaved_32f(
                        p.thread.NDI_send, &NDI_audio_frame);
                }

                thread.rtAudioCurrentFrame += outSamples;

                break;
            }
            default:
                break;
            }
        }

        void OutputDevice::_render(
            const device::DeviceConfig& config,
            const timeline::OCIOOptions& ocioOptions,
            const timeline::LUTOptions& lutOptions,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            const timeline::BackgroundOptions& backgroundOptions)
        {
            TLRENDER_P();

            // Create the offscreen buffer.
            const math::Size2i renderSize = p.thread.size;
            gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType =
                getColorBuffer(p.thread.outputPixelType);
            if (!displayOptions.empty())
            {
                offscreenBufferOptions.colorFilters =
                    displayOptions[0].imageFilters;
            }
            offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
            offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
            if (gl::doCreate(
                    p.thread.offscreenBuffer, renderSize,
                    offscreenBufferOptions))
            {
                p.thread.offscreenBuffer = gl::OffscreenBuffer::create(
                    renderSize, offscreenBufferOptions);
            }

            // Render the video.
            if (p.thread.offscreenBuffer)
            {
                gl::OffscreenBufferBinding binding(p.thread.offscreenBuffer);

                timeline::RenderOptions renderOptions;
                renderOptions.colorBuffer =
                    getColorBuffer(p.thread.outputPixelType);
                p.thread.render->begin(renderSize, renderOptions);
                p.thread.render->setOCIOOptions(ocioOptions);
                p.thread.render->setLUTOptions(lutOptions);

                math::Size2i sourceViewportSize = p.thread.sourceViewportSize;
                if (!sourceViewportSize.isValid())
                    sourceViewportSize = p.thread.size;

                math::Size2i targetViewportSize = p.thread.size;
                if (!targetViewportSize.isValid())
                    return;

                math::Vector2f transformOffset;
                math::Vector2i viewPos = p.thread.viewPos;
                double viewZoom = p.thread.viewZoom;

                const auto renderAspect = renderSize.getAspect();
                const auto viewportAspect = sourceViewportSize.getAspect();
                if (viewportAspect > 1.F)
                {
                    transformOffset.x = renderSize.w / 2.F;
                    transformOffset.y = renderSize.w / renderAspect / 2.F;
                }
                else
                {
                    transformOffset.x = renderSize.h * renderAspect / 2.F;
                    transformOffset.y = renderSize.h / 2.F;
                }

                math::Size2i targetRenderSize = renderSize;
                float scaleX = 1.0, scaleY = 1.0;
                if (p.thread.frameView)
                {
                    double zoom;
                    if (p.thread.rotateZ == 90.F || p.thread.rotateZ == 270.F)
                    {
                        zoom = targetViewportSize.h /
                               static_cast<double>(renderSize.w);
                        if (zoom * renderSize.h > targetViewportSize.w)
                        {
                            zoom = targetViewportSize.w /
                                   static_cast<double>(renderSize.h);
                        }

                        math::Vector2i rotatedCenter;
                        rotatedCenter.x = renderSize.h / 2;
                        rotatedCenter.y = renderSize.w / 2;

                        viewPos.x =
                            targetViewportSize.w / 2.0 - rotatedCenter.x * zoom;
                        viewPos.y =
                            targetViewportSize.h / 2.0 - rotatedCenter.y * zoom;
                    }
                    else
                    {
                        zoom = targetViewportSize.w /
                               static_cast<double>(renderSize.w);
                        if (zoom * renderSize.h > targetViewportSize.h)
                        {
                            zoom = targetViewportSize.h /
                                   static_cast<double>(renderSize.h);
                        }
                    }
                    const math::Vector2i c(
                        targetRenderSize.w / 2, targetRenderSize.h / 2);
                    viewPos.x = targetViewportSize.w / 2.0 - c.x * zoom;
                    viewPos.y = targetViewportSize.h / 2.0 - c.y * zoom;
                    viewZoom = zoom;
                    sourceViewportSize = renderSize;
                }
                const math::Matrix4x4f& translateMatrix = math::translate(
                    math::Vector3f(
                        viewPos.x,
                        static_cast<float>(
                            -viewPos.y - (renderSize.h * (viewZoom - 1.0))),
                        0.F));
                // Create scaling matrix for zoom
                const math::Matrix4x4f& zoomMatrix =
                    math::scale(math::Vector3f(viewZoom, viewZoom, 1.0f));

                // Create rotation matrix (must rotate around image center, not
                // 0,0).
                const math::Matrix4x4f& rotateMatrix =
                    math::rotateZ(-p.thread.rotateZ);

                // Create translation matrix for centering the image (image
                // origin at top-left)
                const math::Matrix4x4f& centeringMatrix = math::translate(
                    math::Vector3f(
                        -renderSize.w / 2.0f, -renderSize.h / 2.0f, 0.0f));

                // Create offset transform
                const math::Matrix4x4f& offsetTransformMatrix = math::translate(
                    math::Vector3f(transformOffset.x, transformOffset.y, 0.F));

                // Create orthographic projection matrix
                const math::Matrix4x4f& pm = math::ortho(
                    0.F, static_cast<float>(targetViewportSize.w), 0.F,
                    static_cast<float>(targetViewportSize.h), -1.F, 1.F);

                // Calculate aspect-correct scale
                math::Matrix4x4f centerTranslationMatrix;
                math::Matrix4x4f resizeScaleMatrix;

                if (!p.thread.frameView)
                {
                    // These calculations are fine?
                    float scaleX = static_cast<float>(targetViewportSize.w) /
                                   static_cast<float>(renderSize.w);
                    float scaleY = static_cast<float>(targetViewportSize.h) /
                                   static_cast<float>(renderSize.h);

                    // Use the smaller scale to fit within the viewport
                    float scale = std::min(scaleX, scaleY);

                    // Scale matrix with aspect-correct scale
                    resizeScaleMatrix =
                        math::scale(math::Vector3f(scale, scale, 1.0f));
                }
                if (!p.thread.videoData.empty())
                {
                    // This is almost fine
                    p.thread.render->setTransform(
                        pm * centerTranslationMatrix * resizeScaleMatrix *
                        translateMatrix * zoomMatrix * offsetTransformMatrix *
                        rotateMatrix * centeringMatrix);
                    p.thread.render->drawVideo(
                        p.thread.videoData,
                        timeline::getBoxes(
                            compareOptions.mode, p.thread.videoData),
                        imageOptions, displayOptions, compareOptions,
                        backgroundOptions);
                }
                if (p.thread.overlay)
                {
                    p.thread.render->setTransform(pm);
                    timeline::ImageOptions imageOptions;
                    imageOptions.alphaBlend =
                        timeline::AlphaBlend::Premultiplied;
                    p.thread.render->drawImage(
                        p.thread.overlay,
                        math::Box2i(
                            0, 0, p.thread.overlay->getWidth(),
                            p.thread.overlay->getHeight()),
                        image::Color4f(1.F, 1.F, 1.F), imageOptions);
                }

                p.thread.render->end();

                glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo);
                glPixelStorei(
                    GL_PACK_ALIGNMENT,
                    getPackPixelsAlign(p.thread.outputPixelType));
                glPixelStorei(
                    GL_PACK_SWAP_BYTES,
                    getPackPixelsSwap(p.thread.outputPixelType));
                glBindTexture(
                    GL_TEXTURE_2D, p.thread.offscreenBuffer->getColorID());
                glGetTexImage(
                    GL_TEXTURE_2D, 0,
                    getPackPixelsFormat(p.thread.outputPixelType),
                    getPackPixelsType(p.thread.outputPixelType), NULL);

                // std::cerr << "pm=" << pm << std::endl;
                // std::cerr << "centerTranslationMatrix=" <<
                // centerTranslationMatrix << std::endl; std::cerr <<
                // "resizeScaleMatrix=" << resizeScaleMatrix << std::endl;
                // std::cerr << "translateMatrix=" << translateMatrix <<
                // std::endl; std::cerr << "zoomMatrix=" << zoomMatrix <<
                // std::endl; std::cerr << "offsetTransformMatrix=" <<
                // offsetTransformMatrix << std::endl; std::cerr <<
                // "rotateMatrix=" << rotateMatrix << std::endl; std::cerr <<
                // "centeringMatrix=" << centeringMatrix << std::endl;
            }
        }

        void OutputDevice::_read(const device::DeviceConfig& config)
        {
            TLRENDER_P();

            auto& video_frame = p.thread.NDI_video_frame;

            glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo);
            if (void* pboP = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY))
            {
                copyPackPixels(
                    video_frame.p_data, pboP, p.thread.size,
                    p.thread.outputPixelType);
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            }
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

            std::shared_ptr<image::HDRData> hdrData;
            switch (p.thread.hdrMode)
            {
            case device::HDRMode::FromFile:
            case device::HDRMode::Custom:
                if (p.thread.videoData.empty())
                    return;
                hdrData = device::getHDRData(p.thread.videoData[0]);
                break;
            case device::HDRMode::Count:
            case device::HDRMode::kNone:
                break;
            }

            if (hdrData && !config.noMetadata)
            {
                std::string primariesName = "bt_2020";
                std::string transferName = "bt_2020";
                std::string matrixName = "bt_2020";
                const auto& primaries = hdrData->primaries;
                if (is_equal(primaries[0].x, 0.708F) &&
                    is_equal(primaries[0].y, 0.292F) &&
                    is_equal(primaries[1].x, 0.170F) &&
                    is_equal(primaries[1].y, 0.797F) &&
                    is_equal(primaries[2].x, 0.131F) &&
                    is_equal(primaries[2].y, 0.046F) &&
                    is_equal(primaries[3].x, 0.3127F) &&
                    is_equal(primaries[3].y, 0.3290F))
                {
                    primariesName = "bt_2020";
                }
                else if (
                    is_equal(primaries[0].x, 0.640F) &&
                    is_equal(primaries[0].y, 0.330F) &&
                    is_equal(primaries[1].x, 0.300F) &&
                    is_equal(primaries[1].y, 0.600F) &&
                    is_equal(primaries[2].x, 0.150F) &&
                    is_equal(primaries[2].y, 0.060F) &&
                    is_equal(primaries[3].x, 0.3127F) &&
                    is_equal(primaries[3].y, 0.3290F))
                {
                    primariesName = "bt_709";
                }
                else if (
                    is_equal(primaries[0].x, 0.630F) &&
                    is_equal(primaries[0].y, 0.340F) &&
                    is_equal(primaries[1].x, 0.310F) &&
                    is_equal(primaries[1].y, 0.595F) &&
                    is_equal(primaries[2].x, 0.155F) &&
                    is_equal(primaries[2].y, 0.070F) &&
                    is_equal(primaries[3].x, 0.3127F) &&
                    is_equal(primaries[3].y, 0.3290F))
                {
                    primariesName = "bt_601";
                }
                else if (
                    is_equal(primaries[0].x, 0.680F) &&
                    is_equal(primaries[0].y, 0.320F) &&
                    is_equal(primaries[1].x, 0.265F) &&
                    is_equal(primaries[1].y, 0.690F) &&
                    is_equal(primaries[2].x, 0.150F) &&
                    is_equal(primaries[2].y, 0.060F) &&
                    is_equal(primaries[3].x, 0.314F) &&
                    is_equal(primaries[3].y, 0.351F))
                {
                    // primariesName = "dci-p3";
                    primariesName = "bt_2020";
                    if (auto context = p.context.lock())
                    {
                        auto logSystem = context->getLogSystem();
                        logSystem->print(
                            "tl::ndi::OutputDevice",
                            string::Format(
                                "DCI-P3 (Theatrical) "
                                "primaries unsupported. "
                                "Using {0} for NDI compatible "
                                "devices.")
                                .arg(primariesName),
                            log::Type::kStatus, kModule);
                    }
                }
                else if (
                    is_equal(primaries[0].x, 0.680F) &&
                    is_equal(primaries[0].y, 0.320F) &&
                    is_equal(primaries[1].x, 0.265F) &&
                    is_equal(primaries[1].y, 0.690F) &&
                    is_equal(primaries[2].x, 0.150F) &&
                    is_equal(primaries[2].y, 0.060F) &&
                    is_equal(primaries[3].x, 0.3127F) &&
                    is_equal(primaries[3].y, 0.3290F))
                {
                    // primariesName = "dci-p3 (D65)";
                    primariesName = "bt_2020";
                    if (auto context = p.context.lock())
                    {
                        auto logSystem = context->getLogSystem();
                        logSystem->print(
                            "tl::ndi::OutputDevice",
                            string::Format(
                                "DCI-P3 (D65) "
                                "primaries unsupported. "
                                "Using {0} for NDI compatible "
                                "devices.")
                                .arg(primariesName),
                            log::Type::kStatus, kModule);
                    }
                }
                else
                {
                    primariesName = "bt_2020";
                    if (auto context = p.context.lock())
                    {
                        auto logSystem = context->getLogSystem();
                        logSystem->print(
                            "tl::ndi::OutputDevice",
                            string::Format(
                                "Unknown primaries:\n"
                                "{0}, {1}\n"
                                "{2}, {3}\n"
                                "{4}, {5}\n"
                                "{6}, {7}\n"
                                "Using {8} for NDI compatible "
                                "devices.")
                                .arg(primaries[0].x)
                                .arg(primaries[0].y)
                                .arg(primaries[1].x)
                                .arg(primaries[1].y)
                                .arg(primaries[2].x)
                                .arg(primaries[2].y)
                                .arg(primaries[3].x)
                                .arg(primaries[3].y)
                                .arg(primariesName),
                            log::Type::kStatus, kModule);
                    }
                }

                switch (hdrData->eotf)
                {
                case image::EOTFType::EOTF_BT601:
                    transferName = "bt_601";
                    matrixName = "bt_601";
                    break;
                case image::EOTFType::EOTF_BT709:
                    transferName = "bt_709";
                    matrixName = "bt_709";
                    break;
                case image::EOTFType::EOTF_BT2020:
                    transferName = "bt_2020";
                    matrixName = "bt_2020";
                    break;
                case image::EOTFType::EOTF_BT2100_HLG:
                    primariesName = "bt_2100";
                    transferName = "bt_2100_hlg";
                    matrixName = "bt_2100";
                    break;
                case image::EOTFType::EOTF_BT2100_PQ:
                    primariesName = "bt_2100";
                    transferName = "bt_2100_pq";
                    matrixName = "bt_2100";
                    break;
                default:
                    break;
                }
                if (auto context = p.context.lock())
                {
                    auto logSystem = context->getLogSystem();
                    logSystem->print(
                        "tl::ndi::OutputDevice",
                        string::Format(
                            "primaries={0} transfer={1} "
                            "matrix={2}.")
                            .arg(primariesName)
                            .arg(transferName)
                            .arg(matrixName),
                        log::Type::Message, kModule);
                }

                locale::SetAndRestore locale;

                nlohmann::json j = *hdrData;
                const std::string mrv2_json = escape_quotes_for_xml(j.dump());
                p.thread.metadata = string::Format(
                                        "<ndi_color_info "
                                        " transfer=\"{0}\" "
                                        " matrix=\"{1}\" "
                                        " primaries=\"{2}\" "
                                        " mrv2=\"{3}\" "
                                        "/>\n")
                                        .arg(transferName)
                                        .arg(matrixName)
                                        .arg(primariesName)
                                        .arg(mrv2_json);
                video_frame.p_metadata = p.thread.metadata.c_str();
            }
            else
            {
                video_frame.p_metadata = nullptr;
            }

            if (p.thread.NDI_send && video_frame.p_data)
            {
                NDIlib_send_send_video(p.thread.NDI_send, &video_frame);
            }
        }
    } // namespace ndi
} // namespace tl

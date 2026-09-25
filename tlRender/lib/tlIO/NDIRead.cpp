// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024 Gonzalo Garramuño
// All rights reserved.

#include <fstream>

#include <tlIO/IOMacros.h>
#include <tlIO/NDIReadPrivate.h>

#include <tlCore/Assert.h>
#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

namespace
{
    const char* kModule = "ndi";
}

namespace tl
{
    namespace ndi
    {
        std::mutex NDI_recv_mutex;
        NDIlib_recv_instance_t NDI_recv = nullptr;

        void VideoRead::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            IVideoRead::_init(path, memory, options, logSystem);

            TLRENDER_P();

            std::ifstream s(path.get());

            if (s.is_open())
            {
                nlohmann::json j;
                s >> j;
                p.options = j;
                s.close();
            }

            const NDIlib_source_t* sources = nullptr;

            NDIlib_find_instance_t NDI_find = nullptr;
            NDI_find = NDIlib_find_create();
            if (!NDI_find)
                throw std::runtime_error("Could not create NDI find");

            using namespace std::chrono;
            for (const auto start = high_resolution_clock::now();
                 high_resolution_clock::now() - start < seconds(3);)
            {
                // Wait up 1000 milliseconds to check for new sources to be
                // added or removed
                if (!NDIlib_find_wait_for_sources(NDI_find, 1000))
                {
                    break;
                }
            }

            uint32_t no_sources = 0;

            // Get the updated list of sources
            while (!no_sources)
            {
                sources =
                    NDIlib_find_get_current_sources(NDI_find, &no_sources);
            }

            int ndiSource = -1;
            for (int i = 0; i < no_sources; ++i)
            {
                if (sources[i].p_ndi_name == p.options.sourceName)
                {
                    ndiSource = i;
                    break;
                }
            }

            if (ndiSource < 0)
            {
                throw std::runtime_error("Could not find a valid source");
            }

            //
            const auto& NDIsource = sources[ndiSource];

            // We now have at least one source,
            // so we create a receiver to look at it.
            NDIlib_recv_create_t recv_desc;
            recv_desc.color_format = NDIlib_recv_color_format_fastest;

            // These are 16-bit formats, but they seem broken.
            // I tried them both with FFmpeg's libswscale and with my own
            // code:
            //      No alpha channel : P216, or UYVY
            //      Alpha channel    : PA16 or UYVA
            if (p.options.bestFormat)
                recv_desc.color_format = NDIlib_recv_color_format_best;

            recv_desc.bandwidth = NDIlib_recv_bandwidth_highest;
            recv_desc.allow_video_fields = false;
            recv_desc.source_to_connect_to = NDIsource;

            if (!NDI_recv)
            {
                NDI_recv = NDIlib_recv_create(&recv_desc);
                if (!NDI_recv)
                    throw std::runtime_error("Could not create NDI receiver");
            }

            // Get the name of the source for debugging purposes
            NDIlib_tally_t tally_state;
            tally_state.on_program = true;
            tally_state.on_preview = false;

            /* Set tally */
            NDIlib_recv_set_tally(NDI_recv, &tally_state);

            double fps = 24.0;
            NDIlib_video_frame_t v;
            NDIlib_frame_type_e type_e = NDIlib_frame_type_none;

            p.videoThread.currentTime = OTIO_NS::RationalTime(0.0, fps);

            // Preroll to find video and (potentially) video stream
            unsigned audioCounter = 0;
            bool hasVideoStream = false;
            while (type_e != NDIlib_frame_type_error &&
                   audioCounter < 120 && !hasVideoStream)
            {
                type_e = NDIlib_recv_capture(NDI_recv, &v,
                                             nullptr, nullptr, 50);
                if (type_e == NDIlib_frame_type_video)
                {
                    if (!p.readVideo)
                    {
                        p.readVideo = std::make_shared<ReadVideo>(
                            recv_desc, v, _logSystem, p.options);
                        const auto& videoInfo = p.readVideo->getInfo();
                        if (videoInfo.isValid())
                        {
                            p.info.video.push_back(videoInfo);
                            p.info.videoTime = p.readVideo->getTimeRange();
                        }
                        p.videoThread.currentTime =
                            p.info.videoTime.has_value() ?
                            p.info.videoTime->start_time() :
                            OTIO_NS::RationalTime(0.F, 24.F);

                        p.videoThread.logTimer =
                            std::chrono::steady_clock::now();
                        p.videoThread.thread = std::thread(
                            [this]
                            {
                                TLRENDER_P();

                                _run();

                                cancelRequests();
                            });

                        hasVideoStream = true;

                        // Release this video frame
                        NDIlib_recv_free_video(NDI_recv, &v);
                    }
                    else if (type_e == NDIlib_frame_type_audio)
                    {
                        ++audioCounter;
                    }
                    else if (type_e == NDIlib_frame_type_error)
                    {
                        LOG_ERROR("Error decoding frame");
                    }
                }
            }

            if (!hasVideoStream)
            {
                LOG_ERROR("Video Stream not found");
            }
        }

        VideoRead::VideoRead() :
            _p(new Private)
        {
            TLRENDER_P();

            // Fallback if no one creates a cache
            p.cache = io::Cache::create();
            p.cache->setMax(4 * memory::gigabyte);
        }

        VideoRead::~VideoRead()
        {
            TLRENDER_P();

            // Stop the video thread
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                p.videoThread.running = false;
            }
            p.videoThread.cv.notify_one();
            if (p.videoThread.thread.joinable())
            {
                p.videoThread.thread.join();
            }

            // We destroy receiver
            if (NDI_recv)
                NDIlib_recv_destroy(NDI_recv);
            NDI_recv = nullptr;

            // We destroy the finder
            if (p.NDI_find)
                NDIlib_find_destroy(p.NDI_find);
            p.NDI_find = nullptr;
        }

        void VideoRead::setCache(const std::shared_ptr<io::Cache>& cache)
        {
            TLRENDER_P();
            if (cache)
            {
                p.cache = cache;
            }
        }

        std::shared_ptr<VideoRead> VideoRead::create(
            const file::Path& path, const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            return VideoRead::create(path, {}, options, logSystem);
        }

        std::shared_ptr<VideoRead> VideoRead::create(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<VideoRead>(new VideoRead);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        std::future<io::Info> VideoRead::getInfo()
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::InfoRequest>();
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                if (!p.videoMutex.stopped)
                {
                    valid = true;
                    p.videoMutex.infoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.videoThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::Info());
            }
            return future;
        }

        std::future<io::VideoData> VideoRead::readVideo(
            const OTIO_NS::RationalTime& time, const io::Options& options)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            request->options = io::merge(options, _options);
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                if (!p.videoMutex.stopped)
                {
                    valid = true;
                    p.videoMutex.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.videoThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::VideoData());
            }
            return future;
        }

        void VideoRead::_run()
        {
            TLRENDER_P();
            p.videoThread.running = true;
            while (p.videoThread.running)
            {
                // Check requests.
                std::shared_ptr<Private::VideoRequest> videoRequest;
                std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
                {
                    std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                    if (p.videoThread.cv.wait_for(
                            lock,
                            std::chrono::milliseconds(p.options.requestTimeout),
                            [this]
                            {
                                return !_p->videoMutex.infoRequests.empty() ||
                                       !_p->videoMutex.videoRequests.empty();
                            }))
                    {
                        infoRequests = std::move(p.videoMutex.infoRequests);
                        if (!p.videoMutex.videoRequests.empty())
                        {
                            videoRequest = p.videoMutex.videoRequests.front();
                            p.videoMutex.videoRequests.pop_front();
                        }
                    }
                }

                // Information requests.
                for (auto& request : infoRequests)
                {
                    request->promise.set_value(p.info);
                }

                // Check the cache.
                io::VideoData videoData;
                if (videoRequest && p.cache)
                {
                    const std::string cacheKey = io::getVideoCacheKey(
                        _path, videoRequest->time, _options,
                        videoRequest->options);
                    if (p.cache->getVideo(cacheKey, videoData))
                    {
                        videoRequest->promise.set_value(videoData);
                        videoRequest.reset();
                    }
                }

                // Seek.
                if (videoRequest && !videoRequest->time.strictly_equal(
                                        p.videoThread.currentTime))
                {
                    p.videoThread.currentTime = videoRequest->time;
                }

                // Process.
                while (videoRequest && p.readVideo->isBufferEmpty() &&
                       p.readVideo->process(p.videoThread.currentTime))
                    ;

                // Video request.
                if (videoRequest)
                {
                    io::VideoData data;
                    data.time = videoRequest->time;
                    if (!p.readVideo->isBufferEmpty())
                    {
                        data.image = p.readVideo->popBuffer();
                    }
                    videoRequest->promise.set_value(data);
                    _addToCache(data, videoRequest->options);

                    p.videoThread.currentTime += OTIO_NS::RationalTime(
                        1.0, p.info.videoTime->duration().rate());
                }

                // Logging.
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff =
                        now - p.videoThread.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.videoThread.logTimer = now;

                        if (auto logSystem = _logSystem.lock())
                        {
                            const std::string id =
                                string::Format("tl::io::ndi::Read {0}")
                                    .arg(this);
                            size_t requestsSize = 0;
                            {
                                std::unique_lock<std::mutex> lock(
                                    p.videoMutex.mutex);
                                requestsSize =
                                    p.videoMutex.videoRequests.size();
                            }
                            logSystem->print(
                                id, string::Format("\n"
                                                   "    Path: {0}\n"
                                                   "    Video requests: {1}")
                                        .arg(_path.get())
                                        .arg(requestsSize));
                        }
                    }
                }
            }
        }

        void VideoRead::cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                infoRequests = std::move(p.videoMutex.infoRequests);
                videoRequests = std::move(p.videoMutex.videoRequests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(io::Info());
            }
            for (auto& request : videoRequests)
            {
                request->promise.set_value(io::VideoData());
            }
        }

        void VideoRead::_addToCache(
            io::VideoData& data, const io::Options& options)
        {
            TLRENDER_P();
            const std::string cacheKey =
                io::getVideoCacheKey(_path, data.time, _options, options);
            p.cache->addVideo(cacheKey, data);
        }

        void AudioRead::_init(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            TLRENDER_P();

            IAudioRead::_init(path, memory, options, logSystem);

            std::ifstream s(path.get());

            if (s.is_open())
            {
                nlohmann::json j;
                s >> j;
                p.options = j;
                s.close();
            }


            const NDIlib_source_t* sources = nullptr;
            NDIlib_find_instance_t NDI_find = nullptr;

            NDI_find = NDIlib_find_create();
            if (!NDI_find)
                throw std::runtime_error("Could not create NDI find");

            using namespace std::chrono;
            for (const auto start = high_resolution_clock::now();
                 high_resolution_clock::now() - start < seconds(3);)
            {
                // Wait up till 1 second to check for new sources to be added or
                // removed
                if (!NDIlib_find_wait_for_sources(NDI_find, 1000))
                {
                    break;
                }
            }

            uint32_t no_sources = 0;

            // Get the updated list of sources
            while (!no_sources)
            {
                sources =
                    NDIlib_find_get_current_sources(NDI_find, &no_sources);
            }

            int ndiSource = -1;
            for (int i = 0; i < no_sources; ++i)
            {
                if (sources[i].p_ndi_name == p.options.sourceName)
                {
                    ndiSource = i;
                    break;
                }
            }

            if (ndiSource < 0)
            {
                throw std::runtime_error("Could not find a valid source");
            }

            //
            const auto& NDIsource = sources[ndiSource];

            // We now have at least one source,
            // so we create a receiver to look at it.
            NDIlib_recv_create_t recv_desc;
            recv_desc.color_format = NDIlib_recv_color_format_fastest;

            // These are 16-bit formats, but they seem broken.
            // I tried them both with FFmpeg's libswscale and with my own
            // code:
            //      No alpha channel : P216, or UYVY
            //      Alpha channel    : PA16 or UYVA
            if (p.options.bestFormat)
                recv_desc.color_format = NDIlib_recv_color_format_best;

            recv_desc.bandwidth = NDIlib_recv_bandwidth_highest;
            recv_desc.allow_video_fields = false;
            recv_desc.source_to_connect_to = NDIsource;

            if (!NDI_recv)
            {
                NDI_recv = NDIlib_recv_create(&recv_desc);
                if (!NDI_recv)
                    throw std::runtime_error("Could not create NDI receiver");
            }

            // Get the name of the source for debugging purposes
            NDIlib_tally_t tally_state;
            tally_state.on_program = true;
            tally_state.on_preview = false;

            /* Set tally */
            NDIlib_recv_set_tally(NDI_recv, &tally_state);

            double fps = 24.0;
            NDIlib_audio_frame_t a;
            NDIlib_frame_type_e type_e = NDIlib_frame_type_none;

            p.audioThread.currentTime = OTIO_NS::RationalTime(0.0, 48000.0);

            // Preroll to find video and (potentially) audio stream
            unsigned videoCounter = 0;
            bool hasAudioStream = false;

            while (type_e != NDIlib_frame_type_error && videoCounter < 1200 &&
                   !hasAudioStream)
            {
                type_e = NDIlib_recv_capture(NDI_recv, nullptr,
                                             &a, nullptr, 50);
                if (type_e == NDIlib_frame_type_audio)
                {
                    if (!p.readAudio)
                    {
                        p.readAudio = std::make_shared<ReadAudio>(
                            recv_desc, a, _logSystem, p.options);
                        p.info.audio = p.readAudio->getInfo();
                        p.info.audioTime = p.readAudio->getTimeRange();
                        p.audioThread.currentTime =
                            p.info.audioTime->start_time();
                        p.audioThread.logTimer =
                            std::chrono::steady_clock::now();

                        p.audioThread.thread = std::thread(
                            [this]
                            {
                                TLRENDER_P();

                                _run();

                                cancelRequests();
                            });

                    }

                    NDIlib_recv_free_audio(NDI_recv, &a);

                    hasAudioStream = true;
                }
                else if (type_e == NDIlib_frame_type_video)
                {
                    ++videoCounter;
                }
                else if (type_e == NDIlib_frame_type_error)
                {
                    LOG_ERROR("Error decoding frame");
                }
            }

            if (!hasAudioStream)
            {
                LOG_ERROR("Audio Stream not found");
            }
        }

        AudioRead::AudioRead() :
            _p(new Private)
        {
        }

        AudioRead::~AudioRead()
        {
            TLRENDER_P();

            // Stop the audio thread
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                p.audioThread.running = false;
            }
            p.audioThread.cv.notify_one();
            if (p.audioThread.thread.joinable())
            {
                p.audioThread.thread.join();
            }

            // We destroy receiver
            if (NDI_recv)
                NDIlib_recv_destroy(NDI_recv);
            NDI_recv = nullptr;

            // We destroy the finder
            if (p.NDI_find)
                NDIlib_find_destroy(p.NDI_find);
            p.NDI_find = nullptr;
        }

        std::shared_ptr<AudioRead> AudioRead::create(
            const file::Path& path, const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = AudioRead::create(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<AudioRead> AudioRead::create(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<AudioRead>(new AudioRead);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        std::future<io::Info> AudioRead::getInfo()
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::InfoRequest>();
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                if (!p.audioMutex.stopped)
                {
                    valid = true;
                    p.audioMutex.infoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.audioThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::Info());
            }
            return future;
        }

        std::future<io::AudioData> AudioRead::readAudio(
            const OTIO_NS::TimeRange& timeRange, const io::Options& options)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->timeRange = timeRange;
            request->options = io::merge(options, _options);
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                if (!p.audioMutex.stopped)
                {
                    valid = true;
                    p.audioMutex.audioRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.audioThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::AudioData());
            }
            return future;
        }

        void AudioRead::_run()
        {
            TLRENDER_P();
            p.audioThread.running = true;
            while (p.audioThread.running)
            {
                std::shared_ptr<Private::AudioRequest> request;
                const double sampleRate = p.info.audioTime->duration().rate();
                size_t requestSampleCount = 0;
                bool seek = false;
                // Check requests.
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    if (p.audioThread.cv.wait_for(
                            lock,
                            std::chrono::milliseconds(p.options.requestTimeout),
                            [this]
                            { return !_p->audioMutex.infoRequests.empty() ||
                                     !_p->audioMutex.audioRequests.empty(); })
                        )
                    {
                        if (!p.audioMutex.audioRequests.empty())
                        {
                            request = p.audioMutex.audioRequests.front();
                            requestSampleCount =
                                request->timeRange.duration()
                                    .rescaled_to(p.info.audio.sampleRate)
                                    .value();
                            p.audioMutex.audioRequests.pop_front();
                            if (!request->timeRange.start_time().strictly_equal(
                                    p.audioThread.currentTime))
                            {
                                seek = true;
                                p.audioThread.currentTime =
                                    request->timeRange.start_time();
                            }
                        }
                    }
                }

                io::AudioData audioData;
                if (request && p.cache)
                {
                    const std::string cacheKey = io::getAudioCacheKey(
                        _path, request->timeRange, _options,
                        request->options);
                    if (p.cache->getAudio(cacheKey, audioData))
                    {
                        request->promise.set_value(audioData);
                        request.reset();
                    }
                }


                // Seek.
                if (seek && request)
                {
                    p.readAudio->seek(p.audioThread.currentTime);
                }

                const size_t sampleCount =
                    requestSampleCount
                        ? requestSampleCount
                        : p.options.audioBufferSize
                              .rescaled_to(p.info.audio.sampleRate)
                              .value();

                // Process.
                bool intersects = false;
                if (request)
                {
                    intersects =
                        request->timeRange.intersects(p.info.audioTime.value());
                }

                while (request && intersects &&
                       p.readAudio->getBufferSize() <
                           request->timeRange.duration()
                               .rescaled_to(p.info.audio.sampleRate)
                               .value() &&
                       p.readAudio->process(
                           p.audioThread.currentTime, sampleCount))
                    ;

                // Handle request.
                if (request)
                {
                    audioData.time = request->timeRange.start_time();
                    audioData.audio = audio::Audio::create(
                        p.info.audio, request->timeRange.duration().value());
                    audioData.audio->zero();
                    if (intersects)
                    {
                        size_t offset = 0;
                        if (audioData.time < p.info.audioTime->start_time())
                        {
                            offset =
                                (p.info.audioTime->start_time() - audioData.time)
                                    .value();
                        }
                        p.readAudio->bufferCopy(
                            audioData.audio->getData() +
                                offset * p.info.audio.getByteCount(),
                            audioData.audio->getSampleCount() - offset);
                    }
                    request->promise.set_value(audioData);
                    _addToCache(audioData, request->timeRange,
                                request->options);
                    p.audioThread.currentTime += request->timeRange.duration();
                }

                // Logging.
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff =
                        now - p.audioThread.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.audioThread.logTimer = now;

                        if (auto logSystem = _logSystem.lock())
                        {
                            const std::string id =
                                string::Format("tl::io::ndi::Read {0}")
                                    .arg(this);
                            size_t requestsSize = 0;
                            {
                                std::unique_lock<std::mutex> lock(
                                    p.audioMutex.mutex);
                                requestsSize = p.audioMutex.audioRequests.size();
                            }
                            logSystem->print(
                                id, string::Format("\n"
                                                   "    Path: {0}\n"
                                                   "    Audio requests: {1}")
                                        .arg(_path.get())
                                        .arg(requestsSize));
                        }
                    }
                }
            }
        }


        void AudioRead::cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                infoRequests = std::move(p.audioMutex.infoRequests);
                audioRequests = std::move(p.audioMutex.audioRequests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(io::Info());
            }
            for (auto& request : audioRequests)
            {
                request->promise.set_value(io::AudioData());
            }
        }

        void AudioRead::setCache(const std::shared_ptr<io::Cache>& cache)
        {
            TLRENDER_P();
            if (cache)
            {
                p.cache = cache;
            }
        }

        void AudioRead::_addToCache(
            io::AudioData& data, const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
        {
            TLRENDER_P();
            const std::string cacheKey =
                io::getAudioCacheKey(_path, timeRange, _options, options);
            p.cache->addAudio(cacheKey, data);
        }
    } // namespace ndi
} // namespace tl

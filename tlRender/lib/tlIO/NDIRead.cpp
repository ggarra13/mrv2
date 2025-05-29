// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024 Gonzalo Garramuño
// All rights reserved.

#include <fstream>

#include <tlIO/FFmpegMacros.h>
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
        std::string Read::Private::sourceName;

        void Read::_init(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            IRead::_init(path, memory, options, cache, logSystem);

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

            p.NDI_find = NDIlib_find_create();
            if (!p.NDI_find)
                throw std::runtime_error("Could not create NDI find");

            using namespace std::chrono;
            for (const auto start = high_resolution_clock::now();
                 high_resolution_clock::now() - start < seconds(3);)
            {
                // Wait up till 1 second to check for new sources to be added or
                // removed
                if (!NDIlib_find_wait_for_sources(
                        p.NDI_find, 1000 /* milliseconds */))
                {
                    break;
                }
            }

            uint32_t no_sources = 0;

            // Get the updated list of sources
            while (!no_sources)
            {
                sources =
                    NDIlib_find_get_current_sources(p.NDI_find, &no_sources);
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

            p.NDI_recv = NDIlib_recv_create(&recv_desc);
            if (!p.NDI_recv)
                throw std::runtime_error("Could not create NDI receiver");

            // Get the name of the source for debugging purposes
            NDIlib_tally_t tally_state;
            tally_state.on_program = true;
            tally_state.on_preview = false;

            /* Set tally */
            NDIlib_recv_set_tally(p.NDI_recv, &tally_state);

            double fps = 24.0;
            NDIlib_video_frame_t v;
            NDIlib_audio_frame_t a;
            NDIlib_frame_type_e type_e = NDIlib_frame_type_none;

            p.audioThread.currentTime = otime::RationalTime(0.0, 48000.0);
            p.videoThread.currentTime = otime::RationalTime(0.0, fps);

            // Preroll to find video and (potentially) audio stream
            unsigned videoCounter = 0;
            bool noAudio = p.options.noAudio;
            if (noAudio)
                LOG_STATUS("NDI Stream with no Audio");
            bool hasStreams = false;
            while (type_e != NDIlib_frame_type_error && !hasStreams)
            {
                type_e = NDIlib_recv_capture(p.NDI_recv, &v, &a, nullptr, 50);
                if (type_e == NDIlib_frame_type_video)
                {
                    if (!p.readVideo)
                    {
                        p.readVideo = std::make_shared<ReadVideo>(
                            p.options.sourceName, NDIsource, recv_desc, v,
                            _logSystem, p.options);
                        const auto& videoInfo = p.readVideo->getInfo();
                        if (videoInfo.isValid())
                        {
                            p.info.video.push_back(videoInfo);
                            p.info.videoTime = p.readVideo->getTimeRange();
                        }
                        p.videoThread.currentTime =
                            p.info.videoTime.start_time();

                        p.videoThread.logTimer =
                            std::chrono::steady_clock::now();

                        p.videoThread.thread = std::thread(
                            [this]
                            {
                                TLRENDER_P();

                                _videoThread();

                                {
                                    std::unique_lock<std::mutex> lock(
                                        p.videoMutex.mutex);
                                    p.videoMutex.stopped = true;
                                }
                                _cancelVideoRequests();
                            });
                    }

                    ++videoCounter;
                    if (videoCounter > 60 && !p.readAudio)
                        noAudio = true;

                    // Release this video frame
                    NDIlib_recv_free_video(p.NDI_recv, &v);
                }
                else if (type_e == NDIlib_frame_type_audio)
                {
                    if (!p.readAudio && !noAudio)
                    {
                        p.readAudio = std::make_shared<ReadAudio>(
                            p.options.sourceName, NDIsource, a, _logSystem,
                            p.options);
                        p.info.audio = p.readAudio->getInfo();
                        p.info.audioTime = p.readAudio->getTimeRange();
                        p.audioThread.currentTime =
                            p.info.audioTime.start_time();
                        p.audioThread.logTimer =
                            std::chrono::steady_clock::now();

                        p.audioThread.thread = std::thread(
                            [this]
                            {
                                TLRENDER_P();

                                _audioThread();

                                {
                                    std::unique_lock<std::mutex> lock(
                                        p.audioMutex.mutex);
                                    p.audioMutex.stopped = true;
                                }
                                _cancelAudioRequests();
                            });
                    }

                    NDIlib_recv_free_audio(p.NDI_recv, &a);
                }

                hasStreams = p.readVideo && (p.readAudio || noAudio);
            }

            // We destroy receiver
            NDIlib_recv_destroy(p.NDI_recv);
            p.NDI_recv = nullptr;

            // We destroy the finder
            NDIlib_find_destroy(p.NDI_find);
            p.NDI_find = nullptr;
        }

        Read::Read() :
            _p(new Private)
        {
        }

        Read::~Read()
        {
            TLRENDER_P();

            p.audioThread.running = false;
            if (p.readAudio)
                p.readAudio->stop();
            if (p.audioThread.thread.joinable())
            {
                p.audioThread.thread.join();
            }

            p.videoThread.running = false;
            if (p.readVideo)
                p.readVideo->stop();
            if (p.videoThread.thread.joinable())
            {
                p.videoThread.thread.join();
            }
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path, const io::Options& options,
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, cache, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, cache, logSystem);
            return out;
        }

        std::future<io::Info> Read::getInfo()
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

        std::future<io::VideoData> Read::readVideo(
            const otime::RationalTime& time, const io::Options& options)
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

        std::future<io::AudioData> Read::readAudio(
            const otime::TimeRange& timeRange, const io::Options& options)
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
                    p.audioMutex.requests.push_back(request);
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

        void Read::cancelRequests()
        {
            _cancelVideoRequests();
            _cancelAudioRequests();
        }

        void Read::_videoThread()
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
                if (videoRequest && _cache)
                {
                    const std::string cacheKey = io::getVideoCacheKey(
                        _path, videoRequest->time, _options,
                        videoRequest->options);
                    if (_cache->getVideo(cacheKey, videoData))
                    {
                        p.videoThread.currentTime = videoRequest->time;
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

                    if (_cache)
                    {
                        const std::string cacheKey = io::getVideoCacheKey(
                            _path, videoRequest->time, _options,
                            videoRequest->options);
                        _cache->addVideo(cacheKey, data);
                    }

                    p.videoThread.currentTime += otime::RationalTime(
                        1.0, p.info.videoTime.duration().rate());
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

        void Read::_audioThread()
        {
            TLRENDER_P();
            p.audioThread.running = true;
            while (p.audioThread.running)
            {
                std::shared_ptr<Private::AudioRequest> request;
                const double sampleRate = p.info.audioTime.duration().rate();
                size_t requestSampleCount = 0;
                bool seek = false;
                // Check requests.
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    if (p.audioThread.cv.wait_for(
                            lock,
                            std::chrono::milliseconds(p.options.requestTimeout),
                            [this]
                            { return !_p->audioMutex.requests.empty(); }))
                    {
                        if (!p.audioMutex.requests.empty())
                        {
                            request = p.audioMutex.requests.front();
                            requestSampleCount =
                                request->timeRange.duration()
                                    .rescaled_to(p.info.audio.sampleRate)
                                    .value();
                            p.audioMutex.requests.pop_front();
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

                // Check the cache.
                io::AudioData audioData;
                if (request && _cache)
                {
                    const std::string cacheKey = io::getAudioCacheKey(
                        _path, request->timeRange, _options, request->options);
                    if (_cache->getAudio(cacheKey, audioData))
                    {
                        request->promise.set_value(audioData);
                        request.reset();
                    }
                }

                // Seek.
                if (seek)
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
                        request->timeRange.intersects(p.info.audioTime);
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
                    io::AudioData audioData;
                    audioData.time = request->timeRange.start_time();
                    audioData.audio = audio::Audio::create(
                        p.info.audio, request->timeRange.duration().value());
                    audioData.audio->zero();
                    if (intersects)
                    {
                        size_t offset = 0;
                        if (audioData.time < p.info.audioTime.start_time())
                        {
                            offset =
                                (p.info.audioTime.start_time() - audioData.time)
                                    .value();
                        }
                        p.readAudio->bufferCopy(
                            audioData.audio->getData() +
                                offset * p.info.audio.getByteCount(),
                            audioData.audio->getSampleCount() - offset);
                    }
                    request->promise.set_value(audioData);

                    if (_cache)
                    {
                        const std::string cacheKey = io::getAudioCacheKey(
                            _path, request->timeRange, _options,
                            request->options);
                        _cache->addAudio(cacheKey, audioData);
                    }

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
                                requestsSize = p.audioMutex.requests.size();
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

        void Read::_cancelVideoRequests()
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

        void Read::_cancelAudioRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::AudioRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                requests = std::move(p.audioMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(io::AudioData());
            }
        }
    } // namespace ndi
} // namespace tl

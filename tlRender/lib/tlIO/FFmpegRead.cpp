// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#include <tlIO/FFmpegReadPrivate.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>

extern "C"
{
#include <libavutil/opt.h>

} // extern "C"

namespace tl
{
    namespace ffmpeg
    {
        AVIOBufferData::AVIOBufferData() {}

        AVIOBufferData::AVIOBufferData(const uint8_t* p, size_t size) :
            p(p),
            size(size)
        {
        }

        int avIOBufferRead(void* opaque, uint8_t* buf, int bufSize)
        {
            AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

            const int64_t remaining = bufferData->size - bufferData->offset;
            int bufSizeClamped = math::clamp(
                static_cast<int64_t>(bufSize), static_cast<int64_t>(0),
                remaining);
            if (!bufSizeClamped)
            {
                return AVERROR_EOF;
            }

            memcpy(buf, bufferData->p + bufferData->offset, bufSizeClamped);
            bufferData->offset += bufSizeClamped;

            return bufSizeClamped;
        }

        int64_t avIOBufferSeek(void* opaque, int64_t offset, int whence)
        {
            AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

            if (whence & AVSEEK_SIZE)
            {
                return bufferData->size;
            }

            bufferData->offset = math::clamp(
                offset, static_cast<int64_t>(0),
                static_cast<int64_t>(bufferData->size));

            return offset;
        }

        void Read::_init(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            IRead::_init(path, memory, options, cache, logSystem);

            TLRENDER_P();

            auto i = options.find("FFmpeg/YUVToRGBConversion");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.yuvToRGBConversion;
            }
            i = options.find("FFmpeg/FastYUV420PConversion");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.fastYUV420PConversion;
            }
            i = options.find("FFmpeg/AudioChannelCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioConvertInfo.channelCount;
            }
            i = options.find("FFmpeg/AudioDataType");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioConvertInfo.dataType;
            }
            i = options.find("FFmpeg/AudioSampleRate");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioConvertInfo.sampleRate;
            }
            i = options.find("FFmpeg/AudioTrack");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioTrack;
            }
            i = options.find("FFmpeg/ThreadCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.threadCount;
            }
            i = options.find("FFmpeg/RequestTimeout");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.requestTimeout;
            }
            i = options.find("FFmpeg/VideoBufferSize");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.videoBufferSize;
            }
            i = options.find("FFmpeg/AudioBufferSize");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioBufferSize;
            }

            p.videoThread.running = true;
            p.audioThread.running = true;
            p.videoThread.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
                    try
                    {
                        p.readVideo = std::make_shared<ReadVideo>(
                            path.get(
                                -1, path.isFileProtocol()
                                        ? file::PathType::Path
                                        : file::PathType::Full),
                            _memory, _logSystem, p.options);
                        const auto& videoInfo = p.readVideo->getInfo();
                        if (videoInfo.isValid())
                        {
                            p.info.video.push_back(videoInfo);
                            p.info.videoTime = p.readVideo->getTimeRange();
                            p.info.tags = p.readVideo->getTags();
                        }

                        try
                        {
                            p.readAudio = std::make_shared<ReadAudio>(
                                path.get(
                                    -1, path.isFileProtocol()
                                    ? file::PathType::Path
                                    : file::PathType::Full),
                                _memory, p.info.videoTime.duration().rate(),
                                p.options);
                            p.info.audio = p.readAudio->getInfo();
                            p.info.audioTime = p.readAudio->getTimeRange();
                            for (const auto& tag : p.readAudio->getTags())
                            {
                                p.info.tags[tag.first] = tag.second;
                            }

                            p.audioThread.thread = std::thread(
                                [this, path]
                                    {
                                        TLRENDER_P();
                                        try
                                        {
                                            _audioThread();
                                        }
                                        catch (const std::exception& e)
                                        {
                                            if (auto logSystem = _logSystem.lock())
                                            {
                                                //! \todo How should this be handled?
                                                const std::string id =
                                                    string::Format("tl::io::ffmpeg::"
                                                                   "Read ({0}: {1})")
                                                    .arg(__FILE__)
                                                    .arg(__LINE__);
                                                logSystem->print(
                                                    id,
                                                    string::Format("{0}: {1}")
                                                    .arg(_path.get())
                                                    .arg(e.what()),
                                                    log::Type::Error);
                                            }
                                        }
                                    });
                        }
                        catch (const std::exception& e)
                        {
                            if (auto logSystem = _logSystem.lock())
                            {
                                //! \todo How should this be handled?
                                const std::string id =
                                    string::Format("tl::io::ffmpeg::"
                                                   "ReadAudio ({0}: {1})")
                                    .arg(__FILE__)
                                    .arg(__LINE__);
                                logSystem->print(
                                    id,
                                    string::Format("{0}: {1}")
                                    .arg(_path.get())
                                    .arg(e.what()),
                                    log::Type::Error);
                            }
                        }

                        _videoThread();
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            const std::string id =
                                string::Format(
                                    "tl::io::ffmpeg::Read ({0}: {1})")
                                    .arg(__FILE__)
                                    .arg(__LINE__);
                            logSystem->print(
                                id,
                                string::Format("{0}: {1}")
                                    .arg(_path.get())
                                    .arg(e.what()),
                                log::Type::Error);
                        }
                    }

                    {
                        std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                        p.videoMutex.stopped = true;
                    }
                    _cancelVideoRequests();
                    {
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        p.audioMutex.stopped = true;
                    }
                    _cancelAudioRequests();
                });
        }

        Read::Read() :
            _p(new Private)
        {
        }

        Read::~Read()
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

        void Read::_addToCache(
            io::VideoData& data, const otime::RationalTime& time,
            const io::Options& options)
        {
            TLRENDER_P();
            data.time = time;
            if (!p.readVideo->isBufferEmpty())
            {
                data.image = p.readVideo->popBuffer();
            }

            const std::string cacheKey =
                io::getVideoCacheKey(_path, time, _options, options);
            _cache->addVideo(cacheKey, data);
        }

        void Read::_videoThread()
        {
            TLRENDER_P();
            p.videoThread.currentTime = p.info.videoTime.start_time();
            p.readVideo->start();
            p.videoThread.logTimer = std::chrono::steady_clock::now();
            while (p.videoThread.running)
            {
                // Check requests.
                std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
                std::shared_ptr<Private::VideoRequest> videoRequest;
                {
                    std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                    p.videoThread.cv.wait(
                            lock, [this]
                            {
                                return (!_p->videoMutex.infoRequests.empty() ||
                                        !_p->videoMutex.videoRequests.empty() ||
                                        !_p->videoThread.running);
                            });

                    // Check if we woke up to stop
                    if (!p.videoThread.running)
                        return;

                    // Check for spurious wakeup
                    if (p.videoMutex.infoRequests.empty())
                        continue;
                    
                    infoRequests = std::move(p.videoMutex.infoRequests);
                    if (!p.videoMutex.videoRequests.empty())
                    {
                        videoRequest = p.videoMutex.videoRequests.front();
                        p.videoMutex.videoRequests.pop_front();
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
                        videoRequest->promise.set_value(videoData);
                        videoRequest.reset();
                    }
                }

                // Seek.
                //
                // \@note: Seeking on some large movies with inter-frame
                //         compression can be slow, as FFmpeg returns the
                //         closest 'F' frame.
                //         When playing backwards, while we look for the
                //         actual request time, we cache all previous 'F' and
                //         'I' frames which allows us to play 4K movies
                //         backwards with no issues.
                bool backwards = false;
                if (videoRequest && !videoRequest->time.strictly_equal(
                                        p.videoThread.currentTime))
                {
                    if (_cache &&
                        videoRequest->time < p.videoThread.currentTime)
                        backwards = true;
                    else
                        p.videoThread.currentTime = videoRequest->time;
                    p.readVideo->seek(videoRequest->time);
                }

                // Process.
                while (videoRequest && p.readVideo->isBufferEmpty() &&
                       p.readVideo->isValid() &&
                       p.readVideo->process(
                           backwards, videoRequest->time,
                           p.videoThread.currentTime))
                {
                    if (backwards)
                    {
                        if (videoRequest->time.strictly_equal(
                                p.videoThread.currentTime))
                            break;
                        io::VideoData data;
                        _addToCache(
                            data, p.videoThread.currentTime,
                            videoRequest->options);
                    }
                }

                // Handle request.
                if (videoRequest)
                {
                    io::VideoData data;
                    _addToCache(
                        data, videoRequest->time, videoRequest->options);
                    videoRequest->promise.set_value(data);
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
                                string::Format("tl::io::ffmpeg::Read {0}")
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
            p.audioThread.currentTime = p.info.audioTime.start_time();
            p.readAudio->start();
            p.audioThread.logTimer = std::chrono::steady_clock::now();
            while (p.audioThread.running)
            {
                // Check requests.
                std::shared_ptr<Private::AudioRequest> request;
                size_t requestSampleCount = 0;
                bool seek = false;
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    p.audioThread.cv.wait(
                        lock, [this]
                            { return (!_p->audioMutex.requests.empty() ||
                                      !_p->audioThread.running); });

                    // Check if we woke up to stop
                    if (!p.audioThread.running)
                        return;
                    
                    // Check for spurious wakeup
                    if (p.audioMutex.requests.empty())
                        continue;
                                        
                    request = p.audioMutex.requests.front();
                    p.audioMutex.requests.pop_front();
                    requestSampleCount =
                        request->timeRange.duration()
                        .rescaled_to(p.info.audio.sampleRate)
                        .value();
                    if (!request->timeRange.start_time().strictly_equal(
                            p.audioThread.currentTime))
                    {
                        seek = true;
                        p.audioThread.currentTime =
                            request->timeRange.start_time();
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
                       p.readAudio->isValid() &&
                       p.readAudio->process(
                           p.audioThread.currentTime,
                           requestSampleCount
                               ? requestSampleCount
                               : p.options.audioBufferSize
                                     .rescaled_to(p.info.audio.sampleRate)
                                     .value()))
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
                                string::Format("tl::io::ffmpeg::Read {0}")
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
    } // namespace ffmpeg
} // namespace tl

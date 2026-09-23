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

            int64_t pos = 0;
            switch (whence & ~AVSEEK_FORCE)
            {
            case SEEK_SET:
                pos = offset;
                break;
            case SEEK_CUR:
                pos = static_cast<int64_t>(bufferData->offset) + offset;
                break;
            case SEEK_END:
                pos = static_cast<int64_t>(bufferData->size) + offset;
                break;
            default:
                return AVERROR(EINVAL);
            }
            if (pos < 0)
            {
                return AVERROR(EINVAL);
            }

            bufferData->offset = math::clamp(
                pos, static_cast<int64_t>(0),
                static_cast<int64_t>(bufferData->size));

            return static_cast<int64_t>(bufferData->offset);
        }

        ReadOptions getReadOptions(const io::Options& options)
        {
            ReadOptions out;
            if (auto i = options.find("FFmpeg/YUVToRGBConversion"); i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.yuvToRGBConversion;
            }
            if (auto i = options.find("FFmpeg/FastYUV420PConversion"); i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.fastYUV420PConversion;
            }
            if (auto i = options.find("FFmpeg/HWAccel"); i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.hwAccel;
            }
            if (auto i = options.find("FFmpeg/AudioChannelCount");
                i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.audioConvertInfo.channelCount;
            }
            if (auto i = options.find("FFmpeg/AudioType"); i != options.end())
            {
                from_string(i->second, out.audioConvertInfo.dataType);
            }
            if (auto i = options.find("FFmpeg/AudioSampleRate");
                i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.audioConvertInfo.sampleRate;
            }
            if (auto i = options.find("FFmpeg/ThreadCount");
                i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.threadCount;
            }
            if (auto i = options.find("FFmpeg/VideoBufferSize");
                i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.videoBufferSize;
            }
            if (auto i = options.find("FFmpeg/AudioBufferSize");
                i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.audioBufferSize;
            }
            if (auto i = options.find("FFmpeg/AudioTrack");
                i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.audioTrack;
            }
            return out;
        }

        int findStream(AVFormatContext* avFormatContext, AVMediaType type)
        {
            int out = -1;
            for (unsigned int i = 0; i < avFormatContext->nb_streams; ++i)
            {
                if (type == avFormatContext->streams[i]->codecpar->codec_type &&
                    AV_DISPOSITION_DEFAULT == avFormatContext->streams[i]->disposition)
                {
                    out = i;
                    break;
                }
            }
            if (-1 == out)
            {
                for (unsigned int i = 0; i < avFormatContext->nb_streams; ++i)
                {
                    if (type == avFormatContext->streams[i]->codecpar->codec_type)
                    {
                        out = i;
                        break;
                    }
                }
            }
            return out;
        }

        namespace
        {
            //! The file name to hand to a worker; a path with a protocol is
            //! opened by FFmpeg itself.
            std::string getFileName(const file::Path& path)
            {
                constexpr bool listdir = true;
                return path.hasProtocol() ? path.get() : path.getFileName(listdir);
            }
        }

        void VideoRead::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);

            TLRENDER_P();

            p.options = getReadOptions(options);

            p.videoThread.running = true;
            p.videoThread.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
                    try
                    {
                        p.readVideo = std::make_shared<ReadVideo>(
                            getFileName(path),
                            _mem, p.options, _logSystem.lock());
                        const auto& videoInfo = p.readVideo->getInfo();
                        if (videoInfo.isValid())
                        {
                            p.info.video.push_back(videoInfo);
                            p.info.videoTime = p.readVideo ->getTimeRange();
                            p.info.tags = p.readVideo->getTags();
                        }

                        _run();
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            //! \todo How should this be handled?
                            const std::string id =
                                string::Format("tl::io::ffmpeg::"
                                               "VideoRead ({0}: {1})")
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

                    // The epilogue.
                    cancelRequests();
                });
        }

        VideoRead::VideoRead() :
            _p(new Private)
        {
            TLRENDER_P();

            // Fallback if no one creates a cache
            p.cache = io::Cache::create();
            p.cache->setMax(4 * memory::gigabyte);
        }

        void VideoRead::setCache(const std::shared_ptr<io::Cache>& cache)
        {
            TLRENDER_P();
            if (cache)
            {
                p.cache = cache;
            }
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
        }

        std::shared_ptr<VideoRead> VideoRead::create(
            const file::Path& path, const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<VideoRead>(new VideoRead);
            out->_init(path, {}, options, logSystem);
            return out;
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

        void VideoRead::_addToCache(
            io::VideoData& data, const io::Options& options)
        {
            TLRENDER_P();
            const std::string cacheKey =
                io::getVideoCacheKey(_path, data.time, _options, options);
            p.cache->addVideo(cacheKey, data);
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

        void VideoRead::_run()
        {
            TLRENDER_P();
            // Fixed once the file is probed, so it is read here rather than
            // per request. A file with no video stream reads nothing, so the
            // empty range it falls back to is never used.
            p.videoThread.currentTime = p.info.videoTime->start_time();
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
                    if (p.cache &&
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
                        data.time = p.videoThread.currentTime;
                        if (!p.readVideo->isBufferEmpty())
                        {
                            data.image = p.readVideo->popBuffer();
                        }

                        _addToCache(data, videoRequest->options);
                    }
                }

                if (videoRequest)
                {
                    // Handle the request.
                    io::VideoData data;
                    data.time = videoRequest->time;
                    if (!p.readVideo->isBufferEmpty())
                    {
                        data.image = p.readVideo->popBuffer();
                    }
                    videoRequest->promise.set_value(data);
                    _addToCache(data, videoRequest->options);

                    p.videoThread.currentTime +=
                        OTIO_NS::RationalTime(1.0,
                                            p.info.videoTime->duration().rate());
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

        void AudioRead::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& mem,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            IRead::_init(path, mem, options, logSystem);
            TLRENDER_P();

            p.options = getReadOptions(options);

            p.audioThread.running = true;
            p.audioThread.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
                    try
                    {
                        p.readAudio = std::make_shared<ReadAudio>(
                            getFileName(path), _mem, p.options);
                        p.info.audio = p.readAudio->getInfo();
                        p.info.audioTime = p.readAudio->getTimeRange();
                        p.info.tags = p.readAudio->getTags();

                        _run();
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::ffmpeg::AudioRead",
                                e.what(),
                                log::Type::Error);
                        }
                        std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
                        ++p.errorMutex.count;
                        if (p.errorMutex.error.empty())
                        {
                            p.errorMutex.error = e.what();
                        }
                    }

                    {
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        p.audioMutex.stopped = true;
                    }

                    // The epilogue.
                    cancelRequests();
                });
        }

        AudioRead::AudioRead() :
            _p(new Private)
        {
            TLRENDER_P();

            // Fallback if no one creates a cache
            p.cache = io::Cache::create();
            p.cache->setMax(2 * memory::gigabyte);
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
        }

        void AudioRead::setCache(const std::shared_ptr<io::Cache>& cache)
        {
            TLRENDER_P();
            if (cache)
            {
                p.cache = cache;
            }
        }

        std::shared_ptr<AudioRead> AudioRead::create(
            const file::Path& path,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<AudioRead>(new AudioRead);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<AudioRead> AudioRead::create(
            const file::Path& path,
            const std::vector<file::MemoryRead>& mem,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<AudioRead>(new AudioRead);
            out->_init(path, mem, options, logSystem);
            return out;
        }

        std::future<io::AudioData> AudioRead::readAudio(
            const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
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

        void AudioRead::cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                infoRequests = std::move(p.audioMutex.infoRequests);
                audioRequests = std::move(p.audioMutex.requests);
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

        std::string AudioRead::getError() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
            return p.errorMutex.error;
        }

        size_t AudioRead::getErrorCount() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
            return p.errorMutex.count;
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
                p.audioThread.cv.notify_one();
            else
                request->promise.set_value(io::Info());
            return future;
        }

        void AudioRead::_run()
        {
            TLRENDER_P();
            p.audioThread.currentTime = p.info.audioTime->start_time();
            p.readAudio->start();
            p.audioThread.logTimer = std::chrono::steady_clock::now();
            while (p.audioThread.running)
            {
                // Check requests.
                std::list<std::shared_ptr<Private::InfoRequest>> infoRequests;
                std::shared_ptr<Private::AudioRequest> request;
                size_t requestSampleCount = 0;
                bool seek = false;
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    p.audioThread.cv.wait(
                        lock, [this]
                            { return (!_p->audioMutex.infoRequests.empty() ||
                                      !_p->audioMutex.requests.empty() ||
                                      !_p->audioThread.running); });
                }

                // Check if we woke up to stop
                if (!p.audioThread.running)
                    return;


                infoRequests = std::move(p.audioMutex.infoRequests);
                for (auto& request : infoRequests)
                    request->promise.set_value(p.info);

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

                // Check the cache.
                io::AudioData audioData;
                if (request && p.cache)
                {
                    const std::string cacheKey = io::getAudioCacheKey(
                        _path, request->timeRange, _options, request->options);
                    if (p.cache->getAudio(cacheKey, audioData))
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
                if (request && p.info.audioTime.has_value())
                {
                    intersects =
                        request->timeRange.intersects(p.info.audioTime.value());
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
                    if (intersects && p.info.audioTime.has_value())
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

                    const std::string cacheKey = io::getAudioCacheKey(
                        _path, request->timeRange, _options,
                        request->options);
                    p.cache->addAudio(cacheKey, audioData);

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

    } // namespace ffmpeg
} // namespace tl

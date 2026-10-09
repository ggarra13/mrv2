// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/FFmpegReadPrivate.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>
#include <tlCore/LogSystem.h>

#include <algorithm>

extern "C"
{
#include <libavutil/opt.h>

} // extern "C"

namespace tl
{
    namespace ffmpeg
    {
        AVIOBufferData::AVIOBufferData(const uint8_t* p, size_t size) :
            p(p),
            size(size)
        {}

        int avIOBufferRead(void* opaque, uint8_t* buf, int bufSize)
        {
            AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

            const int64_t remaining = bufferData->size - bufferData->offset;
            int bufSizeClamped = file_wrong::clamp(
                static_cast<int64_t>(bufSize),
                static_cast<int64_t>(0),
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

            bufferData->offset = file_wrong::clamp(
                pos,
                static_cast<int64_t>(0),
                static_cast<int64_t>(bufferData->size));

            return static_cast<int64_t>(bufferData->offset);
        }

        ReadOptions getReadOptions(const io::Options& options)
        {
            ReadOptions out;
            if (auto i = options.find("FFmpeg/YUVToRGB"); i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.yuvToRGBConversion;
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
                from_string(i->second, out.audioConvertInfo.type);
            }
            if (auto i = options.find("FFmpeg/AudioSampleRate");
                i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.audioConvertInfo.sampleRate;
            }
            if (auto i = options.find("FFmpeg/AudioMerge");
                i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> out.audioMerge;
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
                from_string(i->second, out.audioBufferSize);
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
                return path.hasProtocol() ? path.get() : path.getFileName(true);
            }
        }

        void VideoRead::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& mem,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            IRead::_init(path, mem, options, logSystem);
            TLRENDER_P();

            p.options = getReadOptions(options);

            p.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
                    try
                    {
                        p.readVideo = std::make_shared<ReadVideo>(
                            getFileName(path),
                            _mem,
                            p.options,
                            _logSystem.lock());
                        const auto& videoInfo = p.readVideo->getInfo();
                        if (videoInfo.isValid())
                        {
                            p.info.video.push_back(videoInfo);
                            p.info.videoTime = p.readVideo->getTimeRange();
                            p.info.videoSource = p.readVideo->getSource();
                            p.info.tags = p.readVideo->getTags();
                        }
                        p.infoValid = true;

                        _run();
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::ffmpeg::VideoRead",
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

                    // The epilogue.
                    p.condition.stopQueues();
                });
        }

        VideoRead::VideoRead() :
            _p(new Private)
        {}

        VideoRead::~VideoRead()
        {
            TLRENDER_P();

            // Stop the condition and wake the thread so that shutdown does
            // not have to wait for the request timeout.
            p.condition.stop();
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<VideoRead> VideoRead::create(
            const file::Path& path,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<VideoRead>(new VideoRead);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<VideoRead> VideoRead::create(
            const file::Path& path,
            const std::vector<file::MemoryRead>& mem,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<VideoRead>(new VideoRead);
            out->_init(path, mem, options, logSystem);
            return out;
        }

        std::future<io::Info> VideoRead::getInfo()
        {
            TLRENDER_P();
            if (p.infoValid)
            {
                std::promise<io::Info> promise;
                promise.set_value(p.info);
                return promise.get_future();
            }
            return p.infoRequests.push(std::make_shared<Private::InfoRequest>());
        }

        std::future<io::VideoData> VideoRead::readVideo(
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            request->options = merge(options, _options);
            return p.videoRequests.push(request);
        }

        void VideoRead::cancelRequests()
        {
            TLRENDER_P();
            p.infoRequests.cancel();
            p.videoRequests.cancel();
        }

        std::string VideoRead::getError() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
            return p.errorMutex.error;
        }

        size_t VideoRead::getErrorCount() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
            return p.errorMutex.count;
        }

        void VideoRead::_run()
        {
            TLRENDER_P();
            // Fixed once the file is probed, so it is read here rather than
            // per request. A file with no video stream reads nothing, so the
            // empty range it falls back to is never used.
            const OTIO_NS::TimeRange videoTime =
                p.info.videoTime.value_or(OTIO_NS::TimeRange());
            p.currentTime = videoTime.start_time();
            p.readVideo->start();
            size_t errorCount = 0;
            while (p.condition.wait())
            {
                // Information requests.
                for (const auto& request : p.infoRequests.popAll())
                {
                    request->promise.set_value(p.info);
                }

                // Video request. The guard completes the promise if an
                // exception escapes; see PromiseGuard.
                if (auto videoRequest = p.videoRequests.pop())
                {
                    PromiseGuard<io::VideoData> guard(videoRequest->promise);

                    // Seek.
                    if (!videoRequest->time.strictly_equal(p.currentTime))
                    {
                        p.currentTime = videoRequest->time;
                        p.readVideo->seek(p.currentTime);
                    }

                    // Process.
                    while (
                        p.readVideo->isBufferEmpty() &&
                        p.readVideo->isValid() &&
                        p.readVideo->process(p.currentTime))
                        ;

                    // Handle the request.
                    VideoData data;
                    data.time = videoRequest->time;
                    if (!p.readVideo->isBufferEmpty())
                    {
                        data.image = p.readVideo->popBuffer();
                    }
                    guard.setValue(std::move(data));

                    p.currentTime += OTIO_NS::RationalTime(1.0, videoTime.duration().rate());
                }

                // Record any new errors from the worker, logging the
                // first one.
                if (p.readVideo->getErrorCount() != errorCount)
                {
                    const bool first = 0 == errorCount;
                    errorCount = p.readVideo->getErrorCount();
                    {
                        std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
                        p.errorMutex.count = errorCount;
                        if (p.errorMutex.error.empty())
                        {
                            p.errorMutex.error = p.readVideo->getErrorString();
                        }
                    }
                    if (first)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::ffmpeg::VideoRead",
                                string::Format("Errors reading video: \"{0}\": {1}").
                                    arg(_path.get()).
                                    arg(p.readVideo->getErrorString()),
                                log::Type::Error);
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

            p.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
                    try
                    {
                        p.readAudio = std::make_shared<ReadAudio>(
                            getFileName(path),
                            _mem,
                            p.options);
                        p.info.audio = p.readAudio->getInfo();
                        p.info.audioTime = p.readAudio->getTimeRange();
                        p.info.audioSource = p.readAudio->getSource();
                        p.info.tags = p.readAudio->getTags();

                        if (!p.info.audio.isValid())
                        {
                            // The file has no audio, which most plates do
                            // not. Stopping the queue makes a request for
                            // audio come back empty at once, so nothing
                            // waits on work that will never be done; the
                            // thread stays to serve information requests.
                            p.audioRequests.stop();
                        }
                        p.infoValid = true;

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

                    // The epilogue.
                    p.condition.stopQueues();
                });
        }

        AudioRead::AudioRead() :
            _p(new Private)
        {}

        AudioRead::~AudioRead()
        {
            TLRENDER_P();

            // Stop the condition and wake the thread so that shutdown does
            // not have to wait for the request timeout.
            p.condition.stop();
            if (p.thread.joinable())
            {
                p.thread.join();
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

        std::future<io::Info> AudioRead::getInfo()
        {
            TLRENDER_P();
            if (p.infoValid)
            {
                std::promise<io::Info> promise;
                promise.set_value(p.info);
                return promise.get_future();
            }
            return p.infoRequests.push(std::make_shared<Private::InfoRequest>());
        }

        std::future<io::AudioData> AudioRead::readAudio(
            const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->timeRange = timeRange;
            request->options = merge(options, _options);
            return p.audioRequests.push(request);
        }

        void AudioRead::cancelRequests()
        {
            TLRENDER_P();
            p.infoRequests.cancel();
            p.audioRequests.cancel();
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

        void AudioRead::_run()
        {
            TLRENDER_P();
            // As with the video above: fixed by the probe, and unused when
            // the file has no audio.
            const OTIO_NS::TimeRange audioTime =
                p.info.audioTime.value_or(OTIO_NS::TimeRange());
            p.currentTime = audioTime.start_time();
            p.readAudio->start();
            const bool audioValid = p.info.audio.isValid();
            size_t errorCount = 0;
            while (p.condition.wait())
            {
                // Information requests.
                for (const auto& request : p.infoRequests.popAll())
                {
                    request->promise.set_value(p.info);
                }

                // Audio request. The guard completes the promise if an
                // exception escapes; see PromiseGuard.
                if (auto request = p.audioRequests.pop())
                {
                    PromiseGuard<io::AudioData> guard(request->promise);

                    size_t requestSampleCount = 0;
                    bool seek = false;
                    if (audioValid)
                    {
                        requestSampleCount = request->timeRange.duration().rescaled_to(p.info.audio.sampleRate).value();
                        if (!request->timeRange.start_time().strictly_equal(p.currentTime))
                        {
                            seek = true;
                            p.currentTime = request->timeRange.start_time();
                        }
                    }

                    // Seek.
                    if (seek)
                    {
                        p.readAudio->seek(p.currentTime);
                    }

                    // Process.
                    bool intersects = false;
                    if (audioValid)
                    {
                        intersects = request->timeRange.intersects(audioTime);
                    }
                    while (
                        intersects &&
                        p.readAudio->getBufferSize() < request->timeRange.duration().rescaled_to(p.info.audio.sampleRate).value() &&
                        p.readAudio->isValid() &&
                        p.readAudio->process(
                            p.currentTime,
                            requestSampleCount ?
                            requestSampleCount :
                            p.options.audioBufferSize.rescaled_to(p.info.audio.sampleRate).value()))
                        ;

                    // Handle the request.
                    AudioData audioData;
                    audioData.time = request->timeRange.start_time();
                    if (audioValid)
                    {
                        // Note that the request time range may be expressed
                        // at any rate, so sizes and offsets must be
                        // rescaled to the sample rate rather than using the
                        // raw time values.
                        audioData.audio = Audio::create(
                            p.info.audio,
                            requestSampleCount);
                        audioData.audio->zero();
                        if (intersects)
                        {
                            size_t offset = 0;
                            if (audioData.time < audioTime.start_time())
                            {
                                offset = std::min(
                                    static_cast<size_t>(
                                        (audioTime.start_time() - audioData.time).
                                            rescaled_to(p.info.audio.sampleRate).value()),
                                    requestSampleCount);
                            }
                            p.readAudio->bufferCopy(
                                audioData.audio->getData() + offset * p.info.audio.getByteCount(),
                                audioData.audio->getSampleCount() - offset);
                        }
                    }
                    guard.setValue(std::move(audioData));

                    p.currentTime += request->timeRange.duration();
                }

                // Record any new errors from the worker, logging the
                // first one.
                if (p.readAudio->getErrorCount() != errorCount)
                {
                    const bool first = 0 == errorCount;
                    errorCount = p.readAudio->getErrorCount();
                    {
                        std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
                        p.errorMutex.count = errorCount;
                        if (p.errorMutex.error.empty())
                        {
                            p.errorMutex.error = p.readAudio->getErrorString();
                        }
                    }
                    if (first)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::ffmpeg::AudioRead",
                                string::Format("Errors reading audio: \"{0}\": {1}").
                                    arg(_path.get()).
                                    arg(p.readAudio->getErrorString()),
                                log::Type::Error);
                        }
                    }
                }
            }
        }
    }
}

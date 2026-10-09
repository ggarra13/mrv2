// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlIO/FFmpegPrivate.h>

#include <tlIO/RequestQueuePrivate.h>

#include <tlCore/LogSystem.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>

} // extern "C"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace tl
{
    namespace ffmpeg
    {
        struct AVIOBufferData
        {
            AVIOBufferData() = default;
            AVIOBufferData(const uint8_t* p, size_t size);

            const uint8_t* p = nullptr;
            size_t size = 0;
            size_t offset = 0;
        };

        int avIOBufferRead(void* opaque, uint8_t* buf, int bufSize);
        int64_t avIOBufferSeek(void* opaque, int64_t offset, int whence);

        const size_t avIOContextBufferSize = 4096;

        struct ReadOptions
        {
            bool yuvToRGBConversion = false;
            bool hwAccel = false;
            AudioInfo audioConvertInfo;
            bool audioMerge = Options().audioMerge;
            size_t threadCount = Options().threadCount;
            size_t videoBufferSize = 4;
            OTIO_NS::RationalTime audioBufferSize = OTIO_NS::RationalTime(2.0, 1.0);
        };

        //! Parse the reader options.
        ReadOptions getReadOptions(const io::Options&);

        //! Find the stream of the given type to read, or -1. A stream
        //! marked as the default is preferred over the first one found.
        int findStream(AVFormatContext*, AVMediaType);

        class ReadVideo
        {
        public:
            ReadVideo(
                const std::string& fileName,
                const std::vector<file::MemoryRead>& memory,
                const ReadOptions& options,
                const std::shared_ptr<log::System>& logSystem);

            ~ReadVideo();

            bool isValid() const;
            const image::Info& getInfo() const;
            const OTIO_NS::TimeRange& getTimeRange() const;
            const VideoSourceInfo& getSource() const;
            const image::Tags& getTags() const;

            void start();
            void seek(const OTIO_NS::RationalTime&);
            bool process(const OTIO_NS::RationalTime& currentTime);

            //! The number of read/decode errors encountered, and the
            //! first error. Only accessed from the owning thread.
            size_t getErrorCount() const;
            const std::string& getErrorString() const;

            bool isBufferEmpty() const;
            std::shared_ptr<image::Image> popBuffer();

        private:
            int _decode(const OTIO_NS::RationalTime& currentTime);
            void _copy(const std::shared_ptr<image::Image>&, AVFrame* frame);
            void _setError(int);
            int _openCodec(const AVCodec*, bool hwAccel);
            void _initHwAccel(const AVCodec*);
            bool _hwFallback(const OTIO_NS::RationalTime&);
            void _initFrame2();
            void _initSws(AVPixelFormat srcFormat);
            static AVPixelFormat _getHwFormat(AVCodecContext*, const AVPixelFormat*);
            void _log(const std::string&, log::Type = log::Type::Message) const;
            void _close();

            std::string _fileName;
            ReadOptions _options;
            image::Info _info;
            OTIO_NS::TimeRange _timeRange;
            VideoSourceInfo _source;
            image::Tags _tags;

            AVFormatContext* _avFormatContext = nullptr;
            AVIOBufferData _avIOBufferData;
            uint8_t* _avIOContextBuffer = nullptr;
            AVIOContext* _avIOContext = nullptr;
            AVRational _avSpeed = { 24, 1 };
            int _avStream = -1;
            std::map<int, AVCodecParameters*> _avCodecParameters;
            std::map<int, AVCodecContext*> _avCodecContext;
            AVFrame* _avFrame = nullptr;
            AVFrame* _avFrame2 = nullptr;
            AVPixelFormat _avInputPixelFormat = AV_PIX_FMT_NONE;
            AVPixelFormat _avOutputPixelFormat = AV_PIX_FMT_NONE;
            SwsContext* _swsContext = nullptr;
            //! The format the scaler was built for, which is the format of
            //! the frames that were arriving when it was built rather than
            //! the one the stream declares. See _copy().
            AVPixelFormat _swsInputPixelFormat = AV_PIX_FMT_NONE;
            AVBufferRef* _hwDeviceContext = nullptr;
            AVPixelFormat _hwPixelFormat = AV_PIX_FMT_NONE;
            //! The decoder in use, and the default: they differ when a
            //! hardware-capable decoder was preferred over a software
            //! default, and the default is the fallback.
            const AVCodec* _avCodec = nullptr;
            const AVCodec* _avCodecDefault = nullptr;
            AVFrame* _swFrame = nullptr;
            bool _hwAccel = false;
            bool _hwLogged = false;
            std::weak_ptr<log::System> _logSystem;
            std::list<std::shared_ptr<image::Image> > _buffer;
            bool _eof = false;
            size_t _errorCount = 0;
            std::string _errorString;
        };

        class ReadAudio
        {
        public:
            ReadAudio(
                const std::string& fileName,
                const std::vector<file::MemoryRead>&,
                const ReadOptions&);

            ~ReadAudio();

            bool isValid() const;
            const AudioInfo& getInfo() const;
            const OTIO_NS::TimeRange& getTimeRange() const;
            const AudioSourceInfo& getSource() const;
            const image::Tags& getTags() const;

            void start();
            void seek(const OTIO_NS::RationalTime&);
            bool process(
                const OTIO_NS::RationalTime& currentTime,
                size_t sampleCount);

            //! The number of read/decode errors encountered, and the
            //! first error. Only accessed from the owning thread.
            size_t getErrorCount() const;
            const std::string& getErrorString() const;

            size_t getBufferSize() const;
            void bufferCopy(uint8_t*, size_t sampleCount);

        private:
            int _decode(const OTIO_NS::RationalTime& currentTime);
            void _queueFrame(size_t streamIndex, const OTIO_NS::RationalTime& currentTime);
            void _setError(int);
            void _close();

            std::string _fileName;
            ReadOptions _options;
            AudioInfo _info;
            OTIO_NS::TimeRange _timeRange;
            AudioSourceInfo _source;
            image::Tags _tags;

            AVFormatContext* _avFormatContext = nullptr;
            AVIOBufferData _avIOBufferData;
            uint8_t* _avIOContextBuffer = nullptr;
            AVIOContext* _avIOContext = nullptr;
            //! The stream read, and with it every mono stream merged into
            //! the output as a channel (see Options::audioMerge). The first
            //! is the one seeks and times go by.
            int _avStream = -1;
            std::vector<int> _avStreams;
            std::map<int, AVCodecParameters*> _avCodecParameters;
            std::map<int, AVCodecContext*> _avCodecContext;
            AVFrame* _avFrame = nullptr;
            SwrContext* _swrContext = nullptr;
            //! Decoded samples waiting for the resampler, one queue per
            //! input plane: a plane per merged stream, per channel of a
            //! planar stream, or one holding a packed stream.
            std::vector<std::vector<uint8_t> > _planes;
            size_t _planeByteCount = 0;
            bool _flushed = false;
            std::list<std::shared_ptr<Audio> > _buffer;
            bool _eof = false;
            size_t _errorCount = 0;
            std::string _errorString;
        };

        // Errors are recorded by the worker thread and read through
        // getError()/getErrorCount() from any thread.
        struct ErrorMutex
        {
            std::string error;
            size_t count = 0;
            std::mutex mutex;
        };

        struct VideoRead::Private
        {
            ReadOptions options;

            std::shared_ptr<ReadVideo> readVideo;

            io::Info info;
            //! Set once the open has filled the information, which
            //! never changes afterwards, so getInfo() can answer
            //! immediately instead of queueing behind the request
            //! being served.
            std::atomic<bool> infoValid{ false };
            struct InfoRequest
            {
                std::promise<io::Info> promise;
            };
            struct VideoRequest
            {
                OTIO_NS::RationalTime time;
                io::Options options;
                std::promise<io::VideoData> promise;
            };
            // The info and video queues share one condition so that the
            // thread can wait for a request on either.
            RequestCondition condition;
            RequestQueue<InfoRequest, io::Info> infoRequests{ condition };
            RequestQueue<VideoRequest, VideoData> videoRequests{ condition };

            std::thread thread;
            // Only accessed from the thread above.
            OTIO_NS::RationalTime currentTime;

            ErrorMutex errorMutex;
        };

        struct AudioRead::Private
        {
            ReadOptions options;

            std::shared_ptr<ReadAudio> readAudio;

            io::Info info;
            //! See VideoRead::Private::infoValid; a file without an
            //! audio track sets it too, since empty is the answer.
            std::atomic<bool> infoValid{ false };
            struct InfoRequest
            {
                std::promise<io::Info> promise;
            };
            struct AudioRequest
            {
                OTIO_NS::TimeRange timeRange;
                io::Options options;
                std::promise<io::AudioData> promise;
            };
            RequestCondition condition;
            RequestQueue<InfoRequest, io::Info> infoRequests{ condition };
            RequestQueue<AudioRequest, AudioData> audioRequests{ condition };

            std::thread thread;
            // Only accessed from the thread above.
            OTIO_NS::RationalTime currentTime;

            ErrorMutex errorMutex;
        };
    }
}

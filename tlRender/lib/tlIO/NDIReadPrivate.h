// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024 Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlCore/HDR.h>
#include <tlIO/NDI.h>

extern "C"
{
#include <libavutil/mathematics.h>
#include <libavcodec/avcodec.h>
} // extern "C"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

namespace tl
{
    namespace ndi
    {

        class ReadVideo
        {
        public:
            ReadVideo(
                const std::string& fileName, const NDIlib_source_t& NDIsource,
                const NDIlib_recv_create_t& recv_desc,
                const NDIlib_video_frame_t& video_frame,
                const std::weak_ptr<log::System>& logSystem,
                const Options& options);

            ~ReadVideo();

            const bool isValid() const;
            const image::Info& getInfo() const;
            const otime::TimeRange& getTimeRange() const;

            void start();
            void stop();

            bool process(const otime::RationalTime& currentTime);

            bool isBufferEmpty() const;
            std::shared_ptr<image::Image> popBuffer();

        private:
            int _decode(const otime::RationalTime& currentTime);
            void _from_ndi(const NDIlib_video_frame_t& video_frame);
            void _copy(std::shared_ptr<image::Image>&);

            //! tlRender stuff
            std::weak_ptr<log::System> _logSystem;
            Options _options;
            image::Info _info;
            otime::TimeRange _timeRange = time::invalidTimeRange;
            otime::RationalTime _currentTime = time::invalidTime;
            std::list<std::shared_ptr<image::Image> > _buffer;
            image::Tags _tags;

            //! NDI structs
            const std::string _fileName;
            NDIlib_recv_instance_t NDI_recv = nullptr;
            int frame_rate_N = 30000, frame_rate_D = 1001;

            //! FFmpeg conversion variables
            AVFrame* _avFrame = nullptr;
            AVFrame* _avFrame2 = nullptr;
            AVPixelFormat _avInputPixelFormat = AV_PIX_FMT_NONE;
            AVPixelFormat _avOutputPixelFormat = AV_PIX_FMT_NONE;
            SwsContext* _swsContext = nullptr;

            //! NDI variables
            size_t _ndiStride = 0;
            NDIlib_FourCC_video_type_e _ndiFourCC = NDIlib_FourCC_type_UYVY;
            std::string _ndiPrimariesName;
            std::string _ndiTransferName;
            std::string _ndiMatrixName;
            image::HDRData _hdrData;
        };

        class ReadAudio
        {
        public:
            ReadAudio(
                const std::string& fileName, const NDIlib_source_t& NDIsource,
                const NDIlib_audio_frame_t& audio_frame,
                const std::weak_ptr<log::System>& logSystem, const Options&);

            ~ReadAudio();

            void start();
            void stop();

            const bool isValid() const;
            const audio::Info& getInfo() const;
            const otime::TimeRange& getTimeRange() const;

            void seek(const otime::RationalTime&);

            bool
            process(const otime::RationalTime& currentTime, size_t sampleCount);

            size_t getBufferSize() const;
            void bufferCopy(uint8_t*, size_t sampleCount);

        private:
            int _decode(const otime::RationalTime& currentTime);
            void _from_ndi(const NDIlib_audio_frame_t& audio_frame);

            const std::string _fileName;
            std::weak_ptr<log::System> _logSystem;

            NDIlib_recv_instance_t NDI_recv = nullptr;

            Options _options;
            audio::Info _info;
            otime::TimeRange _timeRange = time::invalidTimeRange;
            std::list<std::shared_ptr<audio::Audio> > _buffer;
        };

        struct Read::Private
        {
            Options options;

            NDIlib_find_instance_t NDI_find = nullptr;
            NDIlib_recv_instance_t NDI_recv = nullptr;
            static std::string sourceName;

            std::shared_ptr<ReadVideo> readVideo;
            std::shared_ptr<ReadAudio> readAudio;

            io::Info info;
            struct InfoRequest
            {
                std::promise<io::Info> promise;
            };

            struct VideoRequest
            {
                otime::RationalTime time = time::invalidTime;
                io::Options options;
                std::promise<io::VideoData> promise;
            };
            struct VideoMutex
            {
                std::list<std::shared_ptr<InfoRequest> > infoRequests;
                std::list<std::shared_ptr<VideoRequest> > videoRequests;
                bool stopped = false;
                std::mutex mutex;
            };
            VideoMutex videoMutex;
            struct VideoThread
            {
                otime::RationalTime currentTime = time::invalidTime;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            VideoThread videoThread;

            struct AudioRequest
            {
                otime::TimeRange timeRange = time::invalidTimeRange;
                io::Options options;
                std::promise<io::AudioData> promise;
            };
            struct AudioMutex
            {
                std::list<std::shared_ptr<AudioRequest> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            AudioMutex audioMutex;
            struct AudioThread
            {
                otime::RationalTime currentTime = time::invalidTime;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            AudioThread audioThread;
        };
    } // namespace ndi
} // namespace tl

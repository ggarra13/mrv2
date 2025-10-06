// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlIO/Plugin.h>

#include <tlCore/LogSystem.h>
#include <tlCore/HDR.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

    struct AVCodecContext;
    struct AVStream;
}

namespace tl
{
    //! FFmpeg video and audio I/O
    namespace ffmpeg
    {
        //! Profiles.
        enum class Profile {
            kNone,
            H264,
            ProRes,
            ProRes_Proxy,
            ProRes_LT,
            ProRes_HQ,
            ProRes_4444,
            ProRes_XQ,
            DNxHD,
            DNxHR_LB,
            DNxHR_SQ,
            DNxHR_HQ,
            DNxHR_HQX,
            DNxHR_444,
            VP9,
            Cineform,
            AV1,
            HAP,

            Count
        };
        TLRENDER_ENUM(Profile);
        TLRENDER_ENUM_SERIALIZE(Profile);

        //! Audio Codecs.
        enum class AudioCodec {
            kNone,
            AAC,
            AC3,
            True_HD,
            MP2,
            MP3,
            OPUS,
            VORBIS,
            PCM_S16LE,

            Count
        };
        TLRENDER_ENUM(AudioCodec);
        TLRENDER_ENUM_SERIALIZE(AudioCodec);

        //! Number of threads.
        const size_t threadCount = 0;

        //! Software scaler flags.
        const int swsScaleFlags = SWS_SPLINE | SWS_ACCURATE_RND |
                                  SWS_FULL_CHR_H_INT | SWS_FULL_CHR_H_INP;

        //! Swap the numerator and denominator.
        AVRational swap(AVRational);

        //! Convert to HDR data.
        bool
        toHDRData(AVPacketSideData* sideData, int size, image::HDRData& hdr);

        //! Convert to HDR data.
        bool toHDRData(AVFrame*, image::HDRData&);

        //! Convert from FFmpeg.
        audio::DataType toAudioType(AVSampleFormat);

        //! Convert to FFmpeg.
        AVSampleFormat fromAudioType(audio::DataType);

        //! Get the timecode from a data stream if it exists.
        std::string getTimecodeFromDataStream(AVFormatContext*);

        //! RAII class for FFmpeg packets.
        class Packet
        {
        public:
            Packet();
            ~Packet();
            AVPacket* p = nullptr;
        };

        //! Get a label for a FFmpeg error code.
        std::string getErrorLabel(int);

        //! FFmpeg reader
        class Read : public io::IRead
        {
        protected:
            void _init(
                const file::Path&, const std::vector<file::MemoryRead>&,
                const io::Options&, const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

            Read();

        public:
            virtual ~Read();

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&, const io::Options&,
                const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&, const std::vector<file::MemoryRead>&,
                const io::Options&, const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

            std::future<io::Info> getInfo() override;
            std::future<io::VideoData> readVideo(
                const otime::RationalTime&,
                const io::Options& = io::Options()) override;
            std::future<io::AudioData> readAudio(
                const otime::TimeRange&,
                const io::Options& = io::Options()) override;
            void cancelRequests() override;

        private:
            void _addToCache(
                io::VideoData& data, const otime::RationalTime&,
                const io::Options&);
            void _videoThread();
            void _audioThread();
            void _cancelVideoRequests();
            void _cancelAudioRequests();

            TLRENDER_PRIVATE();
        };

        //! FFmpeg writer.
        class Write : public io::IWrite
        {
        protected:
            void _init(
                const file::Path&, const io::Info&, const io::Options&,
                const std::weak_ptr<log::System>&);

            Write();

        public:
            virtual ~Write();

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&, const io::Info&, const io::Options&,
                const std::weak_ptr<log::System>&);

            void writeVideo(
                const otime::RationalTime&,
                const std::shared_ptr<image::Image>&,
                const io::Options& = io::Options()) override;

            void writeAudio(
                const otime::TimeRange&, const std::shared_ptr<audio::Audio>&,
                const io::Options& = io::Options()) override;

        private:
            void _attach_hdr_metadata(AVFrame*);
            void _encode(
                AVCodecContext*, const AVStream*, AVFrame*, AVPacket*);
            void _flushAudio();

            TLRENDER_PRIVATE();
        };

        //! FFmpeg Plugin
        class Plugin : public io::IPlugin
        {
        protected:
            void _init(
                const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(
                const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

            std::shared_ptr<io::IRead> read(
                const file::Path&, const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&, const std::vector<file::MemoryRead>&,
                const io::Options& = io::Options()) override;
            image::Info getWriteInfo(
                const image::Info&,
                const io::Options& = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&, const io::Info&,
                const io::Options& = io::Options()) override;

        private:
            static void _logCallback(void*, int, const char*, va_list);

            //! \todo What is a better way to access the log system from the
            //! FFmpeg callback?
            static std::weak_ptr<log::System> _logSystemWeak;
        };
    } // namespace ffmpeg
} // namespace tl

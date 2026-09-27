// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlIO/Read.h>
#include <tlIO/Write.h>

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
            AV1_AOM,
            HEVC,

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

        //! FFmpeg options.
        struct Options
        {
            bool   yuvToRgb    = false;
            bool   hwAccel     = false;
            size_t threadCount = 0;

            bool operator == (const Options&) const;
            bool operator != (const Options&) const;
        };

        //! Number of threads.
        const size_t threadCount = 0;

        //! Software scaler flags.
        const int swsScaleFlags = SWS_SPLINE | SWS_ACCURATE_RND |
                                  SWS_FULL_CHR_H_INT | SWS_FULL_CHR_H_INP;

        //! Swap the numerator and denominator.
        inline AVRational swap(AVRational value)
        {
            return AVRational({value.den, value.num});
        }

        //! Retrieve packet side data.
        inline const uint8_t *get_stream_side_data(const AVStream* st,
                                                   enum AVPacketSideDataType type)
        {
            const AVPacketSideData *sd;
            sd = av_packet_side_data_get(st->codecpar->coded_side_data,
                                         st->codecpar->nb_coded_side_data,
                                         type);
            return sd ? sd->data : NULL;
        }

        //! Attach/replace stream (codecpar) side data.
        inline AVPacketSideData* set_stream_side_data(
            AVCodecParameters* par, enum AVPacketSideDataType type,
            std::size_t size)
        {
            return av_packet_side_data_new(
                &par->coded_side_data, &par->nb_coded_side_data, type, size, 0);
        }

        //! Convert to HDR data.
        bool
        toHDRData(AVStream*, image::HDRData&);

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
        class VideoRead : public io::IVideoRead
        {
        protected:
            void _init(
                const file::Path&, const std::vector<file::MemoryRead>&,
                const io::Options&, const std::shared_ptr<log::System>&);

            VideoRead();

        public:
            virtual ~VideoRead();

            //! Create a new reader.
            static std::shared_ptr<VideoRead> create(
                const file::Path&, const io::Options&,
                const std::shared_ptr<log::System>&);

            //! Create a new reader.
            static std::shared_ptr<VideoRead> create(
                const file::Path&, const std::vector<file::MemoryRead>&,
                const io::Options&, const std::shared_ptr<log::System>&);

            std::future<io::Info> getInfo() override;
            std::future<io::VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const io::Options& = io::Options()) override;
            void setCache(const std::shared_ptr<io::Cache>&) override;
            void cancelRequests() override;

        private:
            void _run();
            void _addToCache(io::VideoData& data, const io::Options&);

            TLRENDER_PRIVATE();
        };

        //! FFmpeg audio reader.
        class AudioRead : public io::IAudioRead
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options&,
                const std::shared_ptr<log::System>&);

            AudioRead();

        public:
            virtual ~AudioRead();

            //! Create a new reader.
            static std::shared_ptr<AudioRead> create(
                const file::Path&,
                const io::Options&,
                const std::shared_ptr<log::System>&);

            //! Create a new reader.
            static std::shared_ptr<AudioRead> create(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options&,
                const std::shared_ptr<log::System>&);

            void setCache(const std::shared_ptr<io::Cache>&) override;
            std::future<io::Info> getInfo() override;
            std::future<io::AudioData> readAudio(
                const OTIO_NS::TimeRange&,
                const io::Options& = io::Options()) override;
            void cancelRequests() override;

            std::string getError() const override;
            size_t getErrorCount() const override;

        private:
            void _run();

            TLRENDER_PRIVATE();
        };

        //! FFmpeg writer.
        class Write : public io::IWrite
        {
        protected:
            void _init(
                const file::Path&, const io::Info&, const io::Options&,
                const std::shared_ptr<log::System>&);

            Write();

        public:
            virtual ~Write();

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&, const io::Info&, const io::Options&,
                const std::shared_ptr<log::System>&);

            void setHDR(const image::HDRData&) override;

            void writeHeader() override;

            void writeVideo(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<image::Image>&,
                const io::Options& = io::Options()) override;

            void writeAudio(
                const OTIO_NS::TimeRange&, const std::shared_ptr<audio::Audio>&,
                const io::Options& = io::Options()) override;

        private:
            void _attach_frame_hdr_metadata(AVFrame*);
            void _attach_stream_hdr_metadata(AVStream*);
            void _encode(
                AVCodecContext*, const AVStream*, AVFrame*, AVPacket*);
            void _flushAudio();

            TLRENDER_PRIVATE();
        };

        //! FFmpeg read plugin.
        class ReadPlugin : public io::IReadPlugin
        {
        protected:
            void _init(const std::shared_ptr<log::System>&);

            ReadPlugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<ReadPlugin> create(
                const std::shared_ptr<log::System>&);

            std::shared_ptr<io::IVideoRead> videoRead(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IVideoRead> videoRead(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options & = io::Options()) override;

            std::shared_ptr<io::IAudioRead> audioRead(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IAudioRead> audioRead(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options & = io::Options()) override;

            std::string getPluginInfo(
                const io::Options& = io::Options()) const override;

        private:
            static void _logCallback(void*, int, const char*, va_list);

            // av_log_set_callback() installs a process-global C callback with
            // no user-data parameter, so it can't be handed an instance
            // pointer; a file-scope weak_ptr is the available way to reach the
            // log system.
            static std::weak_ptr<log::System> _logSystemWeak;

            TLRENDER_PRIVATE();
        };

        //! FFmpeg write plugin.
        class WritePlugin : public io::IWritePlugin
        {
        protected:
            void _init(const std::shared_ptr<log::System>&);

            WritePlugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<WritePlugin> create(
                const std::shared_ptr<log::System>&);

            //! Get the list of video codecs.
            const std::vector<std::string>& getCodecs() const;

            //! Get the list of audio codecs.
            const std::vector<std::string>& getAudioCodecs() const;

            image::Info getInfo(
                const image::Info&,
                const io::Options& = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options& = io::Options()) override;

            std::string getPluginInfo(
                const io::Options& = io::Options()) const override;

        private:
            static void _logCallback(void*, int, const char*, va_list);

            // See comment in ReadPlugin.
            static std::weak_ptr<log::System> _logSystemWeak;

            TLRENDER_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Options&);

        void from_json(const nlohmann::json&, Options&);

        ///@}
    } // namespace ffmpeg
} // namespace tl

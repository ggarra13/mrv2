// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024 Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlCore/LogSystem.h>

#include <tlDevice/NDI/NDI.h>

#include <tlCore/NDIOptions.h>

#include <tlIO/Read.h>

#ifdef TLRENDER_FFMPEG
extern "C"
{
#    include <libavformat/avformat.h>
#    include <libswscale/swscale.h>
}
#endif

namespace tl
{
    //! Ndi video and audio I/O
    namespace ndi
    {
        //! Software scaler flags.
        const int swsScaleFlags = SWS_SPLINE | SWS_ACCURATE_RND |
                                  SWS_FULL_CHR_H_INT | SWS_FULL_CHR_H_INP;

        //! Get a label for a NDI error code.
        std::string getErrorLabel(int);

        //! NDI reader
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
                const otime::RationalTime&,
                const io::Options& = io::Options()) override;
            void setCache(const std::shared_ptr<io::Cache>&) override;
            void cancelRequests() override;

        private:
            void _run();
            void _addToCache(io::VideoData& data, const io::Options&);

            TLRENDER_PRIVATE();
        };

        //! NDI audio reader.
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

            std::future<io::Info> getInfo() override;
            std::future<io::AudioData> readAudio(
                const otio::TimeRange&,
                const io::Options& = io::Options()) override;
            void setCache(const std::shared_ptr<io::Cache>&) override;
            void cancelRequests() override;

        private:
            void _run();
            void _addToCache(
                io::AudioData& data, const otio::TimeRange&,
                const io::Options&);

            TLRENDER_PRIVATE();
        };

        //! NDI read plugin.
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
        };

    } // namespace ndi
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024 Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlCore/LogSystem.h>

#include <tlDevice/NDI/NDI.h>
#include <tlDevice/NDI/NDIOptions.h>

#include <tlIO/Plugin.h>

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

        //! NDI reader
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
            void _videoThread();
            void _audioThread();
            void _cancelVideoRequests();
            void _cancelAudioRequests();

            TLRENDER_PRIVATE();
        };

        //! Ndi Plugin
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
                const io::Options& = io::Options()) const override
            {
                image::Info out;
                return out;
            }
            std::shared_ptr<io::IWrite> write(
                const file::Path&, const io::Info&,
                const io::Options& = io::Options()) override
            {
                std::shared_ptr<io::IWrite> out;
                return out;
            }

        private:
            //! \todo What is a better way to access the log system from the
            //! FFmpeg callback?
            static std::weak_ptr<log::System> _logSystemWeak;
        };
    } // namespace ndi
} // namespace tl

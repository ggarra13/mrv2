// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024 Gonzalo Garramuño
// All rights reserved.

#include <tlIO/FFmpegMacros.h>
#include <tlIO/NDIReadPrivate.h>

#include <tlCore/StringFormat.h>

namespace
{
    const char* kModule = "ndi";
}

namespace tl
{
    namespace ndi
    {

        ReadAudio::ReadAudio(
            const std::string& fileName, const NDIlib_source_t& NDIsource,
            const NDIlib_audio_frame_t& audio_frame,
            const std::weak_ptr<log::System>& logSystem,
            const Options& options) :
            _fileName(fileName),
            _logSystem(logSystem),
            _options(options)
        {

            // We now have at least one source,
            // so we create a receiver to look at it.
            NDIlib_recv_create_t recv_desc;
            recv_desc.color_format = NDIlib_recv_color_format_fastest;
            recv_desc.bandwidth = NDIlib_recv_bandwidth_highest;
            recv_desc.allow_video_fields = false;
            recv_desc.source_to_connect_to = NDIsource;

            NDI_recv = NDIlib_recv_create(&recv_desc);
            if (!NDI_recv)
                throw std::runtime_error("Could not create NDI audio receiver");

            _from_ndi(audio_frame);
            _info.dataType = audio::DataType::F32;

            double startTime = 0.0;
            double lastTime = kNDI_MOVIE_DURATION;
            _timeRange = otime::TimeRange(
                otime::RationalTime(startTime, 1.0)
                    .rescaled_to(_info.sampleRate),
                otime::RationalTime(lastTime, 1.0)
                    .rescaled_to(_info.sampleRate));
        }

        ReadAudio::~ReadAudio()
        {
            stop();
        }

        void ReadAudio::stop()
        {
            if (NDI_recv)
                NDIlib_recv_destroy(NDI_recv);
            NDI_recv = nullptr;
        }

        void ReadAudio::start() {}

        const bool ReadAudio::isValid() const
        {
            return NDI_recv;
        }

        const audio::Info& ReadAudio::getInfo() const
        {
            return _info;
        }

        const otime::TimeRange& ReadAudio::getTimeRange() const
        {
            return _timeRange;
        }

        void ReadAudio::seek(const otime::RationalTime& time)
        {
            _buffer.clear();
        }

        bool ReadAudio::process(
            const otime::RationalTime& currentTime, size_t sampleCount)
        {
            bool out = true;
            const size_t bufferSampleCount = getBufferSize();
            if (bufferSampleCount < sampleCount)
            {
                int decoding = _decode(currentTime);
                if (decoding < 0)
                {
                    LOG_ERROR("Error decoding video stream");
                    out = false;
                }
            }
            return out;
        }

        void ReadAudio::_from_ndi(const NDIlib_audio_frame_t& a)
        {
            _info.channelCount = a.no_channels;
            _info.sampleRate = a.sample_rate;

            auto tmp = audio::Audio::create(_info, a.no_samples);
            memcpy(tmp->getData(), a.p_data, tmp->getByteCount());
            tmp = audio::planarInterleave(tmp);
            _buffer.push_back(tmp);
        }

        int ReadAudio::_decode(const otime::RationalTime& time)
        {
            int out = 0;
            NDIlib_audio_frame_t a;
            NDIlib_frame_type_e type;

            while (out == 0)
            {
                type = NDIlib_recv_capture(NDI_recv, nullptr, &a, nullptr, 50);
                if (type == NDIlib_frame_type_error)
                {
                    out = -1;
                    LOG_ERROR("Error decoding audio stream");
                }
                else if (type == NDIlib_frame_type_audio)
                {
                    _from_ndi(a);
                    NDIlib_recv_free_audio(NDI_recv, &a);
                    out = 1;
                }
                else if (type == NDIlib_frame_type_status_change)
                {
                }
            }
            return out;
        }

        size_t ReadAudio::getBufferSize() const
        {
            return audio::getSampleCount(_buffer);
        }

        void ReadAudio::bufferCopy(uint8_t* out, size_t sampleCount)
        {
            audio::move(_buffer, out, sampleCount);
        }
    } // namespace ndi
} // namespace tl

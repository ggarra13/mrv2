// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpegReadPrivate.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ffmpeg
    {
        ReadAudio::ReadAudio(
            const std::string& fileName,
            const std::vector<file::MemoryRead>& memory, double videoRate,
            const Options& options) :
            _fileName(fileName),
            _options(options)
        {
            if (!memory.empty())
            {
                _avFormatContext = avformat_alloc_context();
                if (!_avFormatContext)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate format context")
                            .arg(fileName));
                }

                _avIOBufferData = AVIOBufferData(memory[0].p, memory[0].size);
                _avIOContextBuffer =
                    static_cast<uint8_t*>(av_malloc(avIOContextBufferSize));
                _avIOContext = avio_alloc_context(
                    _avIOContextBuffer, avIOContextBufferSize, 0,
                    &_avIOBufferData, &avIOBufferRead, nullptr,
                    &avIOBufferSeek);
                if (!_avIOContext)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate I/O context")
                            .arg(fileName));
                }

                _avFormatContext->pb = _avIOContext;
            }

            int r = avformat_open_input(
                &_avFormatContext,
                !_avFormatContext ? fileName.c_str() : nullptr, nullptr,
                nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(fileName)
                                             .arg(getErrorLabel(r)));
            }

            r = avformat_find_stream_info(_avFormatContext, 0);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(fileName)
                                             .arg(getErrorLabel(r)));
            }

            // Count the tracks and get the metadata for each audio track
            image::Tags tags;
            for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
            {
                const auto& avAudioStream = _avFormatContext->streams[i];
                const auto& avAudioCodecParameters = avAudioStream->codecpar;

                if (AVMEDIA_TYPE_AUDIO == avAudioCodecParameters->codec_type)
                {
                    if (options.audioTrack == _info.audioInfo.size())
                    {
                        _avStream = i;
                        _info.currentTrack = _info.audioInfo.size();
                    }

                    std::string fileLanguage = "Default";
                    AVDictionaryEntry* tag = nullptr;
                    unsigned trackNumber = _info.audioInfo.size() + 1;
                    while (
                        (tag = av_dict_get(
                             avAudioStream->metadata, "", tag,
                             AV_DICT_IGNORE_SUFFIX)))
                    {
                        const std::string& key = tag->key;
                        if (key == "language")
                            fileLanguage = tag->value;
                        const std::string& audio_key(
                            string::Format("Audio Stream #{0}: {1}")
                                .arg(trackNumber)
                                .arg(key));
                        tags[audio_key] = tag->value;
                    }

                    const size_t fileChannelCount =
                        avAudioCodecParameters->ch_layout.nb_channels;
                    const audio::DataType fileDataType =
                        toAudioType(static_cast<AVSampleFormat>(
                            avAudioCodecParameters->format));
                    const size_t fileSampleRate =
                        avAudioCodecParameters->sample_rate;

                    std::shared_ptr<audio::Info> info =
                        std::make_shared<audio::Info>();

                    info->name = fileLanguage;
                    info->channelCount = fileChannelCount;
                    info->dataType = fileDataType;
                    info->sampleRate = fileSampleRate;
                    _info.audioInfo.push_back(info);
                }
            }

            // If user selected specific track, use it.
            if (options.audioTrack >= 0)
            {
                int idx = 0;
                for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
                {
                    if (AVMEDIA_TYPE_AUDIO ==
                        _avFormatContext->streams[i]->codecpar->codec_type)
                    {
                        if (options.audioTrack == idx)
                        {
                            _avStream = i;
                            break;
                        }
                        ++idx;
                    }
                }
            }

            // Else, use the disposition track.
            if (-1 == _avStream)
            {
                for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
                {
                    if (AVMEDIA_TYPE_AUDIO == _avFormatContext->streams[i]
                                                  ->codecpar->codec_type &&
                        AV_DISPOSITION_DEFAULT ==
                            _avFormatContext->streams[i]->disposition)
                    {
                        _avStream = i;
                        break;
                    }
                }
            }

            // If all failed, use the first track we find.
            if (-1 == _avStream)
            {
                for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
                {
                    if (AVMEDIA_TYPE_AUDIO ==
                        _avFormatContext->streams[i]->codecpar->codec_type)
                    {
                        _avStream = i;
                        break;
                    }
                }
            }
            std::string timecode = getTimecodeFromDataStream(_avFormatContext);
            if (_avStream != -1)
            {
                // av_dump_format(_avFormatContext, _avStream, fileName.c_str(),
                // 0);

                auto avAudioStream = _avFormatContext->streams[_avStream];
                auto avAudioCodecParameters = avAudioStream->codecpar;
                auto avAudioCodec =
                    avcodec_find_decoder(avAudioCodecParameters->codec_id);
                if (!avAudioCodec)
                {
                    throw std::runtime_error(
                        string::Format("{0}: No audio codec found")
                            .arg(fileName));
                }
                _avCodecParameters[_avStream] = avcodec_parameters_alloc();
                if (!_avCodecParameters[_avStream])
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate parameters")
                            .arg(fileName));
                }
                r = avcodec_parameters_copy(
                    _avCodecParameters[_avStream], avAudioCodecParameters);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}")
                                                 .arg(fileName)
                                                 .arg(getErrorLabel(r)));
                }
                _avCodecContext[_avStream] =
                    avcodec_alloc_context3(avAudioCodec);
                if (!_avCodecContext[_avStream])
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate context")
                            .arg(fileName));
                }
                r = avcodec_parameters_to_context(
                    _avCodecContext[_avStream], _avCodecParameters[_avStream]);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}")
                                                 .arg(fileName)
                                                 .arg(getErrorLabel(r)));
                }
                _avCodecContext[_avStream]->thread_count = options.threadCount;
                _avCodecContext[_avStream]->thread_type = FF_THREAD_FRAME;
                r = avcodec_open2(_avCodecContext[_avStream], avAudioCodec, 0);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}")
                                                 .arg(fileName)
                                                 .arg(getErrorLabel(r)));
                }

                const size_t fileChannelCount =
                    _avCodecParameters[_avStream]->ch_layout.nb_channels;
                const audio::DataType fileDataType =
                    toAudioType(static_cast<AVSampleFormat>(
                        _avCodecParameters[_avStream]->format));
                if (audio::DataType::kNone == fileDataType)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Unsupported audio format")
                            .arg(fileName));
                }
                const size_t fileSampleRate =
                    _avCodecParameters[_avStream]->sample_rate;

                size_t channelCount = fileChannelCount;
                audio::DataType dataType = fileDataType;
                size_t sampleRate = fileSampleRate;
                if (options.audioConvertInfo.isValid())
                {
                    channelCount = options.audioConvertInfo.channelCount;
                    dataType = options.audioConvertInfo.dataType;
                    sampleRate = options.audioConvertInfo.sampleRate;
                }
                _info.channelCount = channelCount;
                _info.dataType = dataType;
                _info.sampleRate = sampleRate;
                _info.trackCount = _info.audioInfo.size();

                int64_t sampleCount = 0;
                if (avAudioStream->duration != AV_NOPTS_VALUE)
                {
                    AVRational r;
                    r.num = 1;
                    r.den = sampleRate;
                    sampleCount = av_rescale_q(
                        avAudioStream->duration, avAudioStream->time_base, r);
                }
                else if (_avFormatContext->duration != AV_NOPTS_VALUE)
                {
                    AVRational r;
                    r.num = 1;
                    r.den = sampleRate;
                    sampleCount = av_rescale_q(
                        _avFormatContext->duration, av_get_time_base_q(), r);
                }

                otime::RationalTime timeReference = time::invalidTime;
                image::Tags tags;
                AVDictionaryEntry* tag = nullptr;
                while (
                    (tag = av_dict_get(
                         _avFormatContext->metadata, "", tag,
                         AV_DICT_IGNORE_SUFFIX)))
                {
                    const std::string key(tag->key);
                    const std::string value(tag->value);
                    tags[key] = value;
                    if (string::compare(
                            key, "timecode", string::Compare::CaseInsensitive))
                    {
                        timecode = value;
                    }
                    else if (string::compare(
                                 key, "time_reference",
                                 string::Compare::CaseInsensitive))
                    {
                        timeReference = otime::RationalTime(
                            std::atoi(value.c_str()), sampleRate);
                    }
                }

                otime::RationalTime startTime(0.0, sampleRate);
                if (!timecode.empty())
                {
                    otime::ErrorStatus errorStatus;
                    const otime::RationalTime time =
                        otime::RationalTime::from_timecode(
                            timecode, videoRate, &errorStatus);
                    if (!otime::is_error(errorStatus))
                    {
                        startTime = time.rescaled_to(sampleRate).floor();
                        // std::cout << fileName << " start time: " << startTime
                        // << std::endl;
                    }
                }
                else if (!timeReference.is_invalid_time())
                {
                    startTime = timeReference;
                }
                _timeRange = otime::TimeRange(
                    startTime, otime::RationalTime(sampleCount, sampleRate));

                for (const auto& i : tags)
                {
                    _tags[i.first] = i.second;
                }
                {
                    std::stringstream ss;
                    ss << static_cast<int>(fileChannelCount);
                    _tags["Audio Channels"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << fileDataType;
                    _tags["Audio Data Type"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(1);
                    ss << std::fixed;
                    ss << fileSampleRate / 1000.F << " kHz";
                    _tags["Audio Sample Rate"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << _timeRange.start_time().rescaled_to(1.0).value()
                       << " seconds";
                    _tags["Audio Start Time"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << _timeRange.duration().rescaled_to(1.0).value()
                       << " seconds";
                    _tags["Audio Duration"] = ss.str();
                }
                {
                    _tags["Audio Codec"] =
                        avcodec_get_name(_avCodecContext[_avStream]->codec_id);
                }
            }
        }

        ReadAudio::~ReadAudio()
        {
            if (_swrContext)
            {
                swr_free(&_swrContext);
            }
            if (_avFrame)
            {
                av_frame_free(&_avFrame);
            }
            for (auto i : _avCodecContext)
            {
                avcodec_free_context(&i.second);
            }
            for (auto i : _avCodecParameters)
            {
                avcodec_parameters_free(&i.second);
            }
            if (_avIOContext && _avIOContext->buffer)
            {
                av_free(_avIOContext->buffer);
            }
            if (_avIOContext)
            {
                avio_context_free(&_avIOContext);
            }
            if (_avFormatContext)
            {
                avformat_close_input(&_avFormatContext);
            }
        }

        bool ReadAudio::isValid() const
        {
            return _avStream != -1;
        }

        const audio::Info& ReadAudio::getInfo() const
        {
            return _info;
        }

        const otime::TimeRange& ReadAudio::getTimeRange() const
        {
            return _timeRange;
        }

        const image::Tags& ReadAudio::getTags() const
        {
            return _tags;
        }

        void ReadAudio::start()
        {
            if (_avStream != -1)
            {
                _avFrame = av_frame_alloc();
                if (!_avFrame)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate frame")
                            .arg(_fileName));
                }

                AVChannelLayout channelLayout;
                av_channel_layout_default(&channelLayout, _info.channelCount);
                const auto& avCodecParameters = _avCodecParameters[_avStream];
                int r = swr_alloc_set_opts2(
                    &_swrContext, &channelLayout, fromAudioType(_info.dataType),
                    _info.sampleRate, &avCodecParameters->ch_layout,
                    static_cast<AVSampleFormat>(avCodecParameters->format),
                    avCodecParameters->sample_rate, 0, NULL);
                av_channel_layout_uninit(&channelLayout);
                if (!_swrContext)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot get context")
                            .arg(_fileName));
                }
                swr_init(_swrContext);
            }
        }

        void ReadAudio::seek(const otime::RationalTime& time)
        {
            // std::cout << "audio seek: " << time << std::endl;

            if (_avStream != -1)
            {
                avcodec_flush_buffers(_avCodecContext[_avStream]);

                AVRational r;
                r.num = 1;
                r.den = _info.sampleRate;
                if (av_seek_frame(
                        _avFormatContext, _avStream,
                        av_rescale_q(
                            time.value() - _timeRange.start_time().value(), r,
                            _avFormatContext->streams[_avStream]->time_base),
                        AVSEEK_FLAG_BACKWARD) < 0)
                {
                    //! \todo How should this be handled?
                }
            }

            if (_swrContext)
            {
                swr_init(_swrContext);
            }

            _buffer.clear();
            _eof = false;
        }

        bool ReadAudio::process(
            const otime::RationalTime& currentTime, size_t sampleCount)
        {
            bool out = false;
            const size_t bufferSampleCount = audio::getSampleCount(_buffer);
            if (_avStream != -1 && bufferSampleCount < sampleCount)
            {
                Packet packet;
                int decoding = 0;
                while (0 == decoding)
                {
                    if (!_eof)
                    {
                        decoding = av_read_frame(_avFormatContext, packet.p);
                        if (AVERROR_EOF == decoding)
                        {
                            _eof = true;
                            decoding = 0;
                        }
                        else if (decoding < 0)
                        {
                            //! \todo How should this be handled?
                            break;
                        }
                    }
                    if ((_eof && _avStream != -1) ||
                        (_avStream == packet.p->stream_index))
                    {
                        decoding = avcodec_send_packet(
                            _avCodecContext[_avStream],
                            _eof ? nullptr : packet.p);
                        if (AVERROR_EOF == decoding)
                        {
                            decoding = 0;
                        }
                        else if (decoding < 0)
                        {
                            //! \todo How should this be handled?
                            break;
                        }
                        decoding = _decode(currentTime);
                        if (AVERROR(EAGAIN) == decoding)
                        {
                            decoding = 0;
                        }
                        else if (AVERROR_EOF == decoding)
                        {
                            const size_t bufferSize =
                                audio::getSampleCount(_buffer);
                            const size_t bufferMax =
                                _options.audioBufferSize
                                    .rescaled_to(_info.sampleRate)
                                    .value();
                            if (bufferSize < bufferMax)
                            {
                                auto audio = audio::Audio::create(
                                    _info, bufferMax - bufferSize);
                                audio->zero();
                                _buffer.push_back(audio);
                            }
                            break;
                        }
                        else if (decoding < 0)
                        {
                            //! \todo How should this be handled?
                            break;
                        }
                        else if (1 == decoding)
                        {
                            out = true;
                            break;
                        }
                    }
                    if (packet.p->buf)
                    {
                        av_packet_unref(packet.p);
                    }
                }
                if (packet.p->buf)
                {
                    av_packet_unref(packet.p);
                }
                // std::cout << "audio buffer size: " <<
                // audio::getSampleCount(_buffer) << std::endl;
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

        int ReadAudio::_decode(const otime::RationalTime& currentTime)
        {
            int out = 0;
            while (0 == out)
            {
                out =
                    avcodec_receive_frame(_avCodecContext[_avStream], _avFrame);
                if (out < 0)
                {
                    return out;
                }
                const int64_t timestamp = _avFrame->pts != AV_NOPTS_VALUE
                                              ? _avFrame->pts
                                              : _avFrame->pkt_dts;
                // std::cout << "audio timestamp: " << timestamp << std::endl;

                AVRational r;
                r.num = 1;
                r.den = _info.sampleRate;
                const auto time = otime::RationalTime(
                    _timeRange.start_time().value() +
                        av_rescale_q(
                            timestamp,
                            _avFormatContext->streams[_avStream]->time_base, r),
                    _info.sampleRate);
                // std::cout << "audio time: " << time << std::endl;

                if (time >= currentTime)
                {
                    // std::cout << "audio time: " << time << std::endl;
                    // std::cout << "nb_samples: " << _avFrame->nb_samples <<
                    // std::endl;
                    const int swrOutputSamples =
                        swr_get_out_samples(_swrContext, _avFrame->nb_samples);
                    // std::cout << "swrOutputSamples: " << swrOutputSamples <<
                    // std::endl;
                    auto swrOutputBuffer =
                        audio::Audio::create(_info, swrOutputSamples);
                    uint8_t* swrOutputBufferP[] = {swrOutputBuffer->getData()};
                    const int swrOutputCount = swr_convert(
                        _swrContext, swrOutputBufferP, swrOutputSamples,
                        (const uint8_t**)_avFrame->data, _avFrame->nb_samples);
                    // std::cout << "swrOutputCount: " << swrOutputCount <<
                    // std::endl << std::endl;
                    auto tmp = audio::Audio::create(
                        _info, swrOutputCount > 0 ? swrOutputCount : 0);
                    memcpy(
                        tmp->getData(), swrOutputBuffer->getData(),
                        tmp->getByteCount());
                    _buffer.push_back(tmp);
                    out = 1;
                    break;
                }
            }
            return out;
        }
    } // namespace ffmpeg
} // namespace tl

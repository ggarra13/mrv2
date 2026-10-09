// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/FFmpegReadPrivate.h>

#include <tlCore/StringFormat.h>

#include <algorithm>
#include <cstdlib>
#include <limits>

namespace tl
{
    namespace ffmpeg
    {
        ReadAudio::ReadAudio(
            const std::string& fileName,
            const std::vector<file::MemoryRead>& memory,
            const ReadOptions& options) :
            _fileName(fileName),
            _options(options)
        {
            try
            {
                if (!memory.empty())
                {
                    _avFormatContext = avformat_alloc_context();
                    if (!_avFormatContext)
                    {
                        throw std::runtime_error(
                            string::Format("Cannot allocate format context: \"{0}\"").
                            arg(fileName));
                    }

                    _avIOBufferData = AVIOBufferData(memory[0].p, memory[0].size);
                    _avIOContextBuffer = static_cast<uint8_t*>(av_malloc(avIOContextBufferSize));
                    _avIOContext = avio_alloc_context(
                        _avIOContextBuffer,
                        avIOContextBufferSize,
                        0,
                        &_avIOBufferData,
                        &avIOBufferRead,
                        nullptr,
                        &avIOBufferSeek);
                    if (!_avIOContext)
                    {
                        throw std::runtime_error(
                            string::Format("Cannot allocate I/O context: \"{0}\"").
                            arg(fileName));
                    }

                    _avFormatContext->pb = _avIOContext;
                }

                int r = avformat_open_input(
                    &_avFormatContext,
                    !_avFormatContext ? fileName.c_str() : nullptr,
                    nullptr,
                    nullptr);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: \"{1}\"").arg(getErrorLabel(r)).arg(fileName));
                }

                r = avformat_find_stream_info(_avFormatContext, 0);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: \"{1}\"").arg(getErrorLabel(r)).arg(fileName));
                }
                _avStream = findStream(_avFormatContext, AVMEDIA_TYPE_AUDIO);
                if (_avStream != -1)
                {
                    _avStreams.push_back(_avStream);
                    // A mono stream is taken with every other mono stream
                    // that matches it, as the channels of one track: that
                    // is how broadcast files carry their audio, one stream
                    // per channel. Streams that differ in codec, rate or
                    // format are something else and are left alone.
                    const auto* first = _avFormatContext->streams[_avStream]->codecpar;
                    if (options.audioMerge && 1 == first->ch_layout.nb_channels)
                    {
                        for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
                        {
                            const auto* par = _avFormatContext->streams[i]->codecpar;
                            if (static_cast<int>(i) != _avStream &&
                                AVMEDIA_TYPE_AUDIO == par->codec_type &&
                                par->codec_id == first->codec_id &&
                                1 == par->ch_layout.nb_channels &&
                                par->sample_rate == first->sample_rate &&
                                par->format == first->format)
                            {
                                _avStreams.push_back(i);
                            }
                        }
                    }
                }

                // The video rate is needed only to parse the timecode tag
                // into a start time below, and is read from this reader's own
                // format context: the audio does not depend on a video reader
                // existing. A file with no video has no rate to parse the
                // timecode against.
                // Negative, so that from_timecode() below rejects it and
                // leaves the start time alone.
                double videoRate = -1.0;
                const int avVideoStream = findStream(
                    _avFormatContext,
                    AVMEDIA_TYPE_VIDEO);
                if (avVideoStream != -1)
                {
                    videoRate = av_q2d(av_guess_frame_rate(
                        _avFormatContext,
                        _avFormatContext->streams[avVideoStream],
                        nullptr));
                }

                const std::string timecode = getTimecode(_avFormatContext);

                // Only the audio streams are read: the demuxer then skips the
                // others' data rather than handing it over to be thrown away.
                // In a movie the audio is interleaved with the video, and
                // reading a second of an 8K DNxHR movie's sound read the 700
                // MB of pictures around it. The timecode and the video rate
                // are metadata, found already.
                for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
                {
                    if (std::find(_avStreams.begin(), _avStreams.end(), static_cast<int>(i)) == _avStreams.end())
                    {
                        _avFormatContext->streams[i]->discard = AVDISCARD_ALL;
                    }
                }
                if (_avStream != -1)
                {
                    //av_dump_format(_avFormatContext, _avStream, fileName.c_str(), 0);

                    auto avAudioStream = _avFormatContext->streams[_avStream];
                    for (int stream : _avStreams)
                    {
                        auto avAudioCodecParameters = _avFormatContext->streams[stream]->codecpar;
                        auto avAudioCodec = avcodec_find_decoder(avAudioCodecParameters->codec_id);
                        if (!avAudioCodec)
                        {
                            throw std::runtime_error(string::Format("No audio codec found: \"{0}\"").arg(fileName));
                        }
                        _avCodecParameters[stream] = avcodec_parameters_alloc();
                        if (!_avCodecParameters[stream])
                        {
                            throw std::runtime_error(string::Format("Cannot allocate parameters: \"{0}\"").arg(fileName));
                        }
                        r = avcodec_parameters_copy(_avCodecParameters[stream], avAudioCodecParameters);
                        if (r < 0)
                        {
                            throw std::runtime_error(string::Format("{0}: \"{1}\"").arg(getErrorLabel(r)).arg(fileName));
                        }
                        _avCodecContext[stream] = avcodec_alloc_context3(avAudioCodec);
                        if (!_avCodecContext[stream])
                        {
                            throw std::runtime_error(string::Format("Cannot allocate context: \"{0}\"").arg(fileName));
                        }
                        r = avcodec_parameters_to_context(_avCodecContext[stream], _avCodecParameters[stream]);
                        if (r < 0)
                        {
                            throw std::runtime_error(string::Format("{0}: \"{1}\"").arg(getErrorLabel(r)).arg(fileName));
                        }
                        _avCodecContext[stream]->thread_count = options.threadCount;
                        _avCodecContext[stream]->thread_type = FF_THREAD_FRAME;
                        r = avcodec_open2(_avCodecContext[stream], avAudioCodec, 0);
                        if (r < 0)
                        {
                            throw std::runtime_error(string::Format("{0}: \"{1}\"").arg(getErrorLabel(r)).arg(fileName));
                        }
                    }

                    const size_t fileChannelCount = _avStreams.size() > 1 ?
                        _avStreams.size() :
                        _avCodecParameters[_avStream]->ch_layout.nb_channels;
                    const AudioType fileAudioType = toAudioType(static_cast<AVSampleFormat>(
                        _avCodecParameters[_avStream]->format));
                    if (AudioType::None == fileAudioType)
                    {
                        throw std::runtime_error(string::Format("Unsupported audio format: \"{0}\"").arg(fileName));
                    }
                    const size_t fileSampleRate = _avCodecParameters[_avStream]->sample_rate;

                    size_t channelCount = fileChannelCount;
                    AudioType audioType = fileAudioType;
                    size_t sampleRate = fileSampleRate;
                    if (options.audioConvertInfo.isValid())
                    {
                        channelCount = options.audioConvertInfo.channelCount;
                        audioType = options.audioConvertInfo.type;
                        sampleRate = options.audioConvertInfo.sampleRate;
                    }
                    _info.channelCount = channelCount;
                    _info.type = audioType;
                    _info.sampleRate = sampleRate;

                    int64_t sampleCount = 0;
                    if (avAudioStream->duration != AV_NOPTS_VALUE)
                    {
                        AVRational r;
                        r.num = 1;
                        r.den = sampleRate;
                        sampleCount = av_rescale_q(
                            avAudioStream->duration,
                            avAudioStream->time_base,
                            r);
                    }
                    else if (_avFormatContext->duration != AV_NOPTS_VALUE)
                    {
                        AVRational r;
                        r.num = 1;
                        r.den = sampleRate;
                        sampleCount = av_rescale_q(
                            _avFormatContext->duration,
                            av_get_time_base_q(),
                            r);
                    }

                    std::optional<OTIO_NS::RationalTime> timeReference;
                    image::Tags tags;
                    AVDictionaryEntry* tag = nullptr;
                    while ((tag = av_dict_get(_avFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                    {
                        const std::string key(tag->key);
                        const std::string value(tag->value);
                        tags[key] = value;
                        if (file_wrong::compare(
                            key,
                            "time_reference",
                            file_wrong::CaseCompare::Insensitive))
                        {
                            timeReference = OTIO_NS::RationalTime(
                                static_cast<double>(
                                    std::strtoll(value.c_str(), nullptr, 10)),
                                sampleRate);
                        }
                    }

                    OTIO_NS::RationalTime startTime(0.0, sampleRate);
                    if (!timecode.empty())
                    {
                        opentime::ErrorStatus errorStatus;
                        const OTIO_NS::RationalTime time = OTIO_NS::RationalTime::from_timecode(
                            timecode,
                            videoRate,
                            &errorStatus);
                        if (!opentime::is_error(errorStatus))
                        {
                            startTime = time.rescaled_to(sampleRate).floor();
                        }
                    }
                    else if (timeReference.has_value())
                    {
                        startTime = timeReference.value();
                    }
                    _timeRange = OTIO_NS::TimeRange(
                        startTime,
                        OTIO_NS::RationalTime(sampleCount, sampleRate));

                    for (const auto& i : tags)
                    {
                        _tags[i.first] = i.second;
                    }
                    {
                        _source.codec =
                            avcodec_get_name(_avCodecContext[_avStream]->codec_id);
                        _source.type = fileAudioType;
                        _source.channelCount = fileChannelCount;
                        _source.sampleRate = fileSampleRate;
                    }
                }
            }
            catch (...)
            {
                _close();
                throw;
            }
        }

        ReadAudio::~ReadAudio()
        {
            _close();
        }

        void ReadAudio::_close()
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
            if (_avFormatContext)
            {
                avformat_close_input(&_avFormatContext);
            }
            if (_avIOContext)
            {
                av_freep(&_avIOContext->buffer);
                avio_context_free(&_avIOContext);
            }
        }

        bool ReadAudio::isValid() const
        {
            return _avStream != -1;
        }

        const AudioInfo& ReadAudio::getInfo() const
        {
            return _info;
        }

        const OTIO_NS::TimeRange& ReadAudio::getTimeRange() const
        {
            return _timeRange;
        }

        const AudioSourceInfo& ReadAudio::getSource() const
    {
        return _source;
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
                    throw std::runtime_error(string::Format("Cannot allocate frame: \"{0}\"").arg(_fileName));
                }

                AVChannelLayout channelLayout;
                av_channel_layout_default(&channelLayout, _info.channelCount);
                const auto& avCodecParameters = _avCodecParameters[_avStream];
                const auto avFormat = static_cast<AVSampleFormat>(avCodecParameters->format);
                const size_t byteCount = av_get_bytes_per_sample(avFormat);
                if (_avStreams.size() > 1)
                {
                    // Merged streams are the planes of one planar input:
                    // each stream's mono samples are one channel.
                    AVChannelLayout inputLayout;
                    av_channel_layout_default(&inputLayout, _avStreams.size());
                    swr_alloc_set_opts2(
                        &_swrContext,
                        &channelLayout,
                        fromAudioType(_info.type),
                        _info.sampleRate,
                        &inputLayout,
                        av_get_planar_sample_fmt(avFormat),
                        avCodecParameters->sample_rate,
                        0,
                        NULL);
                    av_channel_layout_uninit(&inputLayout);
                    _planes.resize(_avStreams.size());
                    _planeByteCount = byteCount;
                }
                else
                {
                    swr_alloc_set_opts2(
                        &_swrContext,
                        &channelLayout,
                        fromAudioType(_info.type),
                        _info.sampleRate,
                        &avCodecParameters->ch_layout,
                        avFormat,
                        avCodecParameters->sample_rate,
                        0,
                        NULL);
                    if (av_sample_fmt_is_planar(avFormat))
                    {
                        _planes.resize(avCodecParameters->ch_layout.nb_channels);
                        _planeByteCount = byteCount;
                    }
                    else
                    {
                        _planes.resize(1);
                        _planeByteCount = byteCount * avCodecParameters->ch_layout.nb_channels;
                    }
                }
                av_channel_layout_uninit(&channelLayout);
                if (!_swrContext)
                {
                    throw std::runtime_error(string::Format("Cannot get context: \"{0}\"").arg(_fileName));
                }
                swr_init(_swrContext);
            }
        }

        void ReadAudio::seek(const OTIO_NS::RationalTime& time)
        {

            if (_avStream != -1)
            {
                for (int stream : _avStreams)
                {
                    avcodec_flush_buffers(_avCodecContext[stream]);
                }

                AVRational r;
                r.num = 1;
                r.den = _info.sampleRate;
                const int seekError = av_seek_frame(
                    _avFormatContext,
                    _avStream,
                    av_rescale_q(
                        time.value() - _timeRange.start_time().value(),
                        r,
                        _avFormatContext->streams[_avStream]->time_base),
                    AVSEEK_FLAG_BACKWARD);
                if (seekError < 0)
                {
                    _setError(seekError);
                }
            }

            if (_swrContext)
            {
                const int drain = swr_get_out_samples(_swrContext, 0);
                std::vector<uint8_t> tmp(drain * _info.getByteCount(), 0);
                uint8_t* tmpP[] = { tmp.data() };
                swr_convert(
                    _swrContext,
                    tmpP,
                    drain,
                    nullptr,
                    0);
            }

            for (auto& plane : _planes)
            {
                plane.clear();
            }
            _buffer.clear();
            _eof = false;
            _flushed = false;
        }

        size_t ReadAudio::getErrorCount() const
        {
            return _errorCount;
        }

        const std::string& ReadAudio::getErrorString() const
        {
            return _errorString;
        }

        void ReadAudio::_setError(int error)
        {
            ++_errorCount;
            if (_errorString.empty())
            {
                _errorString = getErrorLabel(error);
            }
        }

        bool ReadAudio::process(
            const OTIO_NS::RationalTime& currentTime,
            size_t sampleCount)
        {
            bool out = false;
            const size_t bufferSampleCount = getSampleCount(_buffer);
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
                            _setError(decoding);
                            break;
                        }
                    }
                    const bool wanted = !_eof &&
                        std::find(_avStreams.begin(), _avStreams.end(), packet.p->stream_index) !=
                        _avStreams.end();
                    if (_eof || wanted)
                    {
                        if (_eof)
                        {
                            // Drain every decoder, once.
                            if (!_flushed)
                            {
                                _flushed = true;
                                for (int stream : _avStreams)
                                {
                                    avcodec_send_packet(_avCodecContext[stream], nullptr);
                                }
                            }
                        }
                        else
                        {
                            decoding = avcodec_send_packet(
                                _avCodecContext[packet.p->stream_index],
                                packet.p);
                            if (AVERROR_EOF == decoding)
                            {
                                decoding = 0;
                            }
                            else if (decoding < 0)
                            {
                                _setError(decoding);
                                break;
                            }
                        }
                        decoding = _decode(currentTime);
                        if (AVERROR(EAGAIN) == decoding)
                        {
                            decoding = 0;
                        }
                        else if (AVERROR_EOF == decoding)
                        {
                            const size_t bufferSize = getSampleCount(_buffer);
                            const size_t bufferMax = _options.audioBufferSize.rescaled_to(_info.sampleRate).value();
                            if (bufferSize < bufferMax)
                            {
                                auto audio = Audio::create(_info, bufferMax - bufferSize);
                                audio->zero();
                                _buffer.push_back(audio);
                            }
                            break;
                        }
                        else if (decoding < 0)
                        {
                            _setError(decoding);
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
            }
            return out;
        }

        size_t ReadAudio::getBufferSize() const
        {
            return getSampleCount(_buffer);
        }

        void ReadAudio::bufferCopy(uint8_t* out, size_t sampleCount)
        {
            moveAudio(_buffer, out, sampleCount);
        }

        namespace
        {
            size_t getByteCount(AVSampleFormat format)
            {
                size_t out = 0;
                switch (format)
                {
                case AV_SAMPLE_FMT_U8:
                case AV_SAMPLE_FMT_U8P:
                    out = 1;
                    break;
                case AV_SAMPLE_FMT_S16:
                case AV_SAMPLE_FMT_S16P:
                    out = 2;
                    break;
                case AV_SAMPLE_FMT_S32:
                case AV_SAMPLE_FMT_FLT:
                case AV_SAMPLE_FMT_S32P:
                case AV_SAMPLE_FMT_FLTP:
                    out = 4;
                    break;
                case AV_SAMPLE_FMT_DBL:
                case AV_SAMPLE_FMT_DBLP:
                case AV_SAMPLE_FMT_S64:
                case AV_SAMPLE_FMT_S64P:
                    out = 8;
                    break;
                default: break;
                }
                return out;
            }
        }

        int ReadAudio::_decode(const OTIO_NS::RationalTime& currentTime)
        {
            // Take every frame the decoders have into the plane queues.
            size_t eofCount = 0;
            for (size_t i = 0; i < _avStreams.size(); ++i)
            {
                while (true)
                {
                    const int r = avcodec_receive_frame(_avCodecContext[_avStreams[i]], _avFrame);
                    if (AVERROR_EOF == r)
                    {
                        ++eofCount;
                        break;
                    }
                    else if (AVERROR(EAGAIN) == r)
                    {
                        break;
                    }
                    else if (r < 0)
                    {
                        return r;
                    }
                    _queueFrame(i, currentTime);
                    av_frame_unref(_avFrame);
                }
            }

            // Convert the samples every queue has. A merged stream that
            // runs short holds the others back until its next packet, so
            // the channels stay in step.
            size_t sampleCount = std::numeric_limits<size_t>::max();
            for (const auto& plane : _planes)
            {
                sampleCount = std::min(sampleCount, plane.size() / _planeByteCount);
            }
            if (!_planes.empty() && sampleCount > 0)
            {
                const int swrOutputSamples = swr_get_out_samples(_swrContext, sampleCount);
                auto swrOutputBuffer = Audio::create(_info, swrOutputSamples);
                std::vector<const uint8_t*> swrInputBufferP;
                for (const auto& plane : _planes)
                {
                    swrInputBufferP.push_back(plane.data());
                }
                uint8_t* swrOutputBufferP[] = { swrOutputBuffer->getData() };
                const int swrOutputCount = swr_convert(
                    _swrContext,
                    swrOutputBufferP,
                    swrOutputSamples,
                    swrInputBufferP.data(),
                    sampleCount);
                auto tmp = Audio::create(_info, swrOutputCount > 0 ? swrOutputCount : 0);
                memcpy(tmp->getData(), swrOutputBuffer->getData(), tmp->getByteCount());
                _buffer.push_back(tmp);
                for (auto& plane : _planes)
                {
                    plane.erase(plane.begin(), plane.begin() + sampleCount * _planeByteCount);
                }
                return 1;
            }
            return eofCount == _avStreams.size() ? AVERROR_EOF : AVERROR(EAGAIN);
        }

        void ReadAudio::_queueFrame(size_t streamIndex, const OTIO_NS::RationalTime& currentTime)
        {
            const int stream = _avStreams[streamIndex];
            const int64_t timestamp = _avFrame->pts != AV_NOPTS_VALUE ? _avFrame->pts : _avFrame->pkt_dts;
            AVRational r;
            r.num = 1;
            r.den = _info.sampleRate;
            const int64_t time =
                _timeRange.start_time().value() +
                av_rescale_q(timestamp, _avFormatContext->streams[stream]->time_base, r);

            // A frame from before the time wanted is dropped, and one that
            // straddles it is taken from that time.
            if (time + (_avFrame->nb_samples - 1) < currentTime.value())
            {
                return;
            }
            const int64_t skip = time < currentTime.value() ? currentTime.value() - time : 0;
            const int64_t count = _avFrame->nb_samples - skip;
            if (count <= 0)
            {
                return;
            }
            const auto avFormat = static_cast<AVSampleFormat>(_avFrame->format);
            const size_t byteCount = getByteCount(avFormat);
            const auto append = [](std::vector<uint8_t>& plane, const uint8_t* data, size_t size)
            {
                plane.insert(plane.end(), data, data + size);
            };
            if (_avStreams.size() > 1)
            {
                // A merged stream is mono: its samples are one plane.
                append(_planes[streamIndex], _avFrame->extended_data[0] + skip * byteCount, count * byteCount);
            }
            else if (av_sample_fmt_is_planar(avFormat))
            {
                for (int c = 0; c < _avFrame->ch_layout.nb_channels; ++c)
                {
                    append(_planes[c], _avFrame->extended_data[c] + skip * byteCount, count * byteCount);
                }
            }
            else
            {
                const size_t frameByteCount = byteCount * _avFrame->ch_layout.nb_channels;
                append(_planes[0], _avFrame->extended_data[0] + skip * frameByteCount, count * frameByteCount);
            }
        }
    }
}

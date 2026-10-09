// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/FFmpegReadPrivate.h>

#include <tlCore/StringFormat.h>
#include <tlCore/LogSystem.h>

extern "C"
{
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>

} // extern "C"

#if defined(__APPLE__)
#include <VideoToolbox/VideoToolbox.h>
#endif // __APPLE__

namespace tl
{
    namespace ffmpeg
    {
        namespace
        {
            //! Whether a decoder offers any hardware configuration that
            //! works through a device context.
            bool hasHwConfig(const AVCodec* codec)
            {
                for (int i = 0;; ++i)
                {
                    const AVCodecHWConfig* config =
                        avcodec_get_hw_config(codec, i);
                    if (!config)
                    {
                        break;
                    }
                    if (config->methods &
                        AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)
                    {
                        return true;
                    }
                }
                return false;
            }
        }

        ReadVideo::ReadVideo(
            const std::string& fileName,
            const std::vector<file::MemoryRead>& memory,
            const ReadOptions& options,
            const std::shared_ptr<log::System>& logSystem) :
            _fileName(fileName),
            _options(options),
            _logSystem(logSystem)
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
                    throw std::runtime_error(
                        string::Format("{0}: \"{1}\"").
                        arg(getErrorLabel(r)).
                        arg(fileName));
                }

                r = avformat_find_stream_info(_avFormatContext, nullptr);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: \"{1}\"").
                        arg(getErrorLabel(r)).
                        arg(fileName));
                }
                //for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
                //    av_dump_format(_avFormatContext, i, fileName.c_str(), 0);
                _avStream = findStream(_avFormatContext, AVMEDIA_TYPE_VIDEO);
                const std::string timecode = getTimecode(_avFormatContext);
                // Only the video stream is read: the demuxer then skips the
                // others' data rather than handing it over to be thrown away.
                // The timecode and the other streams' parameters are
                // metadata, found already.
                for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
                {
                    if (static_cast<int>(i) != _avStream)
                    {
                        _avFormatContext->streams[i]->discard = AVDISCARD_ALL;
                    }
                }
                if (_avStream != -1)
                {
                    //av_dump_format(_avFormatContext, _avStream, fileName.c_str(), 0);

                    auto avVideoStream = _avFormatContext->streams[_avStream];
                    auto avVideoCodecParameters = avVideoStream->codecpar;
                    const AVCodec* avVideoCodecDefault =
                        avcodec_find_decoder(avVideoCodecParameters->codec_id);
                    if (!avVideoCodecDefault)
                    {
                        throw std::runtime_error(
                            string::Format("No video codec found: \"{0}\"").
                            arg(fileName));
                    }
                    _avCodecDefault = avVideoCodecDefault;
                    const AVCodec* avVideoCodec = avVideoCodecDefault;
                    if (options.hwAccel && !hasHwConfig(avVideoCodecDefault))
                    {
                        // The default decoder for a codec can be a software
                        // library with no hardware configurations at all --
                        // FFmpeg answers AV1 with libaom -- while the
                        // hardware-capable decoder sits behind it (#833). If
                        // the hardware then fails to come up, the default is
                        // put back below.
                        void* i = nullptr;
                        while (const AVCodec* candidate = av_codec_iterate(&i))
                        {
                            if (av_codec_is_decoder(candidate) &&
                                candidate->id == avVideoCodecParameters->codec_id &&
                                hasHwConfig(candidate))
                            {
                                avVideoCodec = candidate;
                                break;
                            }
                        }
                    }
                    _avCodecParameters[_avStream] = avcodec_parameters_alloc();
                    if (!_avCodecParameters[_avStream])
                    {
                        throw std::runtime_error(
                            string::Format("Cannot allocate parameters: \"{0}\"").
                            arg(fileName));
                    }
                    r = avcodec_parameters_copy(_avCodecParameters[_avStream], avVideoCodecParameters);
                    if (r < 0)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: \"{1}\"").
                            arg(getErrorLabel(r)).
                            arg(fileName));
                    }
                    _avCodec = avVideoCodec;
                    r = _openCodec(avVideoCodec, options.hwAccel);
                    if (avVideoCodec != avVideoCodecDefault &&
                        (r < 0 || !_hwAccel))
                    {
                        // The alternative decoder was chosen only for its
                        // hardware; without it -- the device did not come up,
                        // or the codec did not open -- the default software
                        // decoder is the right one after all.
                        if (_hwDeviceContext)
                        {
                            av_buffer_unref(&_hwDeviceContext);
                        }
                        _hwAccel = false;
                        _hwPixelFormat = AV_PIX_FMT_NONE;
                        _avCodec = avVideoCodecDefault;
                        r = _openCodec(_avCodec, false);
                    }
                    if (r < 0)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: \"{1}\"").
                            arg(getErrorLabel(r)).
                            arg(fileName));
                    }

                    _info.size.w = _avCodecParameters[_avStream]->width;
                    _info.size.h = _avCodecParameters[_avStream]->height;
                    // Asked of the format rather than read from the codec
                    // parameters: what a QuickTime carries in its "pasp" atom
                    // reaches the stream, and the parameters copied from the
                    // codec do not have it. An anamorphic movie came back
                    // square, which is what the command line -- ffprobe
                    // reports the stream -- did not do.
                    const AVRational sampleAspectRatio =
                        av_guess_sample_aspect_ratio(
                            _avFormatContext,
                            _avFormatContext->streams[_avStream],
                            nullptr);
                    if (sampleAspectRatio.num > 0 && sampleAspectRatio.den > 0)
                    {
                        _info.pixelAspectRatio = av_q2d(sampleAspectRatio);
                    }
                    _info.layout.mirror.y = true;

                    _avInputPixelFormat = static_cast<AVPixelFormat>(_avCodecParameters[_avStream]->format);
                    switch (_avInputPixelFormat)
                    {
                    case AV_PIX_FMT_RGB24:
                        _avOutputPixelFormat = _avInputPixelFormat;
                        _info.type = image::ImageType::RGB_U8;
                        break;
                    case AV_PIX_FMT_GRAY8:
                        _avOutputPixelFormat = _avInputPixelFormat;
                        _info.type = image::ImageType::L_U8;
                        break;
                    case AV_PIX_FMT_RGBA:
                        _avOutputPixelFormat = _avInputPixelFormat;
                        _info.type = image::ImageType::RGBA_U8;
                        break;
                    case AV_PIX_FMT_YUV420P:
                        if (options.yuvToRGBConversion)
                        {
                            _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                            _info.type = image::ImageType::RGB_U8;
                        }
                        else
                        {
                            _avOutputPixelFormat = _avInputPixelFormat;
                            _info.type = image::ImageType::YUV_420P_U8;
                        }
                        break;
                    case AV_PIX_FMT_YUV422P:
                        if (options.yuvToRGBConversion)
                        {
                            _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                            _info.type = image::ImageType::RGB_U8;
                        }
                        else
                        {
                            _avOutputPixelFormat = _avInputPixelFormat;
                            _info.type = image::ImageType::YUV_422P_U8;
                        }
                        break;
                    case AV_PIX_FMT_YUV444P:
                        if (options.yuvToRGBConversion)
                        {
                            _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                            _info.type = image::ImageType::RGB_U8;
                        }
                        else
                        {
                            _avOutputPixelFormat = _avInputPixelFormat;
                            _info.type = image::ImageType::YUV_444P_U8;
                        }
                        break;
                    case AV_PIX_FMT_YUV420P10BE:
                    case AV_PIX_FMT_YUV420P10LE:
                    case AV_PIX_FMT_YUV420P12BE:
                    case AV_PIX_FMT_YUV420P12LE:
                    case AV_PIX_FMT_YUV420P16BE:
                    case AV_PIX_FMT_YUV420P16LE:
                        if (options.yuvToRGBConversion)
                        {
                            _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                            _info.type = image::ImageType::RGB_U16;
                        }
                        else
                        {
                            //! \todo Use the _info.layout.endian field instead of
                            //! converting endianness.
                            _avOutputPixelFormat = AV_PIX_FMT_YUV420P16LE;
                            _info.type = image::ImageType::YUV_420P_U16;
                        }
                        break;
                    case AV_PIX_FMT_YUV422P10BE:
                    case AV_PIX_FMT_YUV422P10LE:
                    case AV_PIX_FMT_YUV422P12BE:
                    case AV_PIX_FMT_YUV422P12LE:
                    case AV_PIX_FMT_YUV422P16BE:
                    case AV_PIX_FMT_YUV422P16LE:
                        if (options.yuvToRGBConversion)
                        {
                            _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                            _info.type = image::ImageType::RGB_U16;
                        }
                        else
                        {
                            //! \todo Use the _info.layout.endian field instead of
                            //! converting endianness.
                            _avOutputPixelFormat = AV_PIX_FMT_YUV422P16LE;
                            _info.type = image::ImageType::YUV_422P_U16;
                        }
                        break;
                    case AV_PIX_FMT_YUV444P10BE:
                    case AV_PIX_FMT_YUV444P10LE:
                    case AV_PIX_FMT_YUV444P12BE:
                    case AV_PIX_FMT_YUV444P12LE:
                    case AV_PIX_FMT_YUV444P16BE:
                    case AV_PIX_FMT_YUV444P16LE:
                        if (options.yuvToRGBConversion)
                        {
                            _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                            _info.type = image::ImageType::RGB_U16;
                        }
                        else
                        {
                            //! \todo Use the _info.layout.endian field instead of
                            //! converting endianness.
                            _avOutputPixelFormat = AV_PIX_FMT_YUV444P16LE;
                            _info.type = image::ImageType::YUV_444P_U16;
                        }
                        break;
                    case AV_PIX_FMT_GBRP:
                    case AV_PIX_FMT_BGR24:
                    case AV_PIX_FMT_BGR0:
                    case AV_PIX_FMT_RGB0:
                        // RGB in another layout, as FFV1 stores it: converted
                        // to RGB rather than falling to the default below,
                        // which takes it for YUV and loses what a lossless
                        // file kept.
                        _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                        _info.type = image::ImageType::RGB_U8;
                        break;
                    case AV_PIX_FMT_BGRA:
                    case AV_PIX_FMT_ARGB:
                    case AV_PIX_FMT_ABGR:
                    case AV_PIX_FMT_PAL8:
                        // RGB with alpha in another layout, or through a
                        // palette, as a GIF is decoded: likewise converted
                        // to what it is rather than taken for YUV, which
                        // halved the color's resolution.
                        _avOutputPixelFormat = AV_PIX_FMT_RGBA;
                        _info.type = image::ImageType::RGBA_U8;
                        break;
                    case AV_PIX_FMT_GBRP9BE:
                    case AV_PIX_FMT_GBRP9LE:
                    case AV_PIX_FMT_GBRP10BE:
                    case AV_PIX_FMT_GBRP10LE:
                    case AV_PIX_FMT_GBRP12BE:
                    case AV_PIX_FMT_GBRP12LE:
                    case AV_PIX_FMT_GBRP14BE:
                    case AV_PIX_FMT_GBRP14LE:
                    case AV_PIX_FMT_GBRP16BE:
                    case AV_PIX_FMT_GBRP16LE:
                    case AV_PIX_FMT_RGB48BE:
                    case AV_PIX_FMT_RGB48LE:
                        _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        _info.type = image::ImageType::RGB_U16;
                        break;
                    case AV_PIX_FMT_YUVA420P:
                    case AV_PIX_FMT_YUVA422P:
                    case AV_PIX_FMT_YUVA444P:
                        //! \todo Support these formats natively.
                        _avOutputPixelFormat = AV_PIX_FMT_RGBA;
                        _info.type = image::ImageType::RGBA_U8;
                        break;
                    case AV_PIX_FMT_YUVA444P10BE:
                    case AV_PIX_FMT_YUVA444P10LE:
                    case AV_PIX_FMT_YUVA444P12BE:
                    case AV_PIX_FMT_YUVA444P12LE:
                    case AV_PIX_FMT_YUVA444P16BE:
                    case AV_PIX_FMT_YUVA444P16LE:
                        //! \todo Support these formats natively.
                        _avOutputPixelFormat = AV_PIX_FMT_RGBA64;
                        _info.type = image::ImageType::RGBA_U16;
                        break;
                    default:
                        if (options.yuvToRGBConversion)
                        {
                            _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                            _info.type = image::ImageType::RGB_U8;
                        }
                        else
                        {
                            _avOutputPixelFormat = AV_PIX_FMT_YUV420P;
                            _info.type = image::ImageType::YUV_420P_U8;
                        }
                        break;
                    }
                    if (_hwAccel)
                    {
                        // Hardware frames download as NV12 (8-bit) or P010 (>8-bit).
                        // They are handed to the display shader as semi-planar YUV
                        // with no colour conversion -- the shader performs YUV->RGB
                        // exactly as for software-decoded YUV, so hardware and
                        // software decoding match, and the cache keeps the smaller
                        // YUV footprint. _avOutputPixelFormat doubles as the expected
                        // download format and the sws fallback target (see _copy()).
                        const AVPixFmtDescriptor* desc =
                            av_pix_fmt_desc_get(_avInputPixelFormat);
                        const bool gt8 = desc && desc->comp[0].depth > 8;
                        _avOutputPixelFormat = gt8 ? AV_PIX_FMT_P010LE : AV_PIX_FMT_NV12;
                        _info.type = gt8 ? image::ImageType::YUV_420SP_U16 : image::ImageType::YUV_420SP_U8;
                    }
                    if (!isFullRange(
                        _avCodecContext[_avStream]->color_range,
                        _avInputPixelFormat))
                    {
                        _info.videoLevels = file_wrong::VideoLevels::LegalRange;
                    }
                    const bool convertedToRGB =
                        _avInputPixelFormat != _avOutputPixelFormat &&
                        (AV_PIX_FMT_RGB24  == _avOutputPixelFormat ||
                         AV_PIX_FMT_RGB48  == _avOutputPixelFormat ||
                         AV_PIX_FMT_RGBA   == _avOutputPixelFormat ||
                         AV_PIX_FMT_RGBA64 == _avOutputPixelFormat);
                    if (convertedToRGB)
                    {
                        _info.videoLevels = file_wrong::VideoLevels::FullRange;
                    }
                    _info.yuvCoefficients = toYUVCoefficients(
                        _avCodecParameters[_avStream]->color_space,
                        _avInputPixelFormat,
                        _info.size);

                    _avSpeed = av_guess_frame_rate(_avFormatContext, avVideoStream, nullptr);
                    const double speed = av_q2d(_avSpeed);

                    std::size_t sequenceSize = 0;
                    if (avVideoStream->nb_frames > 0)
                    {
                        sequenceSize = avVideoStream->nb_frames;
                    }
                    else if (avVideoStream->duration != AV_NOPTS_VALUE)
                    {
                        sequenceSize = av_rescale_q(
                            avVideoStream->duration,
                            avVideoStream->time_base,
                            swap(avVideoStream->r_frame_rate));
                    }
                    else if (_avFormatContext->duration != AV_NOPTS_VALUE)
                    {
                        sequenceSize = av_rescale_q(
                            _avFormatContext->duration,
                            av_get_time_base_q(),
                            swap(avVideoStream->r_frame_rate));
                    }
        
                    image::Tags tags;
                    AVDictionaryEntry* tag = nullptr;
                    while ((tag = av_dict_get(_avFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                    {
                        const std::string key(tag->key);
                        const std::string value(tag->value);
                        tags[key] = value;
                    }

                    OTIO_NS::RationalTime startTime(0.0, speed);
                    if (!timecode.empty())
                    {
                        opentime::ErrorStatus errorStatus;
                        const OTIO_NS::RationalTime time = OTIO_NS::RationalTime::from_timecode(
                            timecode,
                            speed,
                            &errorStatus);
                        if (!opentime::is_error(errorStatus))
                        {
                            startTime = time.floor();
                        }
                    }
                    _timeRange = OTIO_NS::TimeRange(
                        startTime,
                        OTIO_NS::RationalTime(sequenceSize, speed));

                    for (const auto& i : tags)
                    {
                        _tags[i.first] = i.second;
                    }

                    // The stream's color description, for resolving an
                    // input color space from what the file itself says.
                    // Only what was actually flagged; "unspecified" says
                    // nothing worth repeating.
                    const AVCodecParameters* codecPar = avVideoStream->codecpar;
                    if (codecPar->color_primaries != AVCOL_PRI_UNSPECIFIED)
                    {
                        if (const char* name = av_color_primaries_name(codecPar->color_primaries))
                        {
                            _tags["Color Primaries"] = name;
                        }
                    }
                    if (codecPar->color_trc != AVCOL_TRC_UNSPECIFIED)
                    {
                        if (const char* name = av_color_transfer_name(codecPar->color_trc))
                        {
                            _tags["Color Transfer"] = name;
                        }
                    }
                    if (codecPar->color_space != AVCOL_SPC_UNSPECIFIED)
                    {
                        if (const char* name = av_color_space_name(codecPar->color_space))
                        {
                            _tags["Color Matrix"] = name;
                        }
                    }
                    {
                        _source.codec =
                            avcodec_get_name(_avCodecContext[_avStream]->codec_id);
                        if (const char* name = av_get_pix_fmt_name(_avInputPixelFormat))
                        {
                            _source.pixelFormat = name;
                        }
                    }
                }
            }
            catch (...)
            {
                _close();
                throw;
            }
        }

        ReadVideo::~ReadVideo()
        {
            _close();
        }

        void ReadVideo::_close()
        {
            if (_swsContext)
            {
                sws_freeContext(_swsContext);
                _swsContext = nullptr;
            }
            if (_avFrame2)
            {
                av_frame_free(&_avFrame2);
            }
            if (_avFrame)
            {
                av_frame_free(&_avFrame);
            }
            if (_swFrame)
            {
                av_frame_free(&_swFrame);
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
            if (_hwDeviceContext)
            {
                av_buffer_unref(&_hwDeviceContext);
            }
        }

        bool ReadVideo::isValid() const
        {
            return _avStream != -1;
        }

        const image::Info& ReadVideo::getInfo() const
        {
            return _info;
        }

        const OTIO_NS::TimeRange& ReadVideo::getTimeRange() const
        {
            return _timeRange;
        }

        const VideoSourceInfo& ReadVideo::getSource() const
    {
        return _source;
    }

    const image::Tags& ReadVideo::getTags() const
        {
            return _tags;
        }

        namespace
        {
            std::string pixelFormatName(AVPixelFormat value)
            {
                const char* name = av_get_pix_fmt_name(value);
                return name ? name : "unknown";
            }

            bool canCopy(AVPixelFormat in, AVPixelFormat out)
            {
                return in == out &&
                    (AV_PIX_FMT_RGB24   == in ||
                     AV_PIX_FMT_GRAY8   == in ||
                     AV_PIX_FMT_RGBA    == in ||
                     AV_PIX_FMT_YUV420P == in);
            }
        }

        void ReadVideo::start()
        {
            if (_avStream != -1)
            {
                _avFrame = av_frame_alloc();
                if (!_avFrame)
                {
                    throw std::runtime_error(
                        string::Format("Cannot allocate frame: \"{0}\"").
                        arg(_fileName));
                }

                if (!canCopy(_avInputPixelFormat, _avOutputPixelFormat))
                {
                    _initFrame2();

                    if (_hwAccel)
                    {
                        _swFrame = av_frame_alloc();
                        if (!_swFrame)
                        {
                            throw std::runtime_error(
                                string::Format("Cannot allocate frame: \"{0}\"").
                                arg(_fileName));
                        }
                        // The scaler is created lazily in _copy(), once the real
                        // source format is known (the hardware download format, or
                        // the decoder's software-fallback format).
                    }
                    else
                    {
                        _initSws(_avInputPixelFormat);
                    }
                }
            }
        }

        void ReadVideo::_log(const std::string& message, log::Type type) const
        {
            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print("tl::ffmpeg::ReadVideo", message, type);
            }
        }

        AVPixelFormat ReadVideo::_getHwFormat(
            AVCodecContext* context,
            const AVPixelFormat* formats)
        {
            auto self = static_cast<ReadVideo*>(context->opaque);
            for (const AVPixelFormat* p = formats; *p != AV_PIX_FMT_NONE; ++p)
            {
                if (*p == self->_hwPixelFormat)
                {
                    return *p;
                }
            }
            // The hardware format was not offered for this stream; let the decoder
            // fall back to a software format. _decode() detects this per frame (the
            // frame format will not match _hwPixelFormat, so no download happens)
            // and _copy() builds the scaler from whatever format actually arrives.
            return formats[0];
        }

        namespace
        {
#if defined(__APPLE__)
            //! Whether this machine has a hardware decoder for a codec.
            //!
            //! VideoToolbox does not refuse a codec it has no hardware for:
            //! it decodes in software and hands the frames back the same
            //! way, which is slower than FFmpeg's own decoder on every core
            //! -- an Intel Mac without an HEVC decoder played 1080p at two
            //! thirds speed through it. A codec not listed here is not
            //! asked about, and is left to VideoToolbox as before.
            bool hasVideoToolboxDecoder(AVCodecID id)
            {
                CMVideoCodecType type = 0;
                switch (id)
                {
                case AV_CODEC_ID_H264: type = kCMVideoCodecType_H264; break;
                case AV_CODEC_ID_HEVC: type = kCMVideoCodecType_HEVC; break;
                // The four character codes themselves, since the names for
                // these are newer than the oldest system supported.
                case AV_CODEC_ID_VP9: type = 'vp09'; break;
                case AV_CODEC_ID_AV1: type = 'av01'; break;
                default: return true;
                }
                return VTIsHardwareDecodeSupported(type);
            }
#endif // __APPLE__
        }

        void ReadVideo::_initHwAccel(const AVCodec* codec)
        {
            // The hardware path outputs limited-range YUV, so full-range
            // sources stay on the software decoder, and chroma other than
            // 4:2:0 stays gated for now: newer hardware decodes 4:2:2 and
            // 4:4:4 and this machine's VideoToolbox hands back genuine P210,
            // but the 4:2:2 pixels that reach the screen measurably differ
            // from the software path's at sharp chroma edges, and until that
            // difference is accounted for the gate stands -- hardware
            // decoding is always a faithful match, it never alters the
            // image. The first-frame chroma and depth check below the gate
            // is groundwork for lifting it.
            const AVPixelFormat inputFormat =
                static_cast<AVPixelFormat>(_avCodecParameters[_avStream]->format);
            const AVPixFmtDescriptor* inputDesc = av_pix_fmt_desc_get(inputFormat);
            const bool is420 = inputDesc &&
                1 == inputDesc->log2_chroma_w && 1 == inputDesc->log2_chroma_h;
            if (AVCOL_RANGE_JPEG == _avCodecParameters[_avStream]->color_range)
            {
                _log(
                    string::Format("Hardware decoding skipped for a full-range source; using software decoding: \"{0}\"").arg(_fileName),
                    log::Type::Warning);
                return;
            }
            if (!is420)
            {
                _log(
                    string::Format("Hardware decoding skipped for a non-4:2:0 source; using software decoding: \"{0}\"").arg(_fileName),
                    log::Type::Warning);
                return;
            }
#if defined(__APPLE__)
            if (!hasVideoToolboxDecoder(codec->id))
            {
                _log(
                    string::Format("This machine has no hardware decoder for the codec \"{0}\"; using software decoding").
                    arg(codec->name ? codec->name : "?"),
                    log::Type::Warning);
                return;
            }
#endif // __APPLE__
            // The platform's native API is preferred, but the machine
            // decides: every configuration the codec offers is tried
            // until a device actually creates. A fixed choice broke on
            // hardware that speaks a different API -- VAAPI means
            // nothing to an NVIDIA driver, whose path is CUDA (#833).
#if defined(__APPLE__)
            const AVHWDeviceType preferred = AV_HWDEVICE_TYPE_VIDEOTOOLBOX;
#elif defined(_WIN32)
            const AVHWDeviceType preferred = AV_HWDEVICE_TYPE_D3D11VA;
#else
            const AVHWDeviceType preferred = AV_HWDEVICE_TYPE_VAAPI;
#endif
            AVPixelFormat hwFormat = AV_PIX_FMT_NONE;
            AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
            AVBufferRef* device = nullptr;
            bool codecHasHw = false;
            std::string tried;
            for (int pass = 0; pass < 2 && !device; ++pass)
            {
                for (int i = 0; !device; ++i)
                {
                    const AVCodecHWConfig* config =
                        avcodec_get_hw_config(codec, i);
                    if (!config)
                    {
                        break;
                    }
                    if (!(config->methods &
                        AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX))
                    {
                        continue;
                    }
                    codecHasHw = true;
                    const bool isPreferred =
                        config->device_type == preferred;
                    if (0 == pass ? !isPreferred : isPreferred)
                    {
                        continue;
                    }
                    if (av_hwdevice_ctx_create(
                        &device, config->device_type,
                        nullptr, nullptr, 0) >= 0)
                    {
                        hwFormat = config->pix_fmt;
                        type = config->device_type;
                    }
                    else
                    {
                        if (!tried.empty())
                        {
                            tried += ", ";
                        }
                        tried += av_hwdevice_get_type_name(
                            config->device_type);
                    }
                }
            }
            if (!codecHasHw)
            {
                // This codec has no hardware support in this build of
                // FFmpeg; stay on the software path.
                _log(
                    string::Format("Hardware decoding is not available for the codec \"{0}\"; using software decoding").
                    arg(codec->name ? codec->name : "?"),
                    log::Type::Warning);
                return;
            }
            if (!device)
            {
                _log(
                    string::Format("Cannot create a hardware decoding device ({0}); using software decoding").
                    arg(tried),
                    log::Type::Warning);
                return;
            }
            _hwDeviceContext = device;
            _hwPixelFormat = hwFormat;
            _hwAccel = true;
            _avCodecContext[_avStream]->hw_device_ctx = av_buffer_ref(_hwDeviceContext);
            _avCodecContext[_avStream]->opaque = this;
            _avCodecContext[_avStream]->get_format = _getHwFormat;
            _log(
                string::Format("Hardware decoding enabled ({0}) for the codec \"{1}\"").
                arg(av_hwdevice_get_type_name(type)).
                arg(codec->name ? codec->name : "?"));
        }

        void ReadVideo::_initFrame2()
        {
            _avFrame2 = av_frame_alloc();
            if (!_avFrame2)
            {
                throw std::runtime_error(
                    string::Format("Cannot allocate frame: \"{0}\"").
                    arg(_fileName));
            }
            _avFrame2->format = _avOutputPixelFormat;
            _avFrame2->width = _info.size.w;
            _avFrame2->height = _info.size.h;
            _avFrame2->buf[0] = av_buffer_alloc(_info.getByteCount());
        }

        void ReadVideo::_initSws(AVPixelFormat srcFormat)
        {
            // May be a rebuild: see _copy().
            if (_swsContext)
            {
                sws_freeContext(_swsContext);
                _swsContext = nullptr;
            }
            _swsContext = sws_alloc_context();
            if (!_swsContext)
            {
                throw std::runtime_error(string::Format("Cannot allocate context: \"{0}\"").arg(_fileName));
            }
            av_opt_set_defaults(_swsContext);
            int r = av_opt_set_int(_swsContext, "srcw", _avCodecParameters[_avStream]->width, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "srch", _avCodecParameters[_avStream]->height, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "src_format", srcFormat, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "dstw", _avCodecParameters[_avStream]->width, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "dsth", _avCodecParameters[_avStream]->height, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "dst_format", _avOutputPixelFormat, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "sws_flags", swsReadFlags, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(_swsContext, "threads", _options.threadCount, AV_OPT_SEARCH_CHILDREN);
            r = sws_init_context(_swsContext, nullptr, nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("Cannot initialize sws context: \"{0}\"").arg(_fileName));
            }
            // The matrix and range to convert with, which the scaler does not
            // take from the frames: left to its defaults it converts with
            // BT.601 whatever the stream says, and moves full range JPEG
            // pixels to video range, which the image is then not described
            // as. A YUV output keeps the source's range; RGB is full.
            const AVPixFmtDescriptor* srcDesc = av_pix_fmt_desc_get(srcFormat);
            const AVPixFmtDescriptor* dstDesc = av_pix_fmt_desc_get(_avOutputPixelFormat);
            const bool srcFull =
                (srcDesc && (srcDesc->flags & AV_PIX_FMT_FLAG_RGB)) ||
                isFullRange(_avCodecContext[_avStream]->color_range, srcFormat);
            const bool dstFull =
                (dstDesc && (dstDesc->flags & AV_PIX_FMT_FLAG_RGB)) ||
                srcFull;
            const int* coefficients = sws_getCoefficients(
                toSwsColorspace(_info.yuvCoefficients));
            sws_setColorspaceDetails(
                _swsContext,
                coefficients,
                srcFull ? 1 : 0,
                coefficients,
                dstFull ? 1 : 0,
                0,
                1 << 16,
                1 << 16);
            // Recorded once there is a scaler it describes.
            _swsInputPixelFormat = srcFormat;
        }

        int ReadVideo::_openCodec(const AVCodec* codec, bool hwAccel)
        {
            if (_avCodecContext[_avStream])
            {
                avcodec_free_context(&_avCodecContext[_avStream]);
            }
            _avCodecContext[_avStream] = avcodec_alloc_context3(codec);
            if (!_avCodecContext[_avStream])
            {
                throw std::runtime_error(
                    string::Format("Cannot allocate context: \"{0}\"").
                    arg(_fileName));
            }
            int r = avcodec_parameters_to_context(
                _avCodecContext[_avStream],
                _avCodecParameters[_avStream]);
            if (r < 0)
            {
                throw std::runtime_error(
                    string::Format("{0}: \"{1}\"").
                    arg(getErrorLabel(r)).
                    arg(_fileName));
            }
            _avCodecContext[_avStream]->thread_count = _options.threadCount;
            // Frames of an intra only codec -- DNxHR, ProRes, MJPEG -- are
            // decoded across the threads a slice at a time rather than a
            // frame to a thread. Frame threading keeps a frame in flight for
            // every thread, and a seek throws them away and waits for them
            // again: a reader behind the playhead seeks on every request,
            // and an 8K DNxHR 444 movie decoded at ten frames a second
            // rather than twenty. Slices have nothing in flight, and scale
            // further: decoding that movie from memory, 67 frames a second
            // on 32 threads, where frames peaked at 47 on 16.
            const AVCodecDescriptor* descriptor = avcodec_descriptor_get(codec->id);
            const bool intraOnly = descriptor && (descriptor->props & AV_CODEC_PROP_INTRA_ONLY);
            const bool sliceThreads = codec->capabilities & AV_CODEC_CAP_SLICE_THREADS;
            _avCodecContext[_avStream]->thread_type = intraOnly && sliceThreads ?
                FF_THREAD_SLICE :
                FF_THREAD_FRAME;
            if (hwAccel)
            {
                // Attempt hardware decode. On any failure this is a no-op
                // and decoding stays on the software path.
                _initHwAccel(codec);
            }
            return avcodec_open2(_avCodecContext[_avStream], codec, 0);
        }

        bool ReadVideo::_hwFallback(const OTIO_NS::RationalTime& currentTime)
        {
            // A hardware-only decoder can open and still fail at the first
            // frame -- the device exists, but the codec it was asked for does
            // not, an AV1 file on a GPU without AV1 decode -- and hardware on
            // the default decoder can prove unfaithful at the first frame.
            // Either way: rebuild on the software default and pick up from
            // the current time.
            if (!_avCodecDefault ||
                (_avCodec == _avCodecDefault && !_hwAccel))
            {
                return false;
            }
            _log(
                string::Format("Hardware decoding failed for the codec \"{0}\"; using software decoding: \"{1}\"").
                arg(_avCodec->name ? _avCodec->name : "?").
                arg(_fileName),
                log::Type::Warning);
            if (_hwDeviceContext)
            {
                av_buffer_unref(&_hwDeviceContext);
            }
            _hwAccel = false;
            _hwPixelFormat = AV_PIX_FMT_NONE;
            _avCodec = _avCodecDefault;
            if (_openCodec(_avCodec, false) < 0)
            {
                return false;
            }
            seek(currentTime);
            return true;
        }

        void ReadVideo::seek(const OTIO_NS::RationalTime& time)
        {

            if (_avStream != -1)
            {
                avcodec_flush_buffers(_avCodecContext[_avStream]);

                const int seekError = av_seek_frame(
                    _avFormatContext,
                    _avStream,
                    av_rescale_q(
                        time.value() - _timeRange.start_time().value(),
                        swap(_avSpeed),
                        _avFormatContext->streams[_avStream]->time_base),
                    AVSEEK_FLAG_BACKWARD);
                if (seekError < 0)
                {
                    _setError(seekError);
                }
            }

            _buffer.clear();
            _eof = false;
        }

        size_t ReadVideo::getErrorCount() const
        {
            return _errorCount;
        }

        const std::string& ReadVideo::getErrorString() const
        {
            return _errorString;
        }

        void ReadVideo::_setError(int error)
        {
            ++_errorCount;
            if (_errorString.empty())
            {
                _errorString = getErrorLabel(error);
            }
        }

        bool ReadVideo::process(const OTIO_NS::RationalTime& currentTime)
        {
            bool out = false;
            if (_avStream != -1 &&
                _buffer.size() < _options.videoBufferSize)
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
                    if ((_eof && _avStream != -1) || (_avStream == packet.p->stream_index))
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
                            if (_hwFallback(currentTime))
                            {
                                decoding = 0;
                                continue;
                            }
                            _setError(decoding);
                            break;
                        }
                        decoding = _decode(currentTime);
                        if (AVERROR(EAGAIN) == decoding)
                        {
                            decoding = 0;
                        }
                        else if (AVERROR_EOF == decoding)
                        {
                            break;
                        }
                        else if (decoding < 0)
                        {
                            if (_hwFallback(currentTime))
                            {
                                decoding = 0;
                                continue;
                            }
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

        bool ReadVideo::isBufferEmpty() const
        {
            return _buffer.empty();
        }

        std::shared_ptr<image::Image> ReadVideo::popBuffer()
        {
            std::shared_ptr<image::Image> out;
            if (!_buffer.empty())
            {
                out = _buffer.front();
                _buffer.pop_front();
            }
            return out;
        }

        int ReadVideo::_decode(const OTIO_NS::RationalTime& currentTime)
        {
            int out = 0;
            while (0 == out)
            {
                out = avcodec_receive_frame(_avCodecContext[_avStream], _avFrame);
                if (out < 0)
                {
                    return out;
                }
                AVFrame* frame = _avFrame;
                if (_hwAccel && _avFrame->format == _hwPixelFormat)
                {
                    // Download the hardware surface to a CPU frame (NV12/P010).
                    av_frame_unref(_swFrame);
                    if (av_hwframe_transfer_data(_swFrame, _avFrame, 0) < 0)
                    {
                        _log(
                            string::Format("Cannot download a hardware frame; skipping: \"{0}\"").
                            arg(_fileName),
                            log::Type::Warning);
                        continue;
                    }
                    av_frame_copy_props(_swFrame, _avFrame);
                    frame = _swFrame;
                    if (!_hwLogged)
                    {
                        // The first downloaded frame says whether the hardware
                        // kept its word: coarser chroma or fewer bits than the
                        // source means the image would not be a faithful match,
                        // and the error puts decoding back on software.
                        const AVPixFmtDescriptor* srcDesc =
                            av_pix_fmt_desc_get(_avInputPixelFormat);
                        const AVPixFmtDescriptor* dlDesc = av_pix_fmt_desc_get(
                            static_cast<AVPixelFormat>(_swFrame->format));
                        if (srcDesc && dlDesc &&
                            (dlDesc->log2_chroma_w > srcDesc->log2_chroma_w ||
                             dlDesc->log2_chroma_h > srcDesc->log2_chroma_h ||
                             dlDesc->comp[0].depth < srcDesc->comp[0].depth))
                        {
                            _log(
                                string::Format("Hardware decoding is coarser than the source ({0} for {1}); using software decoding: \"{2}\"").
                                arg(dlDesc->name ? dlDesc->name : "?").
                                arg(srcDesc->name ? srcDesc->name : "?").
                                arg(_fileName),
                                log::Type::Warning);
                            return AVERROR_EXTERNAL;
                        }
                        // Confirms frames are really decoding on the hardware, as
                        // opposed to the device being attached but the decoder
                        // having fallen back to software (see _getHwFormat()).
                        // The downloaded format is named: it is the first
                        // question when a hardware decode looks wrong.
                        _log(string::Format("Hardware decoding is active ({0}): \"{1}\"").
                            arg(dlDesc && dlDesc->name ? dlDesc->name : "?").
                            arg(_fileName));
                        _hwLogged = true;
                    }
                }
                const int64_t timestamp = _avFrame->pts != AV_NOPTS_VALUE ? _avFrame->pts : _avFrame->pkt_dts;

                const OTIO_NS::RationalTime time(
                    _timeRange.start_time().value() +
                    av_rescale_q(
                        timestamp,
                        _avFormatContext->streams[_avStream]->time_base,
                        swap(_avFormatContext->streams[_avStream]->r_frame_rate)),
                    _timeRange.duration().rate());

                if (time >= currentTime)
                {
                    auto image = image::Image::create(_info);
                    
                    auto tags = _tags;
                    AVDictionaryEntry* tag = nullptr;
                    while ((tag = av_dict_get(_avFrame->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                    {
                        tags[tag->key] = tag->value;
                    }
                    HDRData hdrData;
                    toHDRData(_avFrame->side_data, _avFrame->nb_side_data, hdrData);
                    tags["hdr"] = nlohmann::json(hdrData).dump();
                    image->setTags(tags);

                    _copy(image, frame);
                    _buffer.push_back(image);
                    out = 1;
                    break;
                }
            }
            return out;
        }

        void ReadVideo::_copy(const std::shared_ptr<image::Image>& image, AVFrame* frame)
        {
            const auto& info = image->getInfo();
            const std::size_t w = info.size.w;
            const std::size_t h = info.size.h;
            uint8_t* const data = image->getData();
            if (_hwAccel && frame->format == _avOutputPixelFormat)
            {
                // Native semi-planar copy (NV12/P010): luma plane, then the
                // interleaved chroma plane, straight into the ftk image. No colour
                // conversion -- the display shader does YUV->RGB. The sws fallback
                // below covers the rare case of an unexpected download format.
                const std::size_t bytes =
                    (AV_PIX_FMT_P010LE == _avOutputPixelFormat) ? 2 : 1;
                const uint8_t* const dataY = frame->data[0];
                const uint8_t* const dataUV = frame->data[1];
                const int linesizeY = frame->linesize[0];
                const int linesizeUV = frame->linesize[1];
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(data + w * bytes * i, dataY + linesizeY * i, w * bytes);
                }
                uint8_t* const dataOutUV = data + w * h * bytes;
                const std::size_t h2 = h / 2;
                for (std::size_t i = 0; i < h2; ++i)
                {
                    std::memcpy(dataOutUV + w * bytes * i, dataUV + linesizeUV * i, w * bytes);
                }
                return;
            }
            // The format the stream declares is the format of its first
            // frame, and a file may change it partway through: a QuickTime
            // can carry ProRes 4444 and ProRes 422 frames in one stream, and
            // the 422 stretches decode to a format the header never
            // mentioned. So the arriving frame is asked rather than the
            // header, and the scaler is rebuilt whenever the answer changes.
            const AVPixelFormat frameFormat =
                static_cast<AVPixelFormat>(frame->format);
            if (canCopy(frameFormat, _avOutputPixelFormat))
            {
                const uint8_t* const data0 = frame->data[0];
                const int linesize0 = frame->linesize[0];
                switch (frameFormat)
                {
                case AV_PIX_FMT_RGB24:
                    for (std::size_t i = 0; i < h; ++i)
                    {
                        std::memcpy(
                            data + w * 3 * i,
                            data0 + linesize0 * 3 * i,
                            w * 3);
                    }
                    break;
                case AV_PIX_FMT_GRAY8:
                    for (std::size_t i = 0; i < h; ++i)
                    {
                        std::memcpy(
                            data + w * i,
                            data0 + linesize0 * i,
                            w);
                    }
                    break;
                case AV_PIX_FMT_RGBA:
                    for (std::size_t i = 0; i < h; ++i)
                    {
                        std::memcpy(
                            data + w * 4 * i,
                            data0 + linesize0 * 4 * i,
                            w * 4);
                    }
                    break;
                case AV_PIX_FMT_YUV420P:
                {
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    const uint8_t* const data1 = frame->data[1];
                    const uint8_t* const data2 = frame->data[2];
                    const int linesize1 = frame->linesize[1];
                    const int linesize2 = frame->linesize[2];
                    for (std::size_t i = 0; i < h; ++i)
                    {
                        std::memcpy(
                            data + w * i,
                            data0 + linesize0 * i,
                            w);
                    }
                    for (std::size_t i = 0; i < h2; ++i)
                    {
                        std::memcpy(
                            data + (w * h) + w2 * i,
                            data1 + linesize1 * i,
                            w2);
                        std::memcpy(
                            data + (w * h) + (w2 * h2) + w2 * i,
                            data2 + linesize2 * i,
                            w2);
                    }
                    break;
                }
                default: break;
                }
            }
            else
            {
                if (!_swsContext || _swsInputPixelFormat != frameFormat)
                {
                    if (_swsContext)
                    {
                        _log(string::Format(
                            "The video format changed from \"{0}\" to \"{1}\": \"{2}\"").
                            arg(pixelFormatName(_swsInputPixelFormat)).
                            arg(pixelFormatName(frameFormat)).
                            arg(_fileName));
                    }
                    // Built here rather than in start() because the format
                    // that arrives is only known now: the hardware download
                    // format, a software-fallback format, or a change
                    // partway through the stream.
                    _initSws(frameFormat);
                }
                if (!_avFrame2)
                {
                    // The stream started in a format that could be copied
                    // straight out, so start() had no scaling to set up for.
                    _initFrame2();
                }
                av_image_fill_arrays(
                    _avFrame2->data,
                    _avFrame2->linesize,
                    data,
                    _avOutputPixelFormat,
                    w,
                    h,
                    1);
                // The matrix and range the image is described with. A YUV
                // output keeps both, so the scaler changes the layout and
                // nothing else: left to its defaults it converted full range
                // pixels to video range that were then drawn as full range,
                // and blacks and whites came out gray.
                frame->color_range = isFullRange(
                    _avCodecContext[_avStream]->color_range,
                    frameFormat) ?
                    AVCOL_RANGE_JPEG :
                    AVCOL_RANGE_MPEG;
                frame->colorspace = fromYUVCoefficients(_info.yuvCoefficients);
                const AVPixFmtDescriptor* outputDesc =
                    av_pix_fmt_desc_get(_avOutputPixelFormat);
                const bool outputRGB =
                    outputDesc && (outputDesc->flags & AV_PIX_FMT_FLAG_RGB);
                _avFrame2->color_range = outputRGB ?
                    AVCOL_RANGE_JPEG :
                    frame->color_range;
                _avFrame2->colorspace = outputRGB ?
                    AVCOL_SPC_RGB :
                    frame->colorspace;
                sws_scale_frame(_swsContext, _avFrame2, frame);

                // The scaler takes YUV to sixteen bit RGB with white at
                // 65280, 255 shifted up, where white is 65535: a ten bit or
                // deeper picture was read 0.39% low when the conversion was
                // done here, which is when it is asked for and whenever the
                // picture has alpha. And it widens alpha by shifting, so
                // opaque in ten bits was 65472. Both are brought up to
                // where they belong; see also the writer, which has the
                // same to undo on the way out.
                const AVPixFmtDescriptor* frameDesc = av_pix_fmt_desc_get(frameFormat);
                if (frameDesc &&
                    !(frameDesc->flags & AV_PIX_FMT_FLAG_RGB) &&
                    frameDesc->nb_components >= 3 &&
                    (AV_PIX_FMT_RGB48 == _avOutputPixelFormat ||
                        AV_PIX_FMT_RGBA64 == _avOutputPixelFormat))
                {
                    const bool alpha = AV_PIX_FMT_RGBA64 == _avOutputPixelFormat;
                    const size_t channels = alpha ? 4 : 3;
                    const int depth = frameDesc->comp[0].depth;
                    const uint32_t alphaMax = depth < 16 ?
                        (((1U << depth) - 1U) << (16 - depth)) :
                        65535U;
                    uint16_t* p = reinterpret_cast<uint16_t*>(data);
                    const size_t count = w * h;
                    for (size_t i = 0; i < count; ++i, p += channels)
                    {
                        for (size_t c = 0; c < 3; ++c)
                        {
                            const uint32_t v = (p[c] * 65535U + 32640U) / 65280U;
                            p[c] = static_cast<uint16_t>(std::min(v, 65535U));
                        }
                        if (alpha && alphaMax < 65535U)
                        {
                            const uint32_t v = (p[3] * 65535U + alphaMax / 2U) / alphaMax;
                            p[3] = static_cast<uint16_t>(std::min(v, 65535U));
                        }
                    }
                }
            }
        }
    }
}

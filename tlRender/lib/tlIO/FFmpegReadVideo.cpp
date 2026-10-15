// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#include <sstream>

#include <tlIO/FFmpegReadPrivate.h>
#include <tlIO/FFmpegMacros.h>

#include <tlCore/Path.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

extern "C"
{
#include <libavutil/display.h>
#include <libavutil/imgutils.h>
#include <libavutil/hwcontext.h>
#include <libavutil/pixdesc.h>
#include <libavutil/opt.h>   
} // extern "C"

namespace
{
    const char* kModule = "ffmpeg";
    
    static constexpr std::array<double, 16> valid_timecode_rates{
        { 1.0,
          12.0,
          23.97,
          23.976,
          23.98,
          24000.0 / 1001.0,
          24.0,
          25.0,
          29.97,
          30000.0 / 1001.0,
          30.0,
          48.0,
          50.0,
          59.94,
          60000.0 / 1001.0,
          60.0 }
    };
    
    bool is_valid_timecode_rate(double fps)
    {
        auto b = valid_timecode_rates.begin(), e = valid_timecode_rates.end();
        return std::find(b, e, fps) != e;
    }
    
}

namespace tl
{
    namespace ffmpeg
    {

        namespace
        {   
            
            void setPrimariesFromAVColorPrimaries(int ffmpegPrimaries,
                                                  image::HDRData& hdrData)
            {
                using V2 = math::Vector2f;
                switch (ffmpegPrimaries)
                {
                case AVCOL_PRI_BT709:         // Also covers sRGB
                    hdrData.primaries = {
                        V2(0.640F, 0.330F),   // Red
                        V2(0.300F, 0.600F),   // Green
                        V2(0.150F, 0.060F),   // Blue
                        V2(0.3127F, 0.3290F)  // White D65
                    };
                    break;
                case AVCOL_PRI_BT470M:
                    hdrData.primaries = {
                        V2(0.670F, 0.330F),
                        V2(0.210F, 0.710F),
                        V2(0.140F, 0.080F),
                        V2(0.310F, 0.316F) // C (approx)
                    };
                    break;
                case AVCOL_PRI_BT470BG:
                case AVCOL_PRI_SMPTE170M:     // NTSC / PAL / SECAM
                    hdrData.primaries = {
                        V2(0.640F, 0.330F),
                        V2(0.290F, 0.600F),
                        V2(0.150F, 0.060F),
                        V2(0.3127F, 0.3290F)
                    };
                    break;
                case AVCOL_PRI_SMPTE240M:
                    hdrData.primaries = {
                        V2(0.630F, 0.340F),
                        V2(0.310F, 0.595F),
                        V2(0.155F, 0.070F),
                        V2(0.3127F, 0.3290F)
                    };
                    break;
                case AVCOL_PRI_FILM:
                    hdrData.primaries = {
                        V2(0.681F, 0.319F),
                        V2(0.243F, 0.692F),
                        V2(0.145F, 0.049F),
                        V2(0.310F, 0.316F) // Illuminant C
                    };
                    break;
                case AVCOL_PRI_BT2020:
                    hdrData.primaries = {
                        V2(0.708F, 0.292F),
                        V2(0.170F, 0.797F),
                        V2(0.131F, 0.046F),
                        V2(0.3127F, 0.3290F)
                    };
                    break;
                case AVCOL_PRI_SMPTE428: // CIE 1931 XYZ — chromaticities are undefined here
                    hdrData.primaries = {
                        V2(1.F, 1.F), // dummy, XYZ assumed linear in CIE space
                        V2(1.F, 1.F),
                        V2(1.F, 1.F),
                        V2(0.333F, 0.333F) // white (ish)
                    };
                    break;
                case AVCOL_PRI_SMPTE431: // DCI-P3 (DCI white)
                    hdrData.primaries = {
                        V2(0.680F, 0.320F),
                        V2(0.265F, 0.690F),
                        V2(0.150F, 0.060F),
                        V2(0.314F, 0.351F) // DCI white
                    };
                    break;
                case AVCOL_PRI_SMPTE432: // Display-P3 (D65 white)
                    hdrData.primaries = {
                        V2(0.680F, 0.320F),
                        V2(0.265F, 0.690F),
                        V2(0.150F, 0.060F),
                        V2(0.3127F, 0.3290F) // D65
                    };
                    break;
                case AVCOL_PRI_EBU3213:
                    hdrData.primaries = {
                        V2(0.630F, 0.340F),
                        V2(0.295F, 0.605F),
                        V2(0.155F, 0.077F),
                        V2(0.3127F, 0.3290F)
                    };
                    break;
                case AVCOL_PRI_UNSPECIFIED:
                case AVCOL_PRI_RESERVED:
                default:
                    // Safe fallback: Rec.709 primaries with D65
                    hdrData.primaries = {
                        V2(0.640F, 0.330F),
                        V2(0.300F, 0.600F),
                        V2(0.150F, 0.060F),
                        V2(0.3127F, 0.3290F)
                    };
                    break;
                }
            }
        
        

            image::EOTFType toEOTF(AVColorTransferCharacteristic trc)
            {
                image::EOTFType out = image::EOTFType::EOTF_BT709;
                switch (trc)
                {
                case AVCOL_TRC_SMPTE2084: // PQ (HDR10)
                    out = image::EOTFType::EOTF_BT2100_PQ;
                    break;
                case AVCOL_TRC_ARIB_STD_B67: // HLG
                    out = image::EOTFType::EOTF_BT2100_HLG;
                    break;
                case AVCOL_TRC_BT2020_10:
                case AVCOL_TRC_BT2020_12:
                    out = image::EOTFType::EOTF_BT2020;
                    break;
                case AVCOL_TRC_BT709:
                case AVCOL_TRC_SMPTE170M:
                case AVCOL_TRC_SMPTE240M:
                case AVCOL_TRC_IEC61966_2_4:
                case AVCOL_TRC_BT1361_ECG:
                    out = image::EOTFType::EOTF_BT709;
                    break;
                default:
                    out = image::EOTFType::EOTF_BT601;
                }
                return out;
            }
        }

        ReadVideo::ReadVideo(
            const std::string& fileName,
            const std::vector<file::MemoryRead>& memory,
            const std::weak_ptr<log::System>& logSystem,
            const Options& options) :
            _fileName(fileName),
            _logSystem(logSystem),
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

            //
            // If we are potentially reading a .webp sequence, add a format
            // specifier to it to read a sequence of frames if available.
            // 
            std::string formatFileName = fileName;
            file::Path path(fileName);
            const std::string& extension = path.getExtension();
            if (string::compare(extension, ".webp",
                                string::Compare::CaseInsensitive) &&
                !path.getNumber().empty())
            {
                char buf[4096];
                const std::string& directory = path.getDirectory();
                const std::string& baseName  = path.getBaseName();
                const std::string& suffix    = path.getSuffix();
                const std::string& extension = path.getExtension();
                const int padding = path.getPadding();
                if (padding == 0)
                {
                    snprintf(buf, 4096, "%s%s%%d%s%s",
                             directory.c_str(),
                             baseName.c_str(),
                             suffix.c_str(),
                             extension.c_str());
                }
                else
                {
                    snprintf(buf, 4096, "%s%s%%0%dd%s%s",
                             directory.c_str(),
                             baseName.c_str(),
                             padding,
                             suffix.c_str(),
                             extension.c_str());
                }
                formatFileName = buf;
            }

            int r = avformat_open_input(
                &_avFormatContext,
                !_avFormatContext ? formatFileName.c_str() : nullptr, nullptr,
                nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(formatFileName)
                                             .arg(getErrorLabel(r)));
            }

            r = avformat_find_stream_info(_avFormatContext, nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(fileName)
                                             .arg(getErrorLabel(r)));
            }

            for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
            {
                if (AVMEDIA_TYPE_VIDEO ==
                        _avFormatContext->streams[i]->codecpar->codec_type &&
                    (AV_DISPOSITION_ATTACHED_PIC &
                         _avFormatContext->streams[i]->disposition ||
                     AV_DISPOSITION_STILL_IMAGE &
                         _avFormatContext->streams[i]->disposition))
                {
                    _useAudioOnly = true;
                }
                if (AVMEDIA_TYPE_VIDEO ==
                        _avFormatContext->streams[i]->codecpar->codec_type &&
                    AV_DISPOSITION_DEFAULT ==
                        _avFormatContext->streams[i]->disposition)
                {
                    _avStream = i;
                    break;
                }
            }
            if (-1 == _avStream)
            {
                if (_useAudioOnly)
                {
                    for (unsigned int i = 0; i < _avFormatContext->nb_streams;
                         ++i)
                    {
                        if (AVMEDIA_TYPE_AUDIO ==
                            _avFormatContext->streams[i]->codecpar->codec_type)
                        {
                            _avAudioStream = i;
                            break;
                        }
                    }
                }
                for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
                {

                    if (AVMEDIA_TYPE_VIDEO ==
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

                auto avVideoStream = _avFormatContext->streams[_avStream];
                auto avVideoCodecParameters = avVideoStream->codecpar;
                auto avVideoCodec =
                    avcodec_find_decoder(avVideoCodecParameters->codec_id);

                AVDictionaryEntry* tag = nullptr;
                unsigned trackNumber = 1; // we only support one video stream
                while (
                    (tag = av_dict_get(
                         avVideoStream->metadata, "", tag,
                         AV_DICT_IGNORE_SUFFIX)))
                {
                    std::string key(string::Format("Video Stream #{0}: {1}")
                                        .arg(trackNumber)
                                        .arg(tag->key));
                    _tags[key] = tag->value;
                }

                // If we are reading VPX, use libvpx-vp9 external lib if
                // available so we can read an alpha channel.
                if (avVideoCodecParameters->codec_id == AV_CODEC_ID_VP9)
                {
                    auto avLibVpxCodec =
                        avcodec_find_decoder_by_name("libvpx-vp9");
                    if (avLibVpxCodec)
                    {
                        avVideoCodec = avLibVpxCodec;
                        avVideoCodecParameters->codec_id = avVideoCodec->id;
                    }
                }

                if (!avVideoCodec)
                {
                    const AVCodecDescriptor *desc =
                        avcodec_descriptor_get(avVideoCodecParameters->codec_id);
                    throw std::runtime_error(
                        string::Format("{0}: No video codec found - {1} {2}")
                        .arg(fileName)
                        .arg(desc->name)
                        .arg(desc->long_name ? desc->long_name : "N/A"));
                }
                _avCodecParameters[_avStream] = avcodec_parameters_alloc();
                if (!_avCodecParameters[_avStream])
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate parameters")
                            .arg(fileName));
                }
                r = avcodec_parameters_copy(
                    _avCodecParameters[_avStream], avVideoCodecParameters);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}")
                                                 .arg(fileName)
                                                 .arg(getErrorLabel(r)));
                }
                _avVideoCodec = avVideoCodec;
                r = _openVideoCodec(avVideoCodec, options.hardwareDecode);
                if (r < 0 && _hardwareDecodeEnabled)
                {
                    LOG_WARNING(string::Format(
                        "VideoToolbox could not open the decoder ({0}); "
                        "using software decoding.").arg(getErrorLabel(r)));
                    avcodec_free_context(&_avCodecContext[_avStream]);
                    r = _openVideoCodec(avVideoCodec, false);
                }
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}")
                                                 .arg(fileName)
                                                 .arg(getErrorLabel(r)));
                }

                _tags["Video Decode Backend"] = _hardwareDecodeEnabled ?
                    "VideoToolbox (pending)" : "Software";
                
                _info.size.w = _avCodecParameters[_avStream]->width;
                _info.size.h = _avCodecParameters[_avStream]->height;

                if (avVideoStream->sample_aspect_ratio.den > 0 &&
                    avVideoStream->sample_aspect_ratio.num > 0)
                {
                    _info.size.pixelAspectRatio =
                        av_q2d(avVideoStream->sample_aspect_ratio);
                }

                _info.layout.mirror.y = true;

                _avInputPixelFormat = static_cast<AVPixelFormat>(
                    _avCodecParameters[_avStream]->format);

                const char* pixel_format = av_get_pix_fmt_name(_avInputPixelFormat);
                if (pixel_format)
                    _tags["FFmpeg Pixel Format"] = pixel_format;
                else
                    _tags["FFmpeg Pixel Format"] = "Unknown";
                    

                // LibVPX returns AV_PIX_FMT_YUV420P with metadata
                // "alpha_mode" set to 1.
                while (
                    (tag = av_dict_get(
                         avVideoStream->metadata, "", tag,
                         AV_DICT_IGNORE_SUFFIX)))
                {
                    const std::string key(tag->key);
                    const std::string value(tag->value);
                    if (string::compare(
                            key, "alpha_mode",
                            string::Compare::CaseInsensitive))
                    {
                        if (value == "1" &&
                            _avInputPixelFormat == AV_PIX_FMT_YUV420P)
                        {
                            _avInputPixelFormat = AV_PIX_FMT_YUVA420P;
                        }
                    }
                }

                switch (_avInputPixelFormat)
                {
                case AV_PIX_FMT_BGR24:
                case AV_PIX_FMT_RGB24:
                    _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                    _info.pixelType = image::PixelType::RGB_U8;
                    break;
                case AV_PIX_FMT_GRAY8:
                    _avOutputPixelFormat = _avInputPixelFormat;
                    _info.pixelType = image::PixelType::L_U8;
                    break;
                case AV_PIX_FMT_BGRA:
                case AV_PIX_FMT_RGBA:
                    _avOutputPixelFormat = AV_PIX_FMT_RGBA;
                    _info.pixelType = image::PixelType::RGBA_U8;
                    break;
                case AV_PIX_FMT_YUVJ420P: // Deprecated format.
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                        _info.pixelType = image::PixelType::RGB_U8;
                    }
                    else
                    {
                        _fastYUV420PConversion = true;
                        _avOutputPixelFormat = _avInputPixelFormat;
                        _info.pixelType = image::PixelType::YUV_420P_U8;
                    }
                    break;
                case AV_PIX_FMT_YUV420P:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                        _info.pixelType = image::PixelType::RGB_U8;
                    }
                    else
                    {
                        _fastYUV420PConversion = options.fastYUV420PConversion;
                        _avOutputPixelFormat = _avInputPixelFormat;
                        _info.pixelType = image::PixelType::YUV_420P_U8;
                    }
                    break;
                case AV_PIX_FMT_YUV422P:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                        _info.pixelType = image::PixelType::RGB_U8;
                    }
                    else
                    {
                        _avOutputPixelFormat = _avInputPixelFormat;
                        _info.pixelType = image::PixelType::YUV_422P_U8;
                    }
                    break;
                case AV_PIX_FMT_YUV444P:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                        _info.pixelType = image::PixelType::RGB_U8;
                    }
                    else
                    {
                        _avOutputPixelFormat = _avInputPixelFormat;
                        _info.pixelType = image::PixelType::YUV_444P_U8;
                    }
                    break;
                case AV_PIX_FMT_YUV420P10BE:
                case AV_PIX_FMT_YUV420P10LE:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        _info.pixelType = image::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the _info.layout.endian field instead of
                        //! converting endianness.
                        _avOutputPixelFormat = AV_PIX_FMT_YUV420P10LE;
                        _info.pixelType = image::PixelType::YUV_420P_U10;
                    }
                    break;
                case AV_PIX_FMT_YUV420P12BE:
                case AV_PIX_FMT_YUV420P12LE:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        _info.pixelType = image::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the _info.layout.endian field instead of
                        //! converting endianness.
                        _avOutputPixelFormat = AV_PIX_FMT_YUV420P12LE;
                        _info.pixelType = image::PixelType::YUV_420P_U12;
                    }
                    break;
                case AV_PIX_FMT_YUV420P16BE:
                case AV_PIX_FMT_YUV420P16LE:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        _info.pixelType = image::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the _info.layout.endian field instead of
                        //! converting endianness.
                        _avOutputPixelFormat = AV_PIX_FMT_YUV420P16LE;
                        _info.pixelType = image::PixelType::YUV_420P_U16;
                    }
                    break;
                case AV_PIX_FMT_YUV422P10BE:
                case AV_PIX_FMT_YUV422P10LE:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        _info.pixelType = image::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the _info.layout.endian field instead of
                        //! converting endianness.
                        _avOutputPixelFormat = AV_PIX_FMT_YUV422P10LE;
                        _info.pixelType = image::PixelType::YUV_422P_U10;
                    }
                    break;
                case AV_PIX_FMT_YUV422P12BE:
                case AV_PIX_FMT_YUV422P12LE:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        _info.pixelType = image::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the _info.layout.endian field instead of
                        //! converting endianness.
                        _avOutputPixelFormat = AV_PIX_FMT_YUV422P12LE;
                        _info.pixelType = image::PixelType::YUV_422P_U12;
                    }
                    break;
                case AV_PIX_FMT_YUV422P16BE:
                case AV_PIX_FMT_YUV422P16LE:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        _info.pixelType = image::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the _info.layout.endian field instead of
                        //! converting endianness.
                        _avOutputPixelFormat = AV_PIX_FMT_YUV422P16LE;
                        _info.pixelType = image::PixelType::YUV_422P_U16;
                    }
                    break;
                case AV_PIX_FMT_YUV444P10BE:
                case AV_PIX_FMT_YUV444P10LE:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        _info.pixelType = image::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the _info.layout.endian field instead of
                        //! converting endianness.
                        _avOutputPixelFormat = AV_PIX_FMT_YUV444P10LE;
                        _info.pixelType = image::PixelType::YUV_444P_U10;
                    }
                    break;
                case AV_PIX_FMT_YUV444P12BE:
                case AV_PIX_FMT_YUV444P12LE:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        _info.pixelType = image::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the _info.layout.endian field instead of
                        //! converting endianness.
                        _avOutputPixelFormat = AV_PIX_FMT_YUV444P12LE;
                        _info.pixelType = image::PixelType::YUV_444P_U12;
                    }
                    break;
                case AV_PIX_FMT_YUV444P16BE:
                case AV_PIX_FMT_YUV444P16LE:
                    if (options.yuvToRGBConversion)
                    {
                        _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        _info.pixelType = image::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the _info.layout.endian field instead of
                        //! converting endianness.
                        _avOutputPixelFormat = AV_PIX_FMT_YUV444P16LE;
                        _info.pixelType = image::PixelType::YUV_444P_U16;
                    }
                    break;
                case AV_PIX_FMT_GBR24P:
                    _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                    _info.pixelType = image::PixelType::RGB_U8;
                    break;
                case AV_PIX_FMT_GBRP9BE:
                case AV_PIX_FMT_GBRP9LE:
                case AV_PIX_FMT_GBRP10BE:
                case AV_PIX_FMT_GBRP12LE:
                case AV_PIX_FMT_GBRP12BE:
                case AV_PIX_FMT_GBRP10LE:
                case AV_PIX_FMT_GBRP16BE:
                case AV_PIX_FMT_GBRP16LE:
                    _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                    _info.pixelType = image::PixelType::RGB_U16;
                    break;
                case AV_PIX_FMT_YUVA420P:
                case AV_PIX_FMT_YUVA422P:
                case AV_PIX_FMT_YUVA444P:
                    _avOutputPixelFormat = AV_PIX_FMT_RGBA;
                    _info.pixelType = image::PixelType::RGBA_U8;
                    break;
                case AV_PIX_FMT_GBRAP10BE:
                case AV_PIX_FMT_GBRAP12LE:
                case AV_PIX_FMT_GBRAP12BE:
                case AV_PIX_FMT_GBRAP10LE:
                case AV_PIX_FMT_GBRAP16BE:
                case AV_PIX_FMT_GBRAP16LE:
                    _avOutputPixelFormat = AV_PIX_FMT_RGBA64;
                    _info.pixelType = image::PixelType::RGBA_U16;
                    break;
                case AV_PIX_FMT_YUVA444P10BE:
                case AV_PIX_FMT_YUVA444P10LE:
                case AV_PIX_FMT_YUVA444P12BE:
                case AV_PIX_FMT_YUVA444P12LE:
                case AV_PIX_FMT_YUVA444P16BE:
                case AV_PIX_FMT_YUVA444P16LE:
                    _avOutputPixelFormat = AV_PIX_FMT_RGBA64;
                    _info.pixelType = image::PixelType::RGBA_U16;
                    break;
                default:
                    _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                    _info.pixelType = image::PixelType::RGB_U8;
                    break;
                }
                const auto params = _avCodecParameters[_avStream];
                if (params->color_range != AVCOL_RANGE_JPEG)
                {
                    _info.videoLevels = image::VideoLevels::LegalRange;
                }
                switch (params->color_space)
                {
                case AVCOL_SPC_BT2020_NCL:
                case AVCOL_SPC_BT2020_CL:
                    _info.yuvCoefficients = image::YUVCoefficients::BT2020;
                    break;
                    // Standard Dynamic Range (SDR)
                case AVCOL_SPC_BT709:
                    _info.yuvCoefficients = image::YUVCoefficients::REC709;
                    break;

                    // Standard Definition (Optional, depending on your needs)
                case AVCOL_SPC_SMPTE170M:
                case AVCOL_SPC_BT470BG:
                    _info.yuvCoefficients = image::YUVCoefficients::BT601;
                    break;

                default:
                    // A safe default is usually Rec.709, though strictly speaking 
                        // you might want to guess based on video resolution.
                        _info.yuvCoefficients = image::YUVCoefficients::REC709; 
                    break;
                }

                image::Tags tags;
                std::size_t sequenceSize = 0;
                double speed = 24;

                if (_useAudioOnly)
                {
                    auto avAudioStream =
                        _avFormatContext->streams[_avAudioStream];
                    auto avAudioCodecParameters = avAudioStream->codecpar;
                    auto avAudioCodec =
                        avcodec_find_decoder(avAudioCodecParameters->codec_id);
                    if (!avAudioCodec)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: No audio codec found")
                                .arg(fileName));
                    }

                    const size_t sampleRate =
                        avAudioCodecParameters->sample_rate;
                    int64_t sampleCount = 0;
                    if (avAudioStream->duration != AV_NOPTS_VALUE)
                    {
                        AVRational r;
                        r.num = 1;
                        r.den = sampleRate;
                        sampleCount = av_rescale_q(
                            avAudioStream->duration, avAudioStream->time_base,
                            r);
                    }
                    else if (_avFormatContext->duration != AV_NOPTS_VALUE)
                    {
                        AVRational r;
                        r.num = 1;
                        r.den = sampleRate;
                        sampleCount = av_rescale_q(
                            _avFormatContext->duration, av_get_time_base_q(),
                            r);
                    }

                    otime::RationalTime timeReference = time::invalidTime;
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
                                key, "time_reference",
                                string::Compare::CaseInsensitive))
                        {
                            timeReference = otime::RationalTime(
                                std::atoi(value.c_str()), sampleRate);
                        }
                    }

                    otime::RationalTime startTime(0.0, sampleRate);
                    if (!timeReference.is_invalid_time())
                    {
                        startTime = timeReference;
                    }
                    _timeRange = otime::TimeRange(
                        startTime.rescaled_to(60.0),
                        otime::RationalTime(sampleCount, sampleRate)
                            .rescaled_to(60.0));
                }
                else
                {
                    _avSpeed = avVideoStream->r_frame_rate;
                    // Use avg_frame_rate if set
                    if (avVideoStream->avg_frame_rate.num != 0 &&
                        avVideoStream->avg_frame_rate.den != 0)
                        _avSpeed = avVideoStream->avg_frame_rate;

                    if (_avSpeed.num == 1000)
                    {
                        LOG_WARNING("Movie has variable frame rate.  "
                                    "This is not supported.");
                        _avSpeed.num = 12;
                    }

                    speed = av_q2d(_avSpeed);

                    // Some movies can return a rounding error in speed
                    // calculation
                    if (!is_valid_timecode_rate(speed))
                    {
                        speed = round(speed * 1000.F) / 1000.F;
                    }

                    if (avVideoStream->nb_frames > 0)
                    {
                        sequenceSize = avVideoStream->nb_frames;
                    }
                    else if (avVideoStream->duration != AV_NOPTS_VALUE)
                    {
                        sequenceSize = av_rescale_q(
                            avVideoStream->duration, avVideoStream->time_base,
                            swap(_avSpeed));
                    }
                    else if (_avFormatContext->duration != AV_NOPTS_VALUE)
                    {
                        sequenceSize = av_rescale_q(
                            _avFormatContext->duration, av_get_time_base_q(),
                            swap(_avSpeed));
                    }
                    else
                    {
                        // If all fails, assume a single frame
                        sequenceSize = 1;
                    }

                    while (
                        (tag = av_dict_get(
                             _avFormatContext->metadata, "", tag,
                             AV_DICT_IGNORE_SUFFIX)))
                    {
                        const std::string key(tag->key);
                        const std::string value(tag->value);
                        tags[key] = value;
                        if (string::compare(
                                key, "timecode",
                                string::Compare::CaseInsensitive))
                        {
                            timecode = value;
                        }
                    }

                    otime::RationalTime startTime(0.0, speed);
                    if (!timecode.empty())
                    {
                        otime::ErrorStatus errorStatus;
                        const otime::RationalTime time =
                            otime::RationalTime::from_timecode(
                                timecode, speed, &errorStatus);
                        if (!otime::is_error(errorStatus))
                        {
                            startTime = time.floor();
                        }
                    }
                    _timeRange = otime::TimeRange(
                        startTime, otime::RationalTime(sequenceSize, speed));
                }

                for (const auto& i : tags)
                {
                    _tags[i.first] = i.second;
                }
                _rotation = _getRotation(avVideoStream);
                {
                    std::stringstream ss;
                    ss << std::fixed;
                    ss << _rotation;
                    _tags["Video Rotation"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << _info.size.w << " " << _info.size.h;
                    _tags["Video Resolution"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << _info.size.pixelAspectRatio;
                    _tags["Video Pixel Aspect Ratio"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << _info.pixelType;
                    _tags["Video Pixel Type"] = ss.str();
                }
                {
                    _tags["Video Codec"] =
                        avcodec_get_name(_avCodecContext[_avStream]->codec_id);
                }
                {
                    _tags["Video Color Primaries"] =
                        av_color_primaries_name(params->color_primaries);
                    
                }
                {
                    _avColorTRC = params->color_trc;
                    _tags["Video Color TRC"] =
                        av_color_transfer_name(params->color_trc);
                }
                {
                    _tags["Video Color Space"] =
                        av_color_space_name(params->color_space);
                }
                {
                    std::stringstream ss;
                    ss << _info.videoLevels;
                    _tags["Video Levels"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << _timeRange.start_time().to_timecode();
                    _tags["Video Start Time"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << _timeRange.duration().to_timecode();
                    _tags["Video Duration"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << _timeRange.start_time().rate() << " FPS";
                    _tags["Video Speed"] = ss.str();
                }

                toHDRData(avVideoStream, _hdr);
                _hdr.eotf = toEOTF(_avColorTRC);
                setPrimariesFromAVColorPrimaries(params->color_primaries,
                                                 _hdr);
            }
        }

        int ReadVideo::_openVideoCodec(const AVCodec* codec, bool hardware)
        {
            _hardwareDecodeEnabled = false;
            auto& context = _avCodecContext[_avStream];
            context = avcodec_alloc_context3(codec);
            if (!context)
                return AVERROR(ENOMEM);
            int r = avcodec_parameters_to_context(
                context, _avCodecParameters[_avStream]);
            if (r < 0)
                return r;

            context->thread_count = _options.threadCount;
            context->thread_type = FF_THREAD_FRAME;
            if (!_options.threadCount && codec->id == AV_CODEC_ID_AV1)
                context->thread_count = 4;

#if defined(__APPLE__)
            const auto sourceFormat = av_pix_fmt_desc_get(
                static_cast<AVPixelFormat>(_avCodecParameters[_avStream]->format));
            if (hardware && codec->id == AV_CODEC_ID_PRORES && sourceFormat &&
                (sourceFormat->flags & AV_PIX_FMT_FLAG_ALPHA))
            {
                // VideoToolbox's packed ProRes 4444 alpha surfaces can alter
                // chroma samples. Preserve the existing color/alpha behavior.
                hardware = false;
                LOG_STATUS("Using software decoding for ProRes with alpha "
                           "to preserve color and transparency.");
            }
            if (hardware && (codec->id == AV_CODEC_ID_PRORES ||
                             codec->id == AV_CODEC_ID_H264 ||
                             codec->id == AV_CODEC_ID_HEVC))
            {
                bool supported = false;
                for (int i = 0; const auto config = avcodec_get_hw_config(codec, i); ++i)
                {
                    if (config->device_type == AV_HWDEVICE_TYPE_VIDEOTOOLBOX &&
                        config->pix_fmt == AV_PIX_FMT_VIDEOTOOLBOX &&
                        (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX))
                    {
                        supported = true;
                        break;
                    }
                }
                if (supported)
                {
                    r = av_hwdevice_ctx_create(
                        &context->hw_device_ctx, AV_HWDEVICE_TYPE_VIDEOTOOLBOX,
                        nullptr, nullptr, 0);
                    if (r >= 0)
                    {
                        context->opaque = this;
                        context->get_format = _getDecodeFormat;
                        _hardwareDecodeEnabled = true;
                    }
                    else
                    {
                        LOG_WARNING(string::Format(
                            "VideoToolbox device unavailable ({0}); "
                            "using software decoding.").arg(getErrorLabel(r)));
                    }
                }
            }
#else
            (void)hardware;
#endif
            return avcodec_open2(context, codec, nullptr);
        }

        AVPixelFormat ReadVideo::_getDecodeFormat(
            AVCodecContext* context, const AVPixelFormat* formats)
        {
            auto self = static_cast<ReadVideo*>(context->opaque);
            if (self->_hardwareDecodeEnabled)
            {
                for (auto p = formats; *p != AV_PIX_FMT_NONE; ++p)
                {
                    if (*p == AV_PIX_FMT_VIDEOTOOLBOX)
                        return *p;
                }
            }
            // FFmpeg calls get_format again without the hardware format if
            // session creation fails. Choose an actual software format.
            for (auto p = formats; *p != AV_PIX_FMT_NONE; ++p)
            {
                const auto desc = av_pix_fmt_desc_get(*p);
                if (desc && !(desc->flags & AV_PIX_FMT_FLAG_HWACCEL))
                    return *p;
            }
            return AV_PIX_FMT_NONE;
        }

        bool ReadVideo::_fallbackToSoftware(
            int error, const otime::RationalTime& targetTime)
        {
            if (!_hardwareDecodeEnabled)
                return false;
            LOG_WARNING(string::Format(
                "VideoToolbox decoding failed ({0}); retrying with software decoding.")
                .arg(getErrorLabel(error)));
            avcodec_free_context(&_avCodecContext[_avStream]);
            const int r = _openVideoCodec(_avVideoCodec, false);
            if (r < 0)
                throw std::runtime_error(getErrorLabel(r));
            av_frame_unref(_avFrame);
            // Re-read from a keyframe; a new software context cannot decode
            // an arbitrary inter-frame packet from the hardware context.
            seek(targetTime);
            return true;
        }

        ReadVideo::~ReadVideo()
        {
            if (_swsContext)
            {
                sws_freeContext(_swsContext);
            }
            if (_avFrame)
            {
                av_frame_free(&_avFrame);
            }
            if (_avFrame2)
            {
                av_frame_free(&_avFrame2);
            }
            for (auto i : _avCodecContext)
            {
                avcodec_free_context(&i.second);
            }
            for (auto i : _avCodecParameters)
            {
                avcodec_parameters_free(&i.second);
            }
            if (_avIOContext)
            {
                avio_context_free(&_avIOContext);
            }
            //! \bug Free'd by avio_context_free()?
            // if (_avIOContextBuffer)
            //{
            //     av_free(_avIOContextBuffer);
            // }
            if (_avFormatContext)
            {
                avformat_close_input(&_avFormatContext);
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

        const otime::TimeRange& ReadVideo::getTimeRange() const
        {
            return _timeRange;
        }

        const image::Tags& ReadVideo::getTags() const
        {
            return _tags;
        }

        namespace
        {
            bool canCopy(
                AVPixelFormat in, AVPixelFormat out, bool fastYUV420PConversion)
            {
                return in == out &&
                       (AV_PIX_FMT_RGB24 == in || AV_PIX_FMT_GRAY8 == in ||
                        AV_PIX_FMT_RGBA == in ||
                        ((AV_PIX_FMT_YUV420P == in ||
                          AV_PIX_FMT_YUVJ420P == in) &&
                          fastYUV420PConversion) ||
                        AV_PIX_FMT_YUV422P == in ||
                        AV_PIX_FMT_YUV444P == in ||
                        AV_PIX_FMT_YUV420P10LE == in ||
                        AV_PIX_FMT_YUV422P10LE == in ||
                        AV_PIX_FMT_YUV444P10LE == in ||
                        AV_PIX_FMT_YUV420P12LE == in ||
                        AV_PIX_FMT_YUV422P12LE == in ||
                        AV_PIX_FMT_YUV444P12LE == in ||
                        AV_PIX_FMT_YUV420P16LE == in ||
                        AV_PIX_FMT_YUV422P16LE == in ||
                        AV_PIX_FMT_YUV444P16LE == in );
            }
        } // namespace

        void ReadVideo::start()
        {
            if (_avStream != -1)
            {
                _avFrame = av_frame_alloc();
                _avFrame2 = av_frame_alloc();
                if (!_avFrame || !_avFrame2)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate frame").arg(_fileName));
                }
                // Configure conversion lazily from the decoded frame. Hardware
                // downloads may be NV12/P010/packed ProRes instead of the
                // planar pixel format reported in the stream parameters.
            }
        }

        void ReadVideo::_createSwsContext(const AVFrame* frame)
        {
            if (_swsContext)
                sws_freeContext(_swsContext);
            _swsContext = nullptr;
            const int width = frame->width;
            const int height = frame->height;
            const auto params = _avCodecParameters[_avStream];
            const auto colorSpace = frame->colorspace == AVCOL_SPC_UNSPECIFIED ?
                params->color_space : frame->colorspace;
            const auto colorRange = frame->color_range == AVCOL_RANGE_UNSPECIFIED ?
                params->color_range : frame->color_range;
            int r;
            r = sws_isSupportedInput(_avInputPixelFormat);
            if (r == 0)
            {
                throw std::runtime_error(
                    string::Format("{0}: Unsuported pixel input format")
                        .arg(_fileName));
            }
            r = sws_isSupportedOutput(_avOutputPixelFormat);
            if (r == 0)
            {
                throw std::runtime_error(
                    string::Format(
                        "{0}: Unsuported pixel output format")
                        .arg(_fileName));
            }
            _swsContext = sws_alloc_context();
            if (!_swsContext)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot allocate context")
                        .arg(_fileName));
            }
            av_opt_set_defaults(_swsContext);
            r = av_opt_set_int(
                _swsContext, "srcw", width, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(
                _swsContext, "srch", height, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(
                _swsContext, "src_format", _avInputPixelFormat,
                AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(
                _swsContext, "dstw", width, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(
                _swsContext, "dsth", height, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(
                _swsContext, "dst_format", _avOutputPixelFormat,
                AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(
                _swsContext, "sws_flags", swsScaleFlags,
                AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(
                _swsContext, "threads", 0, AV_OPT_SEARCH_CHILDREN);
            r = sws_init_context(_swsContext, nullptr, nullptr);
            if (r < 0)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot initialize sws context")
                        .arg(_fileName));
            }

            // \@bug:
            //    We don't do a BT2020_NCL to BT709 conversion in
            //    software which is slow.
            if (colorSpace != AVCOL_SPC_BT2020_NCL &&
                (colorSpace != AVCOL_SPC_UNSPECIFIED ||
                 width < 4096 || height < 2160))
            {
                int in_full = -1;
                int out_full = -1;
                int brightness = -1;
                int contrast = -1;
                int saturation = -1;
                int *inv_table = nullptr, *table = nullptr;

                sws_getColorspaceDetails(
                    _swsContext, &inv_table, &in_full, &table,
                    &out_full, &brightness, &contrast, &saturation);

                // \@note: sws_getCoefficients uses its own enum,
                //         which mostly matches AV_COL_SPC_* values,
                //         but we still do a special check here just in
                //         case.
                int in_color_space = SWS_CS_DEFAULT;
                switch (colorSpace)
                {
                case AVCOL_SPC_RGB:
                    in_color_space = SWS_CS_ITU601;
                    break;
                case AVCOL_SPC_BT709:
                    in_color_space = SWS_CS_ITU709;
                    break;
                case AVCOL_SPC_FCC:
                    in_color_space = SWS_CS_FCC;
                    break;
                    // case AVCOL_SPC_ITU624 (is not defined)
                    // can be NTSC or PAL in_color_space = SWS_CS_624;
                    // break;
                case AVCOL_SPC_SMPTE170M:
                    in_color_space = SWS_CS_SMPTE170M;
                    break;
                case AVCOL_SPC_SMPTE240M:
                    in_color_space = SWS_CS_SMPTE240M;
                    break;
                case AVCOL_SPC_BT2020_NCL:
                case AVCOL_SPC_BT2020_CL: // \@bug: this one is wrong
                    in_color_space = SWS_CS_BT2020;
                    break;
                default:
                    break;
                }

                in_full = (colorRange == AVCOL_RANGE_JPEG);
                out_full = (colorRange == AVCOL_RANGE_JPEG);

                int out_color_space = SWS_CS_ITU709;

                sws_setColorspaceDetails(
                    _swsContext, sws_getCoefficients(in_color_space),
                    in_full, sws_getCoefficients(out_color_space),
                    out_full, brightness, contrast, saturation);
            }

            _swsWidth = width;
            _swsHeight = height;
            _swsColorSpace = frame->colorspace;
            _swsColorRange = frame->color_range;
        }

        void ReadVideo::seek(const otime::RationalTime& time)
        {
            if (_avStream != -1 && !_useAudioOnly)
            {
                avcodec_flush_buffers(_avCodecContext[_avStream]);

                if (av_seek_frame(
                        _avFormatContext, _avStream,
                        av_rescale_q(
                            time.value() - _timeRange.start_time().value(),
                            swap(_avSpeed),
                            _avFormatContext->streams[_avStream]->time_base),
                        AVSEEK_FLAG_BACKWARD) < 0)
                {
                    //! \todo How should this be handled?
                }
            }

            _buffer.clear();
            _eof = false;
        }

        bool ReadVideo::process(
            const bool backwards, const otime::RationalTime& targetTime,
            otime::RationalTime& currentTime)
        {
            bool out = false;
            if (_avStream != -1 && _buffer.size() < _options.videoBufferSize)
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
                            std::string msg = string::Format("av_read_frame failed with {0}")
                                              .arg(decoding);
                            LOG_ERROR(msg);
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
                            if (decoding != AVERROR(EAGAIN) &&
                                _fallbackToSoftware(decoding, targetTime))
                            {
                                av_packet_unref(packet.p);
                                decoding = 0;
                                continue;
                            }
                            // \@bug: We failed decoding this frame.
                            // This will make FFmpeg try to use the next available frame.
                            seek(targetTime);
                            decoding = 0;
                        }
                        decoding = _decode(backwards, targetTime, currentTime);
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
                            if (_fallbackToSoftware(decoding, targetTime))
                            {
                                av_packet_unref(packet.p);
                                decoding = 0;
                                continue;
                            }
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
                // std::cout << "video buffer size: " << _buffer.size() <<
                // std::endl;
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

        int ReadVideo::_decode(
            const bool backwards, const otime::RationalTime& targetTime,
            otime::RationalTime& currentTime)
        {
            int out = 0;
            if (_singleImage && _singleImage->isValid())
            {
                currentTime = targetTime;
                _buffer.push_back(_singleImage);
                out = 1;
                return out;
            }

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
                // std::cout << "video timestamp: " << timestamp << std::endl;
                const auto& avVideoStream =
                    _avFormatContext->streams[_avStream];
                
                const otime::RationalTime time(
                    _timeRange.start_time().value() +
                        av_rescale_q(
                            timestamp, avVideoStream->time_base,
                            swap(avVideoStream->r_frame_rate)),
                    _timeRange.duration().rate());

                if (time >= targetTime || backwards ||
                    (_avFrame->duration == 0 && _useAudioOnly))
                {
                    // Use actual time of frame when possible, except when
                    // going backwards.
                    if (backwards && time >= targetTime)
                        currentTime = targetTime;
                    else
                        currentTime = time;
                    
                    std::shared_ptr<image::Image> image;

                    const bool hardware =
                        _avFrame->format == AV_PIX_FMT_VIDEOTOOLBOX;
                    std::shared_ptr<AVFrame> safeFrame(
                        av_frame_alloc(), [](AVFrame* f) { av_frame_free(&f); });
                    if (!safeFrame)
                        return AVERROR(ENOMEM);
                    if (hardware)
                    {
                        int r = av_hwframe_transfer_data(
                            safeFrame.get(), _avFrame, 0);
                        if (r < 0)
                            return r;
                        r = av_frame_copy_props(safeFrame.get(), _avFrame);
                        if (r < 0)
                            return r;

                        // Never silently lose alpha, precision or chroma
                        // resolution when the platform chooses a surface.
                        const auto source = av_pix_fmt_desc_get(
                            static_cast<AVPixelFormat>(
                                _avCodecParameters[_avStream]->format));
                        const auto downloaded = av_pix_fmt_desc_get(
                            static_cast<AVPixelFormat>(safeFrame->format));
                        if (!downloaded ||
                            !sws_isSupportedInput(
                                static_cast<AVPixelFormat>(safeFrame->format)) ||
                            (source &&
                             (((source->flags & AV_PIX_FMT_FLAG_ALPHA) &&
                               !(downloaded->flags & AV_PIX_FMT_FLAG_ALPHA)) ||
                              downloaded->comp[0].depth < source->comp[0].depth ||
                              downloaded->log2_chroma_w > source->log2_chroma_w ||
                              downloaded->log2_chroma_h > source->log2_chroma_h)))
                            return AVERROR(ENOSYS);
                    }
                    else
                    {
                        const int r = av_frame_ref(safeFrame.get(), _avFrame);
                        if (r < 0)
                            return r;
                    }
                    const std::string backend = hardware ? "VideoToolbox" : "Software";
                    if (_tags["Video Decode Backend"] != backend)
                    {
                        _tags["Video Decode Backend"] = backend;
                        LOG_STATUS(string::Format("{0}: {1} decoding ({2}).")
                            .arg(_fileName).arg(backend)
                            .arg(avcodec_get_name(_avVideoCodec->id)));
                    }
                    const char* pixelFormat = av_get_pix_fmt_name(
                        static_cast<AVPixelFormat>(safeFrame->format));
                    _tags["FFmpeg Pixel Format"] = pixelFormat ? pixelFormat : "Unknown";
                    auto tags = _tags;

                    tags["otioClipName"] = _fileName;
                    {
                        std::stringstream ss;
                        ss << time;
                        tags["otioClipTime"] = ss.str();
                    }
                    
                    _copy(image, safeFrame);

                    AVDictionaryEntry* tag = nullptr;
                    while (
                        (tag = av_dict_get(
                             avVideoStream->metadata, "", tag,
                             AV_DICT_IGNORE_SUFFIX)))
                    {
                        std::string key(string::Format("Video Stream #{0}: {1}")
                                            .arg(_avStream)
                                            .arg(tag->key));
                        tags[key] = tag->value;
                    }
                    while (
                        (tag = av_dict_get(
                            _avFrame->metadata, "", tag,
                            AV_DICT_IGNORE_SUFFIX)))
                    {
                        tags[tag->key] = tag->value;
                    }
                    
                    _hdr.eotf = toEOTF(_avFrame->color_trc);
                    setPrimariesFromAVColorPrimaries(_avFrame->color_primaries,
                                                     _hdr);
                    if (image::isHDR(_hdr))
                    {
                        toHDRData(_avFrame, _hdr);
                        image->setHDR(_hdr);
                    }
                    image->setTags(tags);
                    
                    _buffer.push_back(image);
                    out = 1;

                    if (_useAudioOnly && _avFrame->duration == 0)
                    {
                        _singleImage = image;
                        currentTime = targetTime;
                    }
                    break;
                }
                
                // Prepare for next decode
                av_frame_unref(_avFrame);
            }
            return out;
        }

        void ReadVideo::_copy(std::shared_ptr<image::Image>& image,
                              std::shared_ptr<AVFrame> avFrame)
        {
            // Check if avFrame changed image size (can happen when reading a
            // sequence of .webp) 
            if (avFrame->width != _info.size.w &&
                avFrame->width > 0)
                _info.size.w = avFrame->width;
            
            if (avFrame->height != _info.size.h &&
                avFrame->height > 0)
                _info.size.h = avFrame->height;
            
            
            const std::size_t w = _info.size.w;
            const std::size_t h = _info.size.h;

            const auto pixelFormat = static_cast<AVPixelFormat>(avFrame->format);
            if (!canCopy(pixelFormat, _avOutputPixelFormat, _fastYUV420PConversion) &&
                (!_swsContext || pixelFormat != _avInputPixelFormat ||
                 _swsWidth != avFrame->width || _swsHeight != avFrame->height ||
                 _swsColorSpace != avFrame->colorspace ||
                 _swsColorRange != avFrame->color_range))
            {
                _avInputPixelFormat = pixelFormat;
                _createSwsContext(avFrame.get());
            }
            _avInputPixelFormat = pixelFormat;

            uint8_t* data;
            if (canCopy(
                    _avInputPixelFormat, _avOutputPixelFormat,
                    _fastYUV420PConversion))
            {
                const uint8_t* const data0 = avFrame->data[0];
                const int linesize0 = avFrame->linesize[0];
                switch (_avInputPixelFormat)
                {
                case AV_PIX_FMT_RGB24:
                    image = image::Image::create(_info);
                    data = image->getData();
                    for (std::size_t i = 0; i < h; ++i)
                    {
                        std::memcpy(
                            data + w * 3 * i, data0 + linesize0 * i, w * 3);
                    }
                    break;
                case AV_PIX_FMT_GRAY8:
                    image = image::Image::create(_info);
                    data = image->getData();
                    for (std::size_t i = 0; i < h; ++i)
                    {
                        std::memcpy(data + w * i, data0 + linesize0 * i, w);
                    }
                    break;
                case AV_PIX_FMT_RGBA:
                    image = image::Image::create(_info);
                    data = image->getData();
                    for (std::size_t i = 0; i < h; ++i)
                    {
                        std::memcpy(
                            data + w * 4 * i, data0 + linesize0 * i, w * 4);
                    }
                    break;
                case AV_PIX_FMT_YUVJ420P:
                case AV_PIX_FMT_YUV420P:
                case AV_PIX_FMT_YUV422P:
                case AV_PIX_FMT_YUV444P:
                case AV_PIX_FMT_YUV420P10LE:
                case AV_PIX_FMT_YUV422P10LE:
                case AV_PIX_FMT_YUV444P10LE:
                case AV_PIX_FMT_YUV420P12LE:
                case AV_PIX_FMT_YUV422P12LE:
                case AV_PIX_FMT_YUV444P12LE:
                case AV_PIX_FMT_YUV420P16LE:
                case AV_PIX_FMT_YUV422P16LE:
                case AV_PIX_FMT_YUV444P16LE:
                {
                    const uint8_t* planes[3] = {
                        avFrame->data[0],
                        avFrame->data[1],
                        avFrame->data[2]
                    };
                    int linesize[3] = {
                        avFrame->linesize[0],
                        avFrame->linesize[1],
                        avFrame->linesize[2]
                    };
                    image = image::Image::create(_info, avFrame,
                                                 planes, linesize);
                    break;
                }
                default:
                    break;
                }
            }
            else
            {
                const auto desc = av_pix_fmt_desc_get(_avOutputPixelFormat);
                const bool planar = desc && desc->nb_components == 3 &&
                    (desc->flags & AV_PIX_FMT_FLAG_PLANAR) &&
                    !(desc->flags & AV_PIX_FMT_FLAG_RGB);
                std::shared_ptr<AVFrame> converted;
                AVFrame* destination = _avFrame2;
                if (planar)
                {
                    converted = std::shared_ptr<AVFrame>(
                        av_frame_alloc(), [](AVFrame* f) { av_frame_free(&f); });
                    if (!converted)
                        throw std::runtime_error(getErrorLabel(AVERROR(ENOMEM)));
                    converted->format = _avOutputPixelFormat;
                    converted->width = w;
                    converted->height = h;
                    const int r = av_frame_get_buffer(converted.get(), 32);
                    if (r < 0)
                        throw std::runtime_error(getErrorLabel(r));
                    destination = converted.get();
                }
                else
                {
                    image = image::Image::create(_info);
                    data = image->getData();
                    av_image_fill_arrays(
                        destination->data, destination->linesize, data,
                        _avOutputPixelFormat, w, h, 1);
                }

                const int r = sws_scale(
                    _swsContext, (uint8_t const* const*)avFrame->data,
                    avFrame->linesize, 0,
                    avFrame->height, destination->data,
                    destination->linesize);
                if (r < 0)
                    throw std::runtime_error(getErrorLabel(r));
                if (planar)
                {
                    // The renderer consumes three independent planes and
                    // their strides, including for downloaded hardware frames.
                    const uint8_t* planes[3] = {
                        destination->data[0], destination->data[1], destination->data[2]};
                    image = image::Image::create(
                        _info, converted, planes, destination->linesize);
                }
            }
        }

        float ReadVideo::_getRotation(const AVStream* st)
        {
            float out = 0.F;

            const AVPacketSideData* psd = (const AVPacketSideData*)
                get_stream_side_data(st, AV_PKT_DATA_DISPLAYMATRIX);

            if (psd)
            {
                out = av_display_rotation_get(reinterpret_cast<const int32_t*>(psd));
            }
            return out;
        }

    } // namespace ffmpeg
} // namespace tl

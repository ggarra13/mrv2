// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpeg.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>
#include <tlCore/LogSystem.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

extern "C"
{
#include <libavutil/channel_layout.h>
#include <libavutil/dict.h>
#include <libavutil/dovi_meta.h>
#include <libavutil/hdr_dynamic_metadata.h>
#include <libavutil/imgutils.h>
#include <libavutil/mastering_display_metadata.h>
}

#include <array>

namespace tl
{
    namespace ffmpeg
    {
        TLRENDER_ENUM_IMPL(
            Profile, "None", "H264", "ProRes", "ProRes_Proxy", "ProRes_LT",
            "ProRes_HQ", "ProRes_4444", "ProRes_XQ", "DNxHD", "DNxHR_LB",
            "DNxHR_SQ", "DNxHR_HQ", "DNxHR_HQX", "DNxHR_444", "VP9", "Cineform",
            "AV1", "HAP");
        TLRENDER_ENUM_SERIALIZE_IMPL(Profile);

        TLRENDER_ENUM_IMPL(
            AudioCodec, "None", "AAC", "AC3", "True_HD", "MP2", "MP3", "OPUS",
            "VORBIS", "PCM_S16LE");
        TLRENDER_ENUM_SERIALIZE_IMPL(AudioCodec);

        AVRational swap(AVRational value)
        {
            return AVRational({value.den, value.num});
        }

        bool
        toHDRData(AVPacketSideData* sideData, int size, image::HDRData& hdr)
        {
            bool out = false;
            for (int i = 0; i < size; ++i)
            {
                switch (sideData[i].type)
                {
                case AV_PKT_DATA_MASTERING_DISPLAY_METADATA:
                {
                    out = true;
                    auto data = reinterpret_cast<AVMasteringDisplayMetadata*>(
                        sideData[i].data);
                    if (data->has_luminance)
                    {
                        float max_luma = av_q2d(data->max_luminance);
                        float min_luma = av_q2d(data->min_luminance);
                        if (max_luma < 5.0F || min_luma >= max_luma)
                            max_luma = min_luma = 0.F;

                        hdr.displayMasteringLuminance =
                            math::FloatRange(min_luma, max_luma);
                    }
                    if (data->has_primaries)
                    {
                        hdr.primaries[image::HDRPrimaries::Red].x =
                            av_q2d(data->display_primaries[0][0]);
                        hdr.primaries[image::HDRPrimaries::Red].y =
                            av_q2d(data->display_primaries[0][1]);

                        hdr.primaries[image::HDRPrimaries::Green].x =
                            av_q2d(data->display_primaries[1][0]);
                        hdr.primaries[image::HDRPrimaries::Green].y =
                            av_q2d(data->display_primaries[1][1]);

                        hdr.primaries[image::HDRPrimaries::Blue].x =
                            av_q2d(data->display_primaries[2][0]);
                        hdr.primaries[image::HDRPrimaries::Blue].y =
                            av_q2d(data->display_primaries[2][1]);

                        hdr.primaries[image::HDRPrimaries::White].x =
                            av_q2d(data->white_point[0]);
                        hdr.primaries[image::HDRPrimaries::White].y =
                            av_q2d(data->white_point[1]);
                    }
                    break;
                }
                case AV_PKT_DATA_CONTENT_LIGHT_LEVEL:
                {
                    out = true;
                    auto data = reinterpret_cast<AVContentLightMetadata*>(
                        sideData[i].data);
                    hdr.maxCLL = data->MaxCLL;
                    hdr.maxFALL = data->MaxFALL;
                    break;
                }
                case AV_PKT_DATA_DYNAMIC_HDR10_PLUS:
                {
                    out = true;
                    auto data =
                        reinterpret_cast<AVDynamicHDRPlus*>(sideData[i].data);
                    if (data->application_version < 1)
                    {
                        const AVHDRPlusColorTransformParams* p = data->params;
                        hdr.sceneMax[0] = 10000.F * av_q2d(p->maxscl[0]);
                        hdr.sceneMax[1] = 10000.F * av_q2d(p->maxscl[1]);
                        hdr.sceneMax[2] = 10000.F * av_q2d(p->maxscl[2]);
                        hdr.sceneAvg = 10000.F * av_q2d(p->average_maxrgb);

                        float histogramMax = 0.F;

                        for (int i = 0;
                             i < p->num_distribution_maxrgb_percentiles; i++)
                        {
                            float value =
                                av_q2d(p->distribution_maxrgb[i].percentile);
                            if (value > histogramMax)
                                histogramMax = value;
                        }

                        histogramMax *= 10000.F;
                        if (!hdr.sceneMax[0])
                            hdr.sceneMax[0] = histogramMax;
                        if (!hdr.sceneMax[1])
                            hdr.sceneMax[1] = histogramMax;
                        if (!hdr.sceneMax[2])
                            hdr.sceneMax[2] = histogramMax;

                        if (p->tone_mapping_flag == 1)
                        {
                            hdr.ootf.targetLuma = av_q2d(
                                data->targeted_system_display_maximum_luminance);
                            hdr.ootf.kneeX = av_q2d(p->knee_point_x);
                            hdr.ootf.kneeY = av_q2d(p->knee_point_y);
                            if (p->num_bezier_curve_anchors < 16)
                            {
                                hdr.ootf.numAnchors =
                                    p->num_bezier_curve_anchors;
                                for (int i = 0; i < hdr.ootf.numAnchors; ++i)
                                    hdr.ootf.anchors[i] =
                                        av_q2d(p->bezier_curve_anchors[i]);
                            }
                        }
                    }
                    break;
                }
                default:
                    break;
                }
            }
            return out;
        }

        bool
        toHDRData(AVFrameSideData** sideData, int size, image::HDRData& hdr)
        {
            bool out = false;
            for (int i = 0; i < size; ++i)
            {
                switch (sideData[i]->type)
                {
                case AV_FRAME_DATA_MASTERING_DISPLAY_METADATA:
                {
                    out = true;
                    auto data = reinterpret_cast<AVMasteringDisplayMetadata*>(
                        sideData[i]->data);
                    if (data->has_luminance)
                    {
                        float max_luma = av_q2d(data->max_luminance);
                        float min_luma = av_q2d(data->min_luminance);
                        if (max_luma < 5.0F || min_luma >= max_luma)
                            max_luma = min_luma = 0.F;

                        hdr.displayMasteringLuminance =
                            math::FloatRange(min_luma, max_luma);
                    }
                    if (data->has_primaries)
                    {
                        hdr.primaries[image::HDRPrimaries::Red].x =
                            av_q2d(data->display_primaries[0][0]);
                        hdr.primaries[image::HDRPrimaries::Red].y =
                            av_q2d(data->display_primaries[0][1]);

                        hdr.primaries[image::HDRPrimaries::Green].x =
                            av_q2d(data->display_primaries[1][0]);
                        hdr.primaries[image::HDRPrimaries::Green].y =
                            av_q2d(data->display_primaries[1][1]);

                        hdr.primaries[image::HDRPrimaries::Blue].x =
                            av_q2d(data->display_primaries[2][0]);
                        hdr.primaries[image::HDRPrimaries::Blue].y =
                            av_q2d(data->display_primaries[2][1]);

                        hdr.primaries[image::HDRPrimaries::White].x =
                            av_q2d(data->white_point[0]);
                        hdr.primaries[image::HDRPrimaries::White].y =
                            av_q2d(data->white_point[1]);
                    }
                    break;
                }
                case AV_FRAME_DATA_CONTENT_LIGHT_LEVEL:
                {
                    out = true;
                    auto data = reinterpret_cast<AVContentLightMetadata*>(
                        sideData[i]->data);
                    hdr.maxCLL = data->MaxCLL;
                    hdr.maxFALL = data->MaxFALL;
                    break;
                }
                case AV_FRAME_DATA_DYNAMIC_HDR_PLUS:
                {
                    out = true;
                    auto data =
                        reinterpret_cast<AVDynamicHDRPlus*>(sideData[i]->data);
                    if (data->application_version < 1)
                    {
                        const AVHDRPlusColorTransformParams* p = data->params;
                        hdr.sceneMax[0] = 10000.F * av_q2d(p->maxscl[0]);
                        hdr.sceneMax[1] = 10000.F * av_q2d(p->maxscl[1]);
                        hdr.sceneMax[2] = 10000.F * av_q2d(p->maxscl[2]);
                        hdr.sceneAvg = 10000.F * av_q2d(p->average_maxrgb);

                        float histogramMax = 0.F;

                        for (int i = 0;
                             i < p->num_distribution_maxrgb_percentiles; i++)
                        {
                            float value =
                                av_q2d(p->distribution_maxrgb[i].percentile);
                            if (value > histogramMax)
                                histogramMax = value;
                        }

                        histogramMax *= 10000.F;
                        if (!hdr.sceneMax[0])
                            hdr.sceneMax[0] = histogramMax;
                        if (!hdr.sceneMax[1])
                            hdr.sceneMax[1] = histogramMax;
                        if (!hdr.sceneMax[2])
                            hdr.sceneMax[2] = histogramMax;

                        if (p->tone_mapping_flag == 1)
                        {
                            hdr.ootf.targetLuma = av_q2d(
                                data->targeted_system_display_maximum_luminance);
                            hdr.ootf.kneeX = av_q2d(p->knee_point_x);
                            hdr.ootf.kneeY = av_q2d(p->knee_point_y);
                            if (p->num_bezier_curve_anchors < 16)
                            {
                                hdr.ootf.numAnchors =
                                    p->num_bezier_curve_anchors;
                                for (int i = 0; i < hdr.ootf.numAnchors; ++i)
                                    hdr.ootf.anchors[i] =
                                        av_q2d(p->bezier_curve_anchors[i]);
                            }
                        }
                    }
                    break;
                }
                case AV_FRAME_DATA_DOVI_METADATA:
                {
                    out = true;
                    // auto data =
                    // reinterpret_cast<AVDOVIMetadata*>(sideData[i]->data);
                    break;
                }
                default:
                    break;
                }
            }
            return out;
        }

        audio::DataType toAudioType(AVSampleFormat value)
        {
            audio::DataType out = audio::DataType::None;
            switch (value)
            {
            case AV_SAMPLE_FMT_S16:
                out = audio::DataType::S16;
                break;
            case AV_SAMPLE_FMT_S32:
                out = audio::DataType::S32;
                break;
            case AV_SAMPLE_FMT_FLT:
                out = audio::DataType::F32;
                break;
            case AV_SAMPLE_FMT_DBL:
                out = audio::DataType::F64;
                break;
            case AV_SAMPLE_FMT_S16P:
                out = audio::DataType::S16;
                break;
            case AV_SAMPLE_FMT_S32P:
                out = audio::DataType::S32;
                break;
            case AV_SAMPLE_FMT_FLTP:
                out = audio::DataType::F32;
                break;
            case AV_SAMPLE_FMT_DBLP:
                out = audio::DataType::F64;
                break;
            default:
                break;
            }
            return out;
        }

        AVSampleFormat fromAudioType(audio::DataType value)
        {
            AVSampleFormat out = AV_SAMPLE_FMT_NONE;
            switch (value)
            {
            case audio::DataType::S16:
                out = AV_SAMPLE_FMT_S16;
                break;
            case audio::DataType::S32:
                out = AV_SAMPLE_FMT_S32;
                break;
            case audio::DataType::F32:
                out = AV_SAMPLE_FMT_FLT;
                break;
            case audio::DataType::F64:
                out = AV_SAMPLE_FMT_DBL;
                break;
            default:
                break;
            }
            return out;
        }

        std::string getTimecodeFromDataStream(AVFormatContext* avFormatContext)
        {
            int dataStream = -1;
            for (unsigned int i = 0; i < avFormatContext->nb_streams; ++i)
            {
                if (AVMEDIA_TYPE_DATA ==
                        avFormatContext->streams[i]->codecpar->codec_type &&
                    AV_DISPOSITION_DEFAULT ==
                        avFormatContext->streams[i]->disposition)
                {
                    dataStream = i;
                    break;
                }
            }
            if (-1 == dataStream)
            {
                for (unsigned int i = 0; i < avFormatContext->nb_streams; ++i)
                {
                    if (AVMEDIA_TYPE_DATA ==
                        avFormatContext->streams[i]->codecpar->codec_type)
                    {
                        dataStream = i;
                        break;
                    }
                }
            }
            std::string timecode;
            if (dataStream != -1)
            {
                AVDictionaryEntry* tag = nullptr;
                while (
                    (tag = av_dict_get(
                         avFormatContext->streams[dataStream]->metadata, "",
                         tag, AV_DICT_IGNORE_SUFFIX)))
                {
                    if (string::compare(
                            tag->key, "timecode",
                            string::Compare::CaseInsensitive))
                    {
                        timecode = tag->value;
                        break;
                    }
                }
            }
            return timecode;
        }

        Packet::Packet()
        {
            p = av_packet_alloc();
        }

        Packet::~Packet()
        {
            av_packet_free(&p);
        }

        std::string getErrorLabel(int r)
        {
            char buf[string::cBufferSize];
            av_strerror(r, buf, string::cBufferSize);
            return std::string(buf);
        }

        std::weak_ptr<log::System> Plugin::_logSystemWeak;

        void Plugin::_init(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            IPlugin::_init(
                "FFmpeg",
                {// Video Formats
                 {".avi", io::FileType::Movie},
                 {".divx", io::FileType::Movie},
                 {".dv", io::FileType::Movie},
                 {".flv", io::FileType::Movie},
                 {".gif", io::FileType::Movie},
                 {".m4v", io::FileType::Movie},
                 {".mkv", io::FileType::Movie},
                 {".mk3d", io::FileType::Movie},
                 {".mov", io::FileType::Movie},
                 {".mp4", io::FileType::Movie},
                 {".mpg", io::FileType::Movie},
                 {".mpeg", io::FileType::Movie},
                 {".mpeg2", io::FileType::Movie},
                 {".mpeg3", io::FileType::Movie},
                 {".mpeg4", io::FileType::Movie},
                 {".mxf", io::FileType::Movie},
                 {".ts", io::FileType::Movie},
                 {".vp9", io::FileType::Movie},
                 {".y4m", io::FileType::Movie},
                 {".webm", io::FileType::Movie},
                 {".wmv", io::FileType::Movie},

                 // Audio Formats
                 {".aiff", io::FileType::Audio},
                 {".mka", io::FileType::Audio},
                 {".m4a", io::FileType::Audio},
                 {".mp3", io::FileType::Audio},
                 {".ogg", io::FileType::Audio},
                 {".opus", io::FileType::Audio},
                 {".vorbis", io::FileType::Audio},
                 {".wav", io::FileType::Audio}},
                cache, logSystem);

            _logSystemWeak = logSystem;
            // av_log_set_level(AV_LOG_QUIET);
            av_log_set_level(AV_LOG_WARNING);
            av_log_set_callback(_logCallback);

            const AVCodec* avCodec = nullptr;
            void* avCodecIterate = nullptr;
            std::vector<std::string> codecNames;
            while ((avCodec = av_codec_iterate(&avCodecIterate)))
            {
                codecNames.push_back(avCodec->name);
            }
            // std::cout << string::join(codecNames, ", ") << std::endl;
            if (auto logSystem = _logSystemWeak.lock())
            {
                logSystem->print(
                    "tl::io::ffmpeg::Plugin",
                    "Codecs: " + string::join(codecNames, ", "));
            }
        }

        Plugin::Plugin() {}

        std::shared_ptr<Plugin> Plugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(cache, logSystem);
            return out;
        }

        std::shared_ptr<io::IRead>
        Plugin::read(const file::Path& path, const io::Options& options)
        {
            return Read::create(path, options, _cache, _logSystem);
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _cache, _logSystem);
        }

        image::Info Plugin::getWriteInfo(
            const image::Info& info, const io::Options& options) const
        {
            image::Info out;
            out.size = info.size;
            switch (info.pixelType)
            {
            case image::PixelType::L_U8:
            case image::PixelType::L_U16:
            case image::PixelType::RGB_U8:
            case image::PixelType::RGB_U16:
            case image::PixelType::RGBA_U8:
            case image::PixelType::RGBA_U16:
                out.pixelType = info.pixelType;
                break;
            case image::PixelType::RGB_F16:
            case image::PixelType::RGB_F32:
                out.pixelType = image::PixelType::RGB_U16;
                break;
            case image::PixelType::RGBA_F16:
            case image::PixelType::RGBA_F32:
                out.pixelType = image::PixelType::RGBA_U16;
                break;
            default:
                break;
            }
            return out;
        }

        std::shared_ptr<io::IWrite> Plugin::write(
            const file::Path& path, const io::Info& info,
            const io::Options& options)
        {
            if (!info.video.empty() &&
                !_isWriteCompatible(info.video[0], options))
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(path.get())
                                             .arg("Unsupported video"));
            return Write::create(path, info, options, _logSystem);
        }

        void
        Plugin::_logCallback(void* avcl, int level, const char* fmt, va_list vl)
        {
            static std::string lastMessage;
            std::string format;

            if (level != AV_LOG_VERBOSE)
            {
                AVClass* avc = avcl ? *(AVClass**)avcl : NULL;
                if (avc)
                {
                    format = "(";
                    format += avc->item_name(avcl);
                    format += ") ";
                }
                format += fmt;
            }

            if (level != AV_LOG_VERBOSE)
            {
                if (auto logSystem = _logSystemWeak.lock())
                {
                    char buf[string::cBufferSize];
                    vsnprintf(buf, string::cBufferSize, format.c_str(), vl);

                    const std::string& message =
                        string::removeTrailingNewlines(buf);

                    if (level < AV_LOG_INFO)
                    {
                        if (message == lastMessage)
                            return;

                        lastMessage = message;
                    }

                    switch (level)
                    {
                    case AV_LOG_PANIC:
                    case AV_LOG_FATAL:
                    case AV_LOG_ERROR:
                        logSystem->print(
                            "tl::io::ffmpeg::Plugin", message, log::Type::Error,
                            "ffmpeg");
                        break;
                    case AV_LOG_WARNING:
                        logSystem->print(
                            "tl::io::ffmpeg::Plugin", message,
                            log::Type::Warning, "ffmpeg");
                        break;
                    case AV_LOG_INFO:
                        logSystem->print(
                            "tl::io::ffmpeg::Plugin", message,
                            log::Type::Message, "ffmpeg");
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    } // namespace ffmpeg
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <tlCore/Math.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/AudioResample.h>
#include <tlCore/LogSystem.h>

#include <tlIO/FFmpeg.h>
#include <tlIO/FFmpegMacros.h>

#ifdef TLRENDER_DOVI
#include <libdovi/rpu_parser.h>
#endif

extern "C"
{

#include <libavutil/audio_fifo.h>
#ifdef TLRENDER_DOVI
#    include <libavutil/dovi_meta.h>
#endif
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libavutil/hdr_dynamic_metadata.h>
#include <libavutil/mastering_display_metadata.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>

#include <libavcodec/version.h>
#include <libavcodec/avcodec.h>

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(61, 13, 100)
#    define HAVE_AVCODEC_GET_SUPPORTED_CONFIG
#endif
}

namespace
{
    const char* kModule = "save";
}

namespace tl
{
    namespace ffmpeg
    {
        namespace
        {
            AVPixelFormat parsePixelFormat(const std::string& c)
            {
                AVPixelFormat o = AV_PIX_FMT_YUV420P;
                const std::string s = string::toUpper(c);

                // Most common formats
                if (s == "YUV420P")
                    o = AV_PIX_FMT_YUV420P;
                else if (s == "YUV422P")
                    o = AV_PIX_FMT_YUV422P;
                else if (s == "YUV444P")
                    o = AV_PIX_FMT_YUV444P;
                else if (s == "YUV_420P")
                    o = AV_PIX_FMT_YUV420P;
                else if (s == "YUV_422P")
                    o = AV_PIX_FMT_YUV422P;
                else if (s == "YUV_444P")
                    o = AV_PIX_FMT_YUV444P;
                else if (s == "RGB_U8")
                    o = AV_PIX_FMT_RGB24;
                else if (s == "RGBA_U8")
                    o = AV_PIX_FMT_RGBA;

                // 10-bits pixel formats

                else if (s == "YUV420P10LE")
                    o = AV_PIX_FMT_YUV420P10LE;
                else if (s == "YUV422P10LE")
                    o = AV_PIX_FMT_YUV422P10LE;
                else if (s == "YUV444P10LE")
                    o = AV_PIX_FMT_YUV444P10LE;

                else if (s == "YUV_420P10LE")
                    o = AV_PIX_FMT_YUV420P10LE;
                else if (s == "YUV_422P10LE")
                    o = AV_PIX_FMT_YUV422P10LE;
                else if (s == "YUV_444P10LE")
                    o = AV_PIX_FMT_YUV444P10LE;

                else if (s == "YUV_420P_10LE")
                    o = AV_PIX_FMT_YUV420P10LE;
                else if (s == "YUV_422P_10LE")
                    o = AV_PIX_FMT_YUV422P10LE;
                else if (s == "YUV_444P_10LE")
                    o = AV_PIX_FMT_YUV444P10LE;

                // 12-bits pixel formats
                else if (s == "YUV420P12LE")
                    o = AV_PIX_FMT_YUV420P12LE;
                else if (s == "YUV422P12LE")
                    o = AV_PIX_FMT_YUV422P12LE;
                else if (s == "YUV444P12LE")
                    o = AV_PIX_FMT_YUV444P12LE;
                else if (s == "YUV_420P12LE")
                    o = AV_PIX_FMT_YUV420P12LE;
                else if (s == "YUV_422P12LE")
                    o = AV_PIX_FMT_YUV422P12LE;
                else if (s == "YUV_444P12LE")
                    o = AV_PIX_FMT_YUV444P12LE;
                else if (s == "YUV_420P_12LE")
                    o = AV_PIX_FMT_YUV420P12LE;
                else if (s == "YUV_422P_12LE")
                    o = AV_PIX_FMT_YUV422P12LE;
                else if (s == "YUV_444P_12LE")
                    o = AV_PIX_FMT_YUV444P12LE;

                // With alpha
                else if (s == "YUVA420P")
                    o = AV_PIX_FMT_YUVA420P;
                else if (s == "YUVA444P10LE")
                    o = AV_PIX_FMT_YUVA444P10LE;
                else if (s == "YUVA444P12LE")
                    o = AV_PIX_FMT_YUVA444P12LE;
                else if (s == "YUVA444P16LE")
                    o = AV_PIX_FMT_YUVA444P16LE;

                else if (s == "YUVA_420P")
                    o = AV_PIX_FMT_YUVA420P;
                else if (s == "YUVA_444P10LE")
                    o = AV_PIX_FMT_YUVA444P10LE;
                else if (s == "YUVA_444P12LE")
                    o = AV_PIX_FMT_YUVA444P12LE;
                else if (s == "YUVA_444P16LE")
                    o = AV_PIX_FMT_YUVA444P16LE;

                else if (s == "YUVA444P")
                    o = AV_PIX_FMT_YUVA444P;
                else if (s == "YUVA_444P_10LE")
                    o = AV_PIX_FMT_YUVA444P10LE;
                else if (s == "YUVA_444P_12LE")
                    o = AV_PIX_FMT_YUVA444P12LE;
                else if (s == "YUVA_444P_16LE")
                    o = AV_PIX_FMT_YUVA444P16LE;

                // BGR formats
                else if (s == "GBRP")
                    o = AV_PIX_FMT_GBRP;
                else if (s == "GBRP10LE")
                    o = AV_PIX_FMT_GBRP10LE;
                else if (s == "GBRP_10LE")
                    o = AV_PIX_FMT_GBRP10LE;
                else if (s == "GBRP12LE")
                    o = AV_PIX_FMT_GBRP12LE;
                else if (s == "GBRP_12LE")
                    o = AV_PIX_FMT_GBRP12LE;

                // BGRA formats
                else if (s == "GBRAP")
                    o = AV_PIX_FMT_GBRAP;
                else if (s == "GBRAP10LE")
                    o = AV_PIX_FMT_GBRAP10LE;
                else if (s == "GBRAP_10LE")
                    o = AV_PIX_FMT_GBRAP10LE;
                else if (s == "GBRAP12LE")
                    o = AV_PIX_FMT_GBRAP12LE;
                else if (s == "GBRAP_12LE")
                    o = AV_PIX_FMT_GBRAP12LE;

                // Hardware formats
                else if (s == "NV12")
                    o = AV_PIX_FMT_NV12;
                else if (s == "P210LE")
                    o = AV_PIX_FMT_P210LE;
                else if (s == "P210_LE")
                    o = AV_PIX_FMT_P210LE;

                else if (s == "P216LE")
                    o = AV_PIX_FMT_P216LE;
                else if (s == "P216_LE")
                    o = AV_PIX_FMT_P216LE;

                else if (s == "P416LE")
                    o = AV_PIX_FMT_P416LE;
                else if (s == "P416_LE")
                    o = AV_PIX_FMT_P416LE;

                else if (s == "AYUV64LE")
                    o = AV_PIX_FMT_AYUV64LE;
                else if (s == "AYUV64_LE")
                    o = AV_PIX_FMT_AYUV64LE;
                else if (s == "AYUV_64_LE")
                    o = AV_PIX_FMT_AYUV64LE;

                else
                    throw std::runtime_error(
                        string::Format("Unknown pixel format {0}").arg(s));
                return o;
            }

            //! Which hardware encoder backend (if any) was used/requested.
            enum class HWBackend {
                kNone,
                kVideoToolbox, // macOS
                kNVENC,        // NVIDIA, plain frames (no CUDA hwframe ctx)
                kVAAPI,        // Linux (Intel/AMD/NVIDIA-via-VAAPI)
                kD3D12VA,      // Windows D3D12VA
            };

            HWBackend parseHWBackend(const std::string& c)
            {
                const std::string s = string::toLower(c);
                if (s == "videotoolbox")
                    return HWBackend::kVideoToolbox;
                else if (s == "nvenc")
                    return HWBackend::kNVENC;
                else if (s == "vaapi")
                    return HWBackend::kVAAPI;
                else if (s == "d3d12va" || s == "d3d12" || s == "directx" ||
                         s == "direct3d")
                    return HWBackend::kD3D12VA;
                return HWBackend::kNone;
            }

            //! Try to find a hardware encoder for \p avCodecID.  If
            //! \p requested is empty or "auto", every backend available on
            //! the current platform is tried, in order of preference,
            //! stopping at the first encoder that FFmpeg actually has
            //! registered.  Otherwise, only the requested backend is
            //! tried.  \p outBackend is set to whichever backend actually
            //! produced an encoder, or HWBackend::kNone if none did.
            const AVCodec* findHardwareEncoder(
                AVCodecID avCodecID, const std::string& requested,
                HWBackend& outBackend)
            {
                outBackend = HWBackend::kNone;
                const bool isAuto =
                    requested.empty() ||
                    string::toLower(requested) == "auto";
                const HWBackend wanted =
                    isAuto ? HWBackend::kNone : parseHWBackend(requested);

                struct Candidate
                {
                    HWBackend backend;
                    std::string name;
                };

                std::vector<Candidate> candidates;
                switch (avCodecID)
                {
                case AV_CODEC_ID_H264:
                    candidates = {
                        {HWBackend::kVideoToolbox, "h264_videotoolbox"},
                        {HWBackend::kNVENC, "h264_nvenc"},
                        {HWBackend::kVAAPI, "h264_vaapi"},
                    };
                    break;
                case AV_CODEC_ID_VP9:
                    // Note: there is no vp9_videotoolbox or vp9_nvenc in
                    // FFmpeg (Apple and NVIDIA do not offer hardware VP9
                    // *encoding*).
                    candidates = {
                        {HWBackend::kVAAPI, "vp9_vaapi"},
                        {HWBackend::kD3D12VA, "vp9_d3d12va"},
                    };
                    break;
                case AV_CODEC_ID_AV1:
                    candidates = {
                        {HWBackend::kVideoToolbox, "av1_videotoolbox"},
                        {HWBackend::kNVENC, "av1_nvenc"},
                        {HWBackend::kVAAPI, "av1_vaapi"},
                    };
                    break;
                case AV_CODEC_ID_HEVC:
                    candidates = {
                        {HWBackend::kVideoToolbox, "hevc_videotoolbox"},
                        {HWBackend::kNVENC, "hevc_nvenc"},
                        {HWBackend::kVAAPI, "hevc_vaapi"},
                    };
                    break;
                case AV_CODEC_ID_PRORES:
                    // ProRes hardware encoding only exists on Apple
                    // silicon via VideoToolbox.
                    candidates = {
                        {HWBackend::kVideoToolbox, "prores_videotoolbox"},
                    };
                    break;
                default:
                    break;
                }

                for (const auto& c : candidates)
                {
                    if (!isAuto && wanted != c.backend)
                        continue;
#ifndef __APPLE__
                    if (c.backend == HWBackend::kVideoToolbox)
                        continue;
#endif
#ifndef __linux__
                    if (c.backend == HWBackend::kVAAPI)
                        continue;
#endif
#ifndef _WIN32
                    if (c.backend == HWBackend::kD3D12VA)
                        continue;
#endif
                    const AVCodec* found =
                        avcodec_find_encoder_by_name(c.name.c_str());
                    if (found)
                    {
                        outBackend = c.backend;
                        return found;
                    }
                }
                return nullptr;
            }

            enum AVPixelFormat choosePixelFormat(
                const AVCodecContext* avctx, const AVCodec* codec,
                enum AVPixelFormat target,
                std::weak_ptr<log::System>& _logSystem)
            {
                const enum AVPixelFormat* p = nullptr;
                int num = 0;
                int i = 0;
                if (codec && avctx)
                {
                    int ok = avcodec_get_supported_config(
                        avctx, codec, AV_CODEC_CONFIG_PIX_FORMAT, 0,
                        (const void**)&p, &num);
                    const AVPixFmtDescriptor* desc =
                        av_pix_fmt_desc_get(target);
                    int has_alpha = desc ? desc->nb_components % 2 == 0 : 0;
                    enum AVPixelFormat best = AV_PIX_FMT_NONE;

                    for (i = 0; i < num; ++i)
                    {
                        best = av_find_best_pix_fmt_of_2(
                            best, p[i], target, has_alpha, NULL);
                        if (p[i] == target)
                            break;
                    }
                    if (i >= num)
                    {
                        if (target != AV_PIX_FMT_NONE)
                        {
                            const char* targetFormat =
                                av_get_pix_fmt_name(target);
                            const char* bestFormat = av_get_pix_fmt_name(best);
                            const std::string msg =
                                string::Format(
                                    "Incompatible pixel format '{0}' for codec "
                                    "'{1}', auto-selecting format '{2}'.")
                                    .arg(targetFormat)
                                    .arg(codec->name)
                                    .arg(bestFormat);
                            LOG_WARNING(msg);
                        }
                        return best;
                    }
                }
                return target;
            }

            AVColorRange parseColorRange(const std::string& c)
            {
                AVColorRange out = AVCOL_RANGE_UNSPECIFIED;
                std::string range = c;
                std::size_t pos = c.find(' ');
                if (pos != std::string::npos)
                {
                    range = c.substr(0, pos - 1);
                }
                const std::string& s = string::toLower(range);
                if (s == "mpeg" || s == "tv")
                    out = AVCOL_RANGE_MPEG;
                else if (s == "jpeg" || s == "pc")
                    out = AVCOL_RANGE_JPEG;
                return out;
            }

            AVColorPrimaries parseColorPrimaries(const std::string& c)
            {
                AVColorPrimaries out = AVCOL_PRI_BT709;
                const std::string& s = string::toLower(c);
                if (s == "unspecified")
                    out = AVCOL_PRI_UNSPECIFIED;
                else if (s == "reserved")
                    out = AVCOL_PRI_RESERVED;
                else if (s == "bt470m")
                    out = AVCOL_PRI_BT470M;
                else if (s == "bt470bg")
                    out = AVCOL_PRI_BT470BG;
                else if (s == "smpte170m")
                    out = AVCOL_PRI_SMPTE170M;
                else if (s == "smpte240m" || s == "smpte-c")
                    out = AVCOL_PRI_SMPTE240M;
                else if (s == "film")
                    out = AVCOL_PRI_FILM;
                else if (s == "bt2020")
                    out = AVCOL_PRI_BT2020;
                else if (s == "smpte428")
                    out = AVCOL_PRI_SMPTE428;
                else if (s == "smpte431")
                    out = AVCOL_PRI_SMPTE431;
                else if (s == "smpte432")
                    out = AVCOL_PRI_SMPTE432;
                else if (s == "ebu3213" || s == "jedec-p22")
                    out = AVCOL_PRI_EBU3213;
                return out;
            }

            AVColorTransferCharacteristic parseColorTRC(const std::string& c)
            {
                AVColorTransferCharacteristic out = AVCOL_TRC_BT709;
                const std::string& s = string::toLower(c);
                if (s == "reserved")
                    out = AVCOL_TRC_RESERVED;
                else if (s == "bt709" || s == "bt1361")
                    out = AVCOL_TRC_BT709;
                else if (s == "unspecified")
                    out = AVCOL_TRC_UNSPECIFIED;
                else if (s == "gamma22")
                    out = AVCOL_TRC_GAMMA22;
                else if (s == "gamma28")
                    out = AVCOL_TRC_GAMMA28;
                else if (s == "smpte170m" || s == "bt601" || s == "bt1358")
                    out = AVCOL_TRC_SMPTE170M;
                else if (s == "smpte240m")
                    out = AVCOL_TRC_SMPTE240M;
                else if (s == "linear")
                    out = AVCOL_TRC_LINEAR;
                else if (s == "log")
                    out = AVCOL_TRC_LOG;
                else if (s == "logsqrt" || s == "log_sqrt")
                    out = AVCOL_TRC_LOG_SQRT;
                else if (s == "iec61966-2-4")
                    out = AVCOL_TRC_IEC61966_2_4;
                else if (s == "bt1361" || s == "bt1361-ecg")
                    out = AVCOL_TRC_BT1361_ECG;
                else if (s == "iec61966-2-1")
                    out = AVCOL_TRC_IEC61966_2_1;
                else if (s == "bt2020-10")
                    out = AVCOL_TRC_BT2020_10;
                else if (s == "bt2020-12")
                    out = AVCOL_TRC_BT2020_12;
                else if (s == "smpte2084" || s == "pq" || s == "bt2100" ||
                         s == "bt2100 (smpte2084)")
                    out = AVCOL_TRC_SMPTE2084;
                else if (s == "arib-std-b67" || s == "hlg")
                    out = AVCOL_TRC_ARIB_STD_B67;

                return out;
            }

            AVColorSpace parseColorSpace(const std::string& c)
            {
                AVColorSpace out = AVCOL_SPC_BT709;
                const std::string& s = string::toLower(c);
                if (s == "rgb" || s == "bgr" || s == "gbr")
                {
                    out = AVCOL_SPC_RGB;
                }
                else if (s == "bt709")
                {
                    out = AVCOL_SPC_BT709;
                }
                else if (s == "unspecified" || s == "unkwown")
                {
                    out = AVCOL_SPC_UNSPECIFIED;
                }
                else if (s == "reserved")
                {
                    out = AVCOL_SPC_RESERVED;
                }
                else if (s == "fcc")
                {
                    out = AVCOL_SPC_FCC;
                }
                else if (s == "bt601" || s == "bt470" || s == "bt470bg" ||
                         s == "bt601 (bt470bg)")
                {
                    out = AVCOL_SPC_BT470BG;
                }
                else if (s == "smpte170m")
                {
                    out = AVCOL_SPC_SMPTE170M;
                }
                else if (s == "smpte240m")
                {
                    out = AVCOL_SPC_SMPTE240M;
                }
                else if (s == "ycgco" || s == "ycocg")
                {
                    out = AVCOL_SPC_YCGCO;
                }
                else if (s == "bt2020" || s == "bt2020nc")
                {
                    out = AVCOL_SPC_BT2020_NCL;
                }
                else if (s == "bt2020c")
                {
                    out = AVCOL_SPC_BT2020_CL;
                }
                else if (s == "smpte2085")
                {
                    out = AVCOL_SPC_SMPTE2085;
                }
                else if (s == "chroma-derived-nc")
                {
                    out = AVCOL_SPC_CHROMA_DERIVED_NCL;
                }
                else if (s == "chroma-derived-c" || s == "chroma-derived-cl")
                {
                    out = AVCOL_SPC_CHROMA_DERIVED_CL;
                }
                else if (s == "ictcp")
                {
                    out = AVCOL_SPC_ICTCP;
                }
                else if (s == "ipt-c2")
                {
                    out = AVCOL_SPC_IPT_C2;
                }
                else if (s == "ycgco-re")
                {
                    out = AVCOL_SPC_YCGCO_RE;
                }
                else if (s == "ycgco-ro")
                {
                    out = AVCOL_SPC_YCGCO_RO;
                }
                return out;
            }

            //! Parse preset file and extract the video codec settings
            void parsePresets(
                AVDictionary*& codecOptions, const std::string& presetFile)
            {
                std::ifstream file(presetFile);
                if (!file.is_open())
                {
                    throw std::runtime_error(
                        string::Format("Unable to open preset file '{0}'.")
                            .arg(presetFile));
                }

                std::string line;
                std::unordered_map<std::string, std::string> settings;
                while (std::getline(file, line))
                {
                    // Ignore lines starting with #
                    if (line.empty() || line[0] == '#')
                        continue;

                    // Find the position of the rightmost colon
                    size_t colonPos = line.rfind(':');
                    if (colonPos == std::string::npos)
                        continue;

                    // Extract option and value
                    std::string option = line.substr(0, colonPos);
                    std::string value = line.substr(colonPos + 1);

                    // Remove leading and trailing whitespaces from option and
                    // value
                    option.erase(0, option.find_first_not_of(" \t"));
                    option.erase(option.find_last_not_of(" \t#") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t#") + 1);

                    // Remove trailing newlines
                    string::removeTrailingNewlines(value);

                    settings[option] = value;
                }
                file.close();

                for (const auto& pair : settings)
                {
                    av_dict_set(
                        &codecOptions, pair.first.c_str(), pair.second.c_str(),
                        0);
                }
            }

            //! Return the equivalent planar format if available.
            AVSampleFormat toPlanarFormat(const enum AVSampleFormat s)
            {
                enum AVSampleFormat out = s;
                switch (s)
                {
                case AV_SAMPLE_FMT_U8:
                    out = AV_SAMPLE_FMT_U8P;
                    break;
                case AV_SAMPLE_FMT_S16:
                    out = AV_SAMPLE_FMT_S16P;
                    break;
                case AV_SAMPLE_FMT_S32:
                    out = AV_SAMPLE_FMT_S32P;
                    break;
                case AV_SAMPLE_FMT_FLT:
                    out = AV_SAMPLE_FMT_FLTP;
                    break;
                case AV_SAMPLE_FMT_DBL:
                    out = AV_SAMPLE_FMT_DBLP;
                    break;
                default:
                    break;
                }
                return out;
            }

            //! Check that a given sample format is supported by the encoder
            bool checkSampleFormat(
                const AVCodecContext* avctx, const AVCodec* codec,
                enum AVSampleFormat sample_fmt)
            {
                bool out = false;
                const enum AVSampleFormat* p = nullptr;
                int num = 0;
                int i = 0;
                if (codec && avctx)
                {
                    int ok = avcodec_get_supported_config(
                        avctx, codec, AV_CODEC_CONFIG_SAMPLE_FORMAT, 0,
                        (const void**)&p, &num);
                    for (i = 0; i < num; ++i)
                    {
                        if (p[i] == sample_fmt)
                        {
                            out = true;
                            break;
                        }
                    }
                }
                return out;
            }

            //! Select layout with equal or the highest channel count
            int selectChannelLayout(
                const AVCodecContext* avctx, const AVCodec* codec,
                AVChannelLayout* dst, int channelCount)
            {
                int out = 1;
                if (codec && avctx)
                {
                    const AVChannelLayout* p = nullptr;
                    const AVChannelLayout* best_ch_layout = nullptr;
                    int best_nb_channels = 0;
                    int num = 0;
                    int i = 0;
                    int ok = avcodec_get_supported_config(
                        avctx, codec, AV_CODEC_CONFIG_CHANNEL_LAYOUT, 0,
                        (const void**)&p, &num);
                    if (num == 0)
                    {
                        av_channel_layout_default(dst, channelCount);
                        out = 0;
                    }
                    else
                    {
                        for (i = 0; i < num; ++i)
                        {
                            int nb_channels = p[i].nb_channels;

                            if (nb_channels > best_nb_channels)
                            {
                                best_ch_layout = &p[i];
                                best_nb_channels = nb_channels;
                            }
                        }
                        out = av_channel_layout_copy(dst, best_ch_layout);
                    }
                }
                return out;
            }

            //! Return an equal or higher supported samplerate
            int selectSampleRate(
                const AVCodecContext* avctx, const AVCodec* codec,
                const int sampleRate)
            {
                int out = 0;
                int* p = nullptr;
                int num = 0;
                int i = 0;
                if (codec && avctx)
                {
                    int ok = avcodec_get_supported_config(
                        avctx, codec, AV_CODEC_CONFIG_SAMPLE_RATE, 0,
                        (const void**)&p, &num);
                    if (num == 0)
                        out = sampleRate;
                    for (i = 0; i < num; ++i)
                    {
                        if (p[i] == sampleRate)
                        {
                            out = sampleRate;
                            break;
                        }

                        if (!out ||
                            abs(sampleRate - p[i]) < abs(sampleRate - out))
                            out = p[i];
                    }
                }
                return out;
            }

        } // namespace

        struct Write::Private
        {
            std::string fileName;
            file::Path path;
            io::Info info;
            io::Options options;
            AVFormatContext* avFormatContext = nullptr;

            // Video
            AVCodecContext* avCodecContext = nullptr;
            AVStream* avVideoStream = nullptr;
            AVPacket* avPacket = nullptr;
            AVFrame* avFrame = nullptr;
            AVPixelFormat avPixelFormatIn = AV_PIX_FMT_NONE;
            AVFrame* avFrame2 = nullptr;
            SwsContext* swsContext = nullptr;
            OTIO_NS::RationalTime videoStartTime = time::invalidTime;
            double avSpeed = 24.0;

            bool hasHDR = false;
            image::HDRData hdr;

            // Hardware Video Encoding
            bool useVAAPI = false;
            bool useD3D12VA = false;

            // sws_scale dest format
            AVPixelFormat avSwPixelFormat = AV_PIX_FMT_NONE;
            // real hw surface (VAAPI or D3D12) sent to the encoder
            AVFrame* avHwFrame = nullptr;
            AVBufferRef* avHWFramesCtx = nullptr;
            AVBufferRef* avHWDeviceCtx = nullptr;

            // Audio
            AVCodecContext* avAudioCodecContext = nullptr;
            AVStream* avAudioStream = nullptr;
            AVAudioFifo* avAudioFifo = nullptr;
            AVFrame* avAudioFrame = nullptr;
            AVPacket* avAudioPacket = nullptr;
            bool avAudioPlanar = false;
            uint64_t totalSamples = 0;
            int64_t audioStartSamples = 0;
            size_t sampleRate = 0;
            std::shared_ptr<audio::AudioResample> resample;
            std::vector<uint8_t*> flatPointers;

            bool opened = false;
        };

        void Write::_init(
            const file::Path& path, const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IWrite::_init(path, options, info, logSystem);

            TLRENDER_P();

            p.fileName = path.get();
            p.path = path;
            p.info = info;
            p.options = options;

            if (info.video.empty() && !info.audio.isValid())
            {
                throw std::runtime_error(
                    string::Format("{0}: No video or audio").arg(p.fileName));
            }
        }

        void Write::writeHeader()
        {
            TLRENDER_P();

            int r = avformat_alloc_output_context2(
                &p.avFormatContext, NULL, NULL, p.fileName.c_str());
            if (r < 0)
                throw std::runtime_error(
                    string::Format("{0}: Could not allocate output context")
                        .arg(p.fileName));

            AVCodec* avCodec = nullptr;
            AVCodecID avAudioCodecID = AV_CODEC_ID_AAC;
            auto option = p.options.find("FFmpeg/AudioCodec");
            if (option != p.options.end())
            {
                AudioCodec audioCodec;
                std::stringstream ss(option->second);
                ss >> audioCodec;
                switch (audioCodec)
                {
                case AudioCodec::kNone:
                    avAudioCodecID = AV_CODEC_ID_NONE;
                    break;
                case AudioCodec::AAC:
                    avAudioCodecID = AV_CODEC_ID_AAC;
                    break;
                case AudioCodec::AC3:
                    avAudioCodecID = AV_CODEC_ID_AC3;
                    break;
                case AudioCodec::True_HD:
                    avAudioCodecID = AV_CODEC_ID_TRUEHD;
                    break;
                case AudioCodec::MP2:
                    avAudioCodecID = AV_CODEC_ID_MP2;
                    break;
                case AudioCodec::MP3:
                    avAudioCodecID = AV_CODEC_ID_MP3;
                    break;
                case AudioCodec::PCM_S16LE:
                    avAudioCodecID = AV_CODEC_ID_PCM_S16LE;
                    break;
                default:
                {
                    const std::string codec = ss.str();
                    const char* name = codec.c_str();
                    avCodec = const_cast<AVCodec*>(
                        avcodec_find_encoder_by_name(name));
                    if (!avCodec)
                    {
                        const AVCodecDescriptor* desc =
                            avcodec_descriptor_get_by_name(name);
                        if (desc)
                        {
                            avAudioCodecID = desc->id;
                        }
                    }
                    break;
                }
                }

                // Sanity check on codecs and containers.
                const std::string extension =
                    string::toLower(p.path.getExtension());
                if (extension == ".wav")
                {
                    if (avAudioCodecID != AV_CODEC_ID_PCM_S16LE &&
                        avAudioCodecID != AV_CODEC_ID_MP3 &&
                        avAudioCodecID != AV_CODEC_ID_AAC)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::ffmpeg::Plugin::Write",
                                "Invalid codec for .wav, switching to AAC",
                                log::Type::Error);
                        }
                        avAudioCodecID = AV_CODEC_ID_AAC;
                    }
                }
                else if (extension == ".aiff")
                {
                    if (avAudioCodecID != AV_CODEC_ID_PCM_S16LE)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::ffmpeg::Plugin::Write",
                                "Invalid codec for .aiff, switching to "
                                "PCM_S16LE",
                                log::Type::Error);
                        }
                        avAudioCodecID = AV_CODEC_ID_PCM_S16LE;
                    }
                }
                else if (extension == ".mp3")
                {
                    if (avAudioCodecID != AV_CODEC_ID_MP3)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::ffmpeg::Plugin::Write",
                                "Invalid codec for .mp3, switching to MP3 "
                                "(needs libmp3lame)",
                                log::Type::Error);
                        }
                        avAudioCodecID = AV_CODEC_ID_MP3;
                    }
                }
                else if (extension == ".opus")
                {
                    if (avAudioCodecID != AV_CODEC_ID_OPUS)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::ffmpeg::Plugin::Write",
                                string::Format(
                                    "Invalid codec for {0}, "
                                    "switching to OPUS (needs libopus)")
                                    .arg(extension),
                                log::Type::Error);
                        }
                        avAudioCodecID = AV_CODEC_ID_OPUS;
                    }
                }
                else if (extension == ".vorbis" || extension == ".ogg")
                {
                    if (avAudioCodecID != AV_CODEC_ID_VORBIS)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::ffmpeg::Plugin::Write",
                                string::Format(
                                    "Invalid codec for {0}, "
                                    "switching to VORBIS (needs libvorbis)")
                                    .arg(extension),
                                log::Type::Error);
                        }
                        avAudioCodecID = AV_CODEC_ID_VORBIS;
                    }
                }
            }

            std::string msg;
            if (p.info.audio.isValid() && avAudioCodecID != AV_CODEC_ID_NONE)
            {
                if (!avCodec)
                    avCodec = const_cast<AVCodec*>(
                        avcodec_find_encoder(avAudioCodecID));
                if (!avCodec)
                    throw std::runtime_error("Could not find audio encoder");

                p.avAudioStream =
                    avformat_new_stream(p.avFormatContext, avCodec);
                if (!p.avAudioStream)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate audio stream")
                            .arg(p.fileName));
                }

                p.avAudioStream->id = p.avFormatContext->nb_streams - 1;

                p.avAudioCodecContext = avcodec_alloc_context3(avCodec);
                if (!p.avAudioCodecContext)
                {
                    throw std::runtime_error(
                        string::Format(
                            "{0}: Cannot allocate audio codec context")
                            .arg(p.fileName));
                }

                bool resample = false;
                p.avAudioCodecContext->sample_fmt =
                    fromAudioType(p.info.audio.dataType);
                if (!checkSampleFormat(
                        p.avAudioCodecContext, avCodec,
                        p.avAudioCodecContext->sample_fmt))
                {
                    // Try it as a planar format then.
                    AVSampleFormat planarFormat =
                        toPlanarFormat(p.avAudioCodecContext->sample_fmt);

                    if (!checkSampleFormat(
                            p.avAudioCodecContext, avCodec, planarFormat))
                    {
                        // If that also failed, initialize a resampler
                        resample = true;

                        if (checkSampleFormat(
                                p.avAudioCodecContext, avCodec,
                                AV_SAMPLE_FMT_FLT))
                        {
                            p.avAudioPlanar = false;
                            p.avAudioCodecContext->sample_fmt =
                                AV_SAMPLE_FMT_FLT;
                        }
                        else if (checkSampleFormat(
                                     p.avAudioCodecContext, avCodec,
                                     AV_SAMPLE_FMT_FLTP))
                        {
                            p.avAudioPlanar = true;
                            p.avAudioCodecContext->sample_fmt =
                                AV_SAMPLE_FMT_FLTP;
                        }
                        else if (checkSampleFormat(
                                     p.avAudioCodecContext, avCodec,
                                     AV_SAMPLE_FMT_S16))
                        {
                            p.avAudioPlanar = false;
                            p.avAudioCodecContext->sample_fmt =
                                AV_SAMPLE_FMT_S16;
                        }
                        else
                        {
                            throw std::runtime_error(
                                string::Format(
                                    "Sample format {0} not supported!")
                                    .arg(av_get_sample_fmt_name(
                                        p.avAudioCodecContext->sample_fmt)));
                        }
                    }
                    else
                    {
                        p.avAudioCodecContext->sample_fmt = planarFormat;
                        p.avAudioPlanar = true;
                    }
                }

                if (p.avAudioPlanar)
                    p.flatPointers.resize(p.info.audio.channelCount);
                else
                    p.flatPointers.resize(1);

                r = selectChannelLayout(
                    p.avAudioCodecContext, avCodec,
                    &p.avAudioCodecContext->ch_layout,
                    p.info.audio.channelCount);
                if (r < 0)
                    throw std::runtime_error(
                        string::Format(
                            "{0}: Could not select audio channel layout")
                            .arg(p.fileName));

                p.sampleRate = selectSampleRate(
                    p.avAudioCodecContext, avCodec, p.info.audio.sampleRate);
                if (p.sampleRate == 0)
                    throw std::runtime_error(
                        string::Format("{0}: Could not select sample rate")
                            .arg(p.fileName));

                char buf[256];
                av_channel_layout_describe(
                    &p.avAudioCodecContext->ch_layout, buf, 256);

                if (p.sampleRate != p.info.audio.sampleRate || resample)
                {
                    const audio::Info& input = p.info.audio;
                    audio::Info output(
                        p.info.audio.channelCount,
                        toAudioType(p.avAudioCodecContext->sample_fmt),
                        p.sampleRate);
                    p.resample = audio::AudioResample::create(input, output);

                    if (auto logSystem = _logSystem.lock())
                    {
                        logSystem->print(
                            "tl::io::ffmpeg::Plugin::Write",
                            string::Format(
                                "Resample from layout {0}, {1} channels, type "
                                "{2}, sample rate {3} to layout {4}, {5} "
                                "channels, type {6}, sample rate {7}.")
                                .arg(buf)
                                .arg(input.channelCount)
                                .arg(input.dataType)
                                .arg(input.sampleRate)
                                .arg(buf)
                                .arg(output.channelCount)
                                .arg(output.dataType)
                                .arg(output.sampleRate));
                    }
                }
                else
                {
                    const audio::Info& input = p.info.audio;
                    if (auto logSystem = _logSystem.lock())
                    {
                        logSystem->print(
                            "tl::io::ffmpeg::Plugin::Write",
                            string::Format(
                                "Save from layout {0}, {1} channels, type "
                                "{2}, sample rate {3}.")
                                .arg(buf)
                                .arg(input.channelCount)
                                .arg(input.dataType)
                                .arg(input.sampleRate));
                    }
                }

                if (avAudioCodecID == AV_CODEC_ID_AC3)
                {
                    p.avAudioCodecContext->block_align = 0;
                    // valid bit rate for ac3 (very strict)
                    p.avAudioCodecContext->bit_rate = 448000;
                }
                else if (avAudioCodecID == AV_CODEC_ID_AAC)
                {
                    p.avAudioCodecContext->block_align = 0;
                    // bit rate typical for stereo
                    p.avAudioCodecContext->bit_rate = 128000;
                }
                else if (avAudioCodecID == AV_CODEC_ID_VORBIS)
                {
                    p.avAudioCodecContext->block_align = 0;
                    // bit rate typical for vorbis
                    p.avAudioCodecContext->bit_rate = 192000;
                }
                else if (avAudioCodecID == AV_CODEC_ID_MP3)
                {
                    // bit rate typical for MP3 (block align done later)
                    p.avAudioCodecContext->bit_rate = 192000;
                }
                else if (avAudioCodecID == AV_CODEC_ID_OPUS)
                {
                    p.avAudioCodecContext->block_align = 0;
                    // bit rate recommended for speech
                    p.avAudioCodecContext->bit_rate = 64000;
                }

                p.avAudioCodecContext->sample_rate = p.sampleRate;
                p.avAudioCodecContext->time_base.num = 1;
                p.avAudioCodecContext->time_base.den = p.sampleRate;

                if ((p.avAudioCodecContext->block_align == 1 ||
                     p.avAudioCodecContext->block_align == 1152 ||
                     p.avAudioCodecContext->block_align == 576) &&
                    p.avAudioCodecContext->codec_id == AV_CODEC_ID_MP3)
                    p.avAudioCodecContext->block_align = 0;

                r = avcodec_open2(p.avAudioCodecContext, avCodec, NULL);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Could not open audio codec - {1}.")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                const std::string codecName = avCodec->name;
                msg = string::Format("Tring to save audio with '{0}' codec.")
                          .arg(codecName);
                LOG_STATUS(msg);

                r = avcodec_parameters_from_context(
                    p.avAudioStream->codecpar, p.avAudioCodecContext);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Could not copy parameters from "
                                       "context - {1}.")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                p.avAudioPacket = av_packet_alloc();
                if (!p.avAudioPacket)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate audio packet")
                            .arg(p.fileName));
                }

                p.avAudioFifo = av_audio_fifo_alloc(
                    p.avAudioCodecContext->sample_fmt,
                    p.info.audio.channelCount,
                    1); // cannot be 0, must be 1 at least
                if (!p.avAudioFifo)
                {
                    throw std::runtime_error(
                        string::Format(
                            "{0}: Cannot allocate audio FIFO buffer - {1}.")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                p.avAudioFrame = av_frame_alloc();
                if (!p.avAudioFrame)
                {
                    throw std::runtime_error(
                        string::Format(
                            "{0}: Cannot allocate audio frame - {1}.")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                int workSize = p.avAudioCodecContext->frame_size;
                if (workSize <= 0 && avAudioCodecID != AV_CODEC_ID_PCM_S16LE)
                {
                    workSize = 1024;
                }

                p.avAudioFrame->nb_samples = workSize;
                p.avAudioFrame->format = p.avAudioCodecContext->sample_fmt;
                p.avAudioFrame->sample_rate = p.sampleRate;
                r = av_channel_layout_copy(
                    &p.avAudioFrame->ch_layout,
                    &p.avAudioCodecContext->ch_layout);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Could not copy channel layout to "
                                       "audio frame - {1}.")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                r = av_frame_get_buffer(p.avAudioFrame, 0);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Could not allocate buffer for "
                                       "audio frame - {1}.")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }
            }

            if (!p.info.video.empty())
            {
                AVCodecID avCodecID = AV_CODEC_ID_MPEG4;
                Profile profile = Profile::kNone;
                int avProfile = AV_PROFILE_UNKNOWN;
                auto option = p.options.find("FFmpeg/WriteProfile");
                if (option != p.options.end())
                {
                    std::stringstream ss(option->second);
                    ss >> profile;
                }
                bool hardwareEncode = false;
                option = p.options.find("FFmpeg/HardwareEncode");
                if (option != p.options.end())
                {
                    std::stringstream ss(option->second);
                    ss >> hardwareEncode;
                }
                std::string avBitrate;
                std::string profileString;
                switch (profile)
                {
                case Profile::H264:
                    avCodecID = AV_CODEC_ID_H264;
                    avProfile = AV_PROFILE_H264_HIGH;
                    break;
                case Profile::ProRes:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = AV_PROFILE_PRORES_STANDARD;
                    profileString = "standard";
                    break;
                case Profile::ProRes_Proxy:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = AV_PROFILE_PRORES_PROXY;
                    profileString = "proxy";
                    break;
                case Profile::ProRes_LT:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = AV_PROFILE_PRORES_LT;
                    profileString = "lt";
                    break;
                case Profile::ProRes_HQ:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = AV_PROFILE_PRORES_HQ;
                    profileString = "hq";
                    break;
                case Profile::ProRes_4444:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = AV_PROFILE_PRORES_4444;
                    profileString = "4444";
                    break;
                case Profile::ProRes_XQ:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = AV_PROFILE_PRORES_XQ;
                    profileString = "4444xq";
                    break;
                case Profile::DNxHD:
                    avCodecID = AV_CODEC_ID_DNXHD;
                    avProfile = AV_PROFILE_DNXHD;
                    avBitrate = "44000";
                    break;
                case Profile::DNxHR_LB:
                    avCodecID = AV_CODEC_ID_DNXHD;
                    avProfile = AV_PROFILE_DNXHR_LB;
                    profileString = "dnxhr_lb";
                    break;
                case Profile::DNxHR_SQ:
                    avCodecID = AV_CODEC_ID_DNXHD;
                    avProfile = AV_PROFILE_DNXHR_SQ;
                    profileString = "dnxhr_sq";
                    break;
                case Profile::DNxHR_HQ:
                    avCodecID = AV_CODEC_ID_DNXHD;
                    avProfile = AV_PROFILE_DNXHR_HQ;
                    profileString = "dnxhr_hq";
                    break;
                case Profile::DNxHR_HQX:
                    avCodecID = AV_CODEC_ID_DNXHD;
                    avProfile = AV_PROFILE_DNXHR_HQX;
                    profileString = "dnxhr_hqx";
                    break;
                case Profile::DNxHR_444:
                    avCodecID = AV_CODEC_ID_DNXHD;
                    avProfile = AV_PROFILE_DNXHR_444;
                    profileString = "dnxhr_444";
                    break;
                case Profile::VP9:
                    avCodecID = AV_CODEC_ID_VP9;
                    avProfile = AV_PROFILE_UNKNOWN;
                    break;
                case Profile::Cineform:
                    avCodecID = AV_CODEC_ID_CFHD;
                    avProfile = AV_PROFILE_UNKNOWN;
                    break;
                case Profile::AV1:
                    avCodecID = AV_CODEC_ID_AV1;
                    // AV_PROFILE_UNKNOWN (-99) works fine for software
                    // encoders (libaom/libsvtav1 just pick their own
                    // default), but hardware backends like av1_d3d12va
                    // and av1_vaapi have to translate this value into a
                    // concrete driver-level profile enum before they can
                    // even validate the encode session -- passing -99
                    // through produces an invalid enum and the driver
                    // rejects it as "Codec configuration not supported"
                    // at avcodec_open2() time.  Main is the only AV1
                    // profile that's broadly supported (4:2:0, 8/10-bit)
                    // and matches the NV12/P010 hw surfaces we upload.
                    avProfile = AV_PROFILE_AV1_MAIN;
                    break;
                case Profile::HAP:
                    avCodecID = AV_CODEC_ID_HAP;
                    avProfile = AV_PROFILE_UNKNOWN;
                    break;
                case Profile::AV1_AOM:
                    avCodecID = AV_CODEC_ID_AV1;
                    avProfile = AV_PROFILE_AV1_MAIN;
                    break;
                case Profile::HEVC:
                    avCodecID = AV_CODEC_ID_HEVC;
                    // Same reasoning as AV1 above -- hevc_d3d12va and
                    // hevc_vaapi need a concrete profile.  Refined for
                    // 10-bit/4:2:2/4:4:4 sources just below, alongside
                    // the existing H.264 bit-depth switch.
                    avProfile = AV_PROFILE_HEVC_MAIN;
                    break;
                default:
                    break;
                }

                p.avSpeed = p.info.videoTime->duration().rate();
                const auto& videoInfo = p.info.video[0];

                // Allow setting the speed if not saving audio
                if (!p.info.audio.isValid() ||
                    avAudioCodecID == AV_CODEC_ID_NONE)
                {
                    option = p.options.find("FFmpeg/Speed");
                    if (option != p.options.end())
                    {
                        std::stringstream ss(option->second);
                        ss >> p.avSpeed;
                    }
                }

                // Which hardware backend to try: "auto" (default) tries
                // VideoToolbox / NVENC / VAAPI / D3D12VA in that order,
                // whichever is actually registered on this
                // build/platform.  It can also be forced to one specific
                // backend via "FFmpeg/HardwareEncoder" (e.g. "d3d12va").
                std::string hwBackendOption = "auto";
                option = p.options.find("FFmpeg/HardwareEncoder");
                if (option != p.options.end())
                {
                    std::stringstream ss(option->second);
                    ss >> hwBackendOption;
                }

                HWBackend hwBackend = HWBackend::kNone;
                const AVCodec* avCodec = nullptr;
                if (hardwareEncode)
                {
                    if (hardwareEncode)
                    {
                        LOG_STATUS("Trying Hardware encoding.");
                    }
                    avCodec = findHardwareEncoder(
                        avCodecID, hwBackendOption, hwBackend);
                    if (!avCodec)
                    {
                        // No matching hardware encoder was found/registered
                        // -- fall back to software below.
                        hardwareEncode = false;
                        LOG_STATUS(
                            "No hardware encoder available, using "
                            "software encoder instead.");
                    }
                    else
                    {
                        LOG_STATUS(
                            string::Format(
                                "Using hardware encoder '{0}'.")
                                .arg(avCodec->name));
                    }
                }

                // Software fallbacks / codec-specific default encoder
                // names, used when hardware encoding was off, unavailable,
                // or not requested for this codec.
                if (!avCodec && avCodecID == AV_CODEC_ID_VP9)
                {
                    avCodec = avcodec_find_encoder_by_name("libvpx-vp9");
                }
                else if (!avCodec && avCodecID == AV_CODEC_ID_AV1 &&
                         profile == Profile::AV1_AOM)
                {
                    avCodec = avcodec_find_encoder_by_name("libaom-av1");
                }
                else if (!avCodec && avCodecID == AV_CODEC_ID_PRORES)
                {
                    avCodec = avcodec_find_encoder_by_name("prores_ks");
                }

                if (!avCodec)
                    avCodec = avcodec_find_encoder(avCodecID);
                if (!avCodec)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot find video encoder")
                            .arg(p.fileName));
                }
                const std::string codecName = avCodec->name;

                // Safety net: if avcodec_find_encoder() above happened to
                // resolve to a hardware encoder on its own (e.g. no
                // software encoder was compiled in), make sure our state
                // reflects that.
                if (hwBackend == HWBackend::kNone)
                {
                    if (codecName.find("videotoolbox") != std::string::npos)
                        hwBackend = HWBackend::kVideoToolbox;
                    else if (codecName.find("nvenc") != std::string::npos)
                        hwBackend = HWBackend::kNVENC;
                    else if (codecName.find("vaapi") != std::string::npos)
                        hwBackend = HWBackend::kVAAPI;
                    else if (codecName.find("d3d12va") != std::string::npos)
                        hwBackend = HWBackend::kD3D12VA;
                }
                if (hwBackend != HWBackend::kNone)
                    hardwareEncode = true;

                // VAAPI and D3D12VA are the only backends here that
                // require an AVHWDeviceContext / AVHWFramesContext and
                // real hardware surfaces -- VideoToolbox and NVENC both
                // accept plain system-memory AVFrames directly.
                const bool useVAAPI = (hwBackend == HWBackend::kVAAPI);
                const bool useD3D12VA = (hwBackend == HWBackend::kD3D12VA);
                p.useVAAPI = useVAAPI;
                p.useD3D12VA = useD3D12VA;
                p.avCodecContext = avcodec_alloc_context3(avCodec);
                if (!p.avCodecContext)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate video context")
                            .arg(p.fileName));
                }
                p.avVideoStream =
                    avformat_new_stream(p.avFormatContext, avCodec);
                if (!p.avVideoStream)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate video stream")
                            .arg(p.fileName));
                }
                p.avVideoStream->id = p.avFormatContext->nb_streams - 1;

                if ((videoInfo.size.w == 0 && videoInfo.size.h == 0) ||
                    videoInfo.size.pixelAspectRatio == 0)
                {
                    throw std::runtime_error("Resolution to save is 0x0");
                }

                p.avCodecContext->codec_id = avCodec->id;
                p.avCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
                p.avCodecContext->width = videoInfo.size.w;
                p.avCodecContext->height = videoInfo.size.h;
                p.avCodecContext->sample_aspect_ratio = AVRational({1, 1});
                const auto rational = time::toRational(p.avSpeed);
                p.avCodecContext->time_base = {rational.second, rational.first};
                p.avCodecContext->framerate = {rational.first, rational.second};

                if (avCodecID == AV_CODEC_ID_PRORES || hardwareEncode)
                {
                    // Equivalent to -color_range tv (1)
                    p.avCodecContext->color_range = AVCOL_RANGE_MPEG;
                }
                else
                {
                    // Equivalent to -color_range pc (2)
                    p.avCodecContext->color_range = AVCOL_RANGE_JPEG;
                }

                std::string value;
                option = p.options.find("FFmpeg/ColorRange");
                if (option != p.options.end())
                {
                    std::stringstream ss(option->second);
                    ss >> value;
                    LOG_STATUS(
                        string::Format("Parsing color range {0}").arg(value));
                    p.avCodecContext->color_range = parseColorRange(value);
                }
                if (p.avCodecContext->color_range == AVCOL_RANGE_MPEG)
                    value = "TV (Legal Range)";
                else
                    value = "PC (Full Range)";
                LOG_STATUS(string::Format("Color Range is {0}").arg(value));

                // Equivalent to -colorspace bt709
                p.avCodecContext->colorspace = AVCOL_SPC_BT709;
                option = p.options.find("FFmpeg/ColorSpace");
                if (option != p.options.end())
                {
                    std::stringstream ss(option->second);
                    ss >> value;
                    LOG_STATUS(
                        string::Format("Parsing color space {0}").arg(value));
                    p.avCodecContext->colorspace = parseColorSpace(value);
                }
                value = av_color_space_name(p.avCodecContext->colorspace);
                LOG_STATUS(string::Format("FFmpeg Color Space is {0}").arg(value));

                // Equivalent to -color_primaries bt709
                p.avCodecContext->color_primaries = AVCOL_PRI_BT709;
                option = p.options.find("FFmpeg/ColorPrimaries");
                if (option != p.options.end())
                {
                    std::stringstream ss(option->second);
                    ss >> value;
                    LOG_STATUS(string::Format("Parsing color primaries {0}")
                                   .arg(value));
                    p.avCodecContext->color_primaries =
                        parseColorPrimaries(value);
                }

                value =
                    av_color_primaries_name(p.avCodecContext->color_primaries);
                LOG_STATUS(string::Format("FFmpeg Color Primaries is {0}").arg(value));

                // Equivalent to -color_trc iec61966-2-1 (ie. sRGB)
                if (!hardwareEncode)
                    p.avCodecContext->color_trc = AVCOL_TRC_IEC61966_2_1;
                else
                    p.avCodecContext->color_trc = AVCOL_TRC_BT709;
                option = p.options.find("FFmpeg/ColorTRC");
                if (option != p.options.end())
                {
                    std::stringstream ss(option->second);
                    ss >> value;
                    LOG_STATUS(
                        string::Format("Parsing color trc {0}").arg(value));
                    p.avCodecContext->color_trc = parseColorTRC(value);
                }
                value = av_color_transfer_name(p.avCodecContext->color_trc);
                LOG_STATUS(string::Format("FFmpeg Color TRC is {0}").arg(value));

                if ((p.avFormatContext->oformat->flags & AVFMT_GLOBALHEADER) &&
                    !useD3D12VA)
                {
                    // NOTE: as of this writing, FFmpeg's D3D12VA hardware
                    // encoders (h264_d3d12va/hevc_d3d12va/av1_d3d12va)
                    // never populate AVCodecContext::extradata -- they
                    // unconditionally bake VPS/SPS/PPS (or the AV1
                    // equivalent) into the bitstream of every IDR frame
                    // instead, regardless of this flag.  If we still
                    // requested AV_CODEC_FLAG_GLOBAL_HEADER, muxers that
                    // build their container-level header from extradata
                    // (matroska's hvcC "CodecPrivate", mp4's avcC/hvcC
                    // box, etc.) receive an empty buffer and
                    // avformat_write_header() fails immediately with
                    // "Invalid data found when processing input" --
                    // which is exactly the failure this avoids.  Leaving
                    // the flag off lets the muxer fall back to treating
                    // the stream as Annex-B with in-band parameter sets,
                    // which is what the encoder actually produces.
                    p.avCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
                }
                p.avCodecContext->thread_count = 0;
                p.avCodecContext->thread_type = FF_THREAD_FRAME;

                // Get the pixel format from the options.
                std::string pixelFormat = "YUV420P_U8"; // all codecs support it
                option = p.options.find("FFmpeg/PixelFormat");
                if (option != p.options.end())
                {
                    std::stringstream ss(option->second);
                    ss >> pixelFormat;
                }

                // Parse the pixel format and check that it is a valid one.
                AVPixelFormat pix_fmt = parsePixelFormat(pixelFormat);
                if (!useVAAPI && !useD3D12VA)
                {
                    // avcodec_get_supported_config() for a *_vaapi or
                    // *_d3d12va encoder only ever reports its own hw
                    // pixel format, so this validation step does not
                    // apply -- the real pixel format lives one level
                    // down, in the hw frame's sw_format (set up below).
                    pix_fmt = choosePixelFormat(
                        p.avCodecContext, avCodec, pix_fmt, _logSystem);
                }
                // Actual pixel format of the software buffer that gets
                // rendered into with sws_scale, and either handed to the
                // encoder directly (VideoToolbox/NVENC/software) or
                // uploaded to a real VAAPI/D3D12 hardware surface.
                // Starts out equal to pix_fmt, but real hw surfaces
                // require remapping below.
                AVPixelFormat swFormat = pix_fmt;
                if (useVAAPI || useD3D12VA)
                {
                    // Real VAAPI / D3D12 hardware surfaces only accept a
                    // handful of packed sw_formats that map onto an
                    // actual GPU texture layout -- NV12 for 8-bit 4:2:0,
                    // P010 for higher bit depth 4:2:0.  A planar format
                    // like AV_PIX_FMT_YUV420P (the default, and what a
                    // user's "FFmpeg/PixelFormat" option may resolve to)
                    // is NOT a valid AVHWFramesContext::sw_format and
                    // av_hwframe_ctx_init() will fail with "Invalid
                    // argument" / "Unsupported pixel format" if used
                    // directly.  Map down to the closest hw-compatible
                    // format here; sws_scale (set up further below)
                    // converts into it right before upload.  Profile
                    // selection just below still uses the original
                    // pix_fmt so e.g. a 10-bit source still selects a
                    // 10-bit H.264 profile even though the actual GPU
                    // surface is P010.
                    const AVPixFmtDescriptor* desc =
                        av_pix_fmt_desc_get(pix_fmt);
                    const int depth = desc ? desc->comp[0].depth : 8;
                    swFormat =
                        (depth > 8) ? AV_PIX_FMT_P010 : AV_PIX_FMT_NV12;
                }
                p.avCodecContext->pix_fmt =
                    useVAAPI     ? AV_PIX_FMT_VAAPI
                    : useD3D12VA ? AV_PIX_FMT_D3D12
                                 : pix_fmt;
                p.avSwPixelFormat = swFormat;

                if (profile == Profile::H264)
                {
                    switch (pix_fmt)
                    {
                    case AV_PIX_FMT_YUV420P10LE:
                        avProfile = AV_PROFILE_H264_HIGH_10;
                        break;
                    case AV_PIX_FMT_YUV422P10LE:
                        avProfile = AV_PROFILE_H264_HIGH_422;
                        break;
                    case AV_PIX_FMT_YUV444P10LE:
                        avProfile = AV_PROFILE_H264_HIGH_444;
                        break;
                    default:
                        avProfile = AV_PROFILE_H264_HIGH;
                    }
                }
                else if (profile == Profile::HEVC)
                {
                    switch (pix_fmt)
                    {
                    case AV_PIX_FMT_YUV420P10LE:
                        avProfile = AV_PROFILE_HEVC_MAIN_10;
                        break;
                    case AV_PIX_FMT_YUV422P:
                    case AV_PIX_FMT_YUV422P10LE:
                    case AV_PIX_FMT_YUV444P:
                    case AV_PIX_FMT_YUV444P10LE:
                        // 4:2:2/4:4:4 need the Range Extensions profile;
                        // not all hw backends support it (hevc_d3d12va
                        // in particular is 4:2:0-only right now), so
                        // this combination may still need to fall back
                        // to software or 4:2:0 upstream.
                        avProfile = AV_PROFILE_HEVC_REXT;
                        break;
                    default:
                        avProfile = AV_PROFILE_HEVC_MAIN;
                    }
                }

                p.avCodecContext->profile = avProfile;

                // Get codec options from preset file
                AVDictionary* codecOptions = NULL;

                std::string presetFile;
                option = p.options.find("FFmpeg/PresetFile");
                if (option != p.options.end())
                {
                    std::stringstream ss(option->second);
                    std::getline(ss, presetFile);
                    presetFile =
                        string::removeTrailingNewlines(presetFile.c_str());
                }

                msg = string::Format("Trying to save video with '{0}' codec.")
                          .arg(codecName);
                LOG_STATUS(msg);

                msg = string::Format("FFmpeg pixel format '{0}'.")
                          .arg(av_get_pix_fmt_name(pix_fmt));
                LOG_STATUS(msg);

                if (hardwareEncode)
                {
                    LOG_STATUS("Hardware encoding is on.");
                }
                else
                {
                    LOG_STATUS("Hardware encoding is off.");

                    if (!profileString.empty())
                    {
                        av_dict_set(&codecOptions, "profile", profileString.c_str(),
                                    0);
                    }
                    if (!avBitrate.empty())
                    {
                        av_dict_set(&codecOptions, "bitrate", avBitrate.c_str(),
                                    0);
                    }

                    if (!presetFile.empty())
                    {
                        parsePresets(codecOptions, presetFile);
                    }
                }

                if (useVAAPI)
                {
                    // Optional render node, e.g. "/dev/dri/renderD128".
                    // Left empty, FFmpeg picks the first VAAPI-capable
                    // device it finds.
                    std::string vaapiDevice;
                    option = p.options.find("FFmpeg/VAAPIDevice");
                    if (option != p.options.end())
                    {
                        std::stringstream ss(option->second);
                        ss >> vaapiDevice;
                    }

                    AVBufferRef* hwDeviceCtx = nullptr;
                    r = av_hwdevice_ctx_create(
                        &hwDeviceCtx, AV_HWDEVICE_TYPE_VAAPI,
                        vaapiDevice.empty() ? nullptr : vaapiDevice.c_str(),
                        nullptr, 0);
                    if (r < 0)
                    {
                        throw std::runtime_error(
                            string::Format(
                                "{0}: Cannot create VAAPI device - {1}")
                                .arg(p.fileName)
                                .arg(getErrorLabel(r)));
                    }

                    AVBufferRef* hwFramesRef =
                        av_hwframe_ctx_alloc(hwDeviceCtx);
                    if (!hwFramesRef)
                    {
                        av_buffer_unref(&hwDeviceCtx);
                        throw std::runtime_error(
                            string::Format(
                                "{0}: Cannot allocate VAAPI frames context")
                                .arg(p.fileName));
                    }

                    AVHWFramesContext* framesCtx =
                        reinterpret_cast<AVHWFramesContext*>(
                            hwFramesRef->data);
                    framesCtx->format = AV_PIX_FMT_VAAPI;
                    framesCtx->sw_format = p.avSwPixelFormat;
                    framesCtx->width = p.avCodecContext->width;
                    framesCtx->height = p.avCodecContext->height;
                    // A small pool is enough since we upload/encode one
                    // frame at a time and immediately release it.
                    framesCtx->initial_pool_size = 4;

                    r = av_hwframe_ctx_init(hwFramesRef);
                    if (r < 0)
                    {
                        av_buffer_unref(&hwFramesRef);
                        av_buffer_unref(&hwDeviceCtx);
                        throw std::runtime_error(
                            string::Format(
                                "{0}: Cannot initialize VAAPI frames "
                                "context - {1}")
                                .arg(p.fileName)
                                .arg(getErrorLabel(r)));
                    }

                    p.avCodecContext->hw_device_ctx =
                        av_buffer_ref(hwDeviceCtx);
                    p.avCodecContext->hw_frames_ctx =
                        av_buffer_ref(hwFramesRef);

                    // Private keeps its own references so it can release
                    // them independently of the codec context.
                    p.avHWDeviceCtx = hwDeviceCtx;
                    p.avHWFramesCtx = hwFramesRef;
                }

#ifdef _WIN32
                if (useD3D12VA)
                {
                    // Optional adapter index (e.g. "0", "1", ...) picking
                    // which GPU to use.  Left empty, FFmpeg picks the
                    // default D3D12 adapter.
                    std::string d3d12Device;
                    option = p.options.find("FFmpeg/D3D12Device");
                    if (option != p.options.end())
                    {
                        std::stringstream ss(option->second);
                        ss >> d3d12Device;
                    }

                    AVBufferRef* hwDeviceCtx = nullptr;
                    r = av_hwdevice_ctx_create(
                        &hwDeviceCtx, AV_HWDEVICE_TYPE_D3D12VA,
                        d3d12Device.empty() ? nullptr : d3d12Device.c_str(),
                        nullptr, 0);
                    if (r < 0)
                    {
                        throw std::runtime_error(
                            string::Format(
                                "{0}: Cannot create Direct3D 12 device - {1}")
                                .arg(p.fileName)
                                .arg(getErrorLabel(r)));
                    }

                    AVBufferRef* hwFramesRef =
                        av_hwframe_ctx_alloc(hwDeviceCtx);
                    if (!hwFramesRef)
                    {
                        av_buffer_unref(&hwDeviceCtx);
                        throw std::runtime_error(
                            string::Format(
                                "{0}: Cannot allocate Direct3D 12 frames "
                                "context")
                                .arg(p.fileName));
                    }

                    AVHWFramesContext* framesCtx =
                        reinterpret_cast<AVHWFramesContext*>(
                            hwFramesRef->data);
                    framesCtx->format = AV_PIX_FMT_D3D12;
                    // D3D12 video-encode surfaces are typically
                    // restricted to NV12 (8-bit) or P010 (10-bit); other
                    // sw_formats will fail at av_hwframe_ctx_init()
                    // below with a clear error rather than silently
                    // misbehaving.
                    framesCtx->sw_format = p.avSwPixelFormat;
                    framesCtx->width = p.avCodecContext->width;
                    framesCtx->height = p.avCodecContext->height;
                    // A small pool is enough since we upload/encode one
                    // frame at a time and immediately release it.
                    framesCtx->initial_pool_size = 1;

                    r = av_hwframe_ctx_init(hwFramesRef);
                    if (r < 0)
                    {
                        av_buffer_unref(&hwFramesRef);
                        av_buffer_unref(&hwDeviceCtx);
                        throw std::runtime_error(
                            string::Format(
                                "{0}: Cannot initialize Direct3D 12 frames "
                                "context - {1}")
                                .arg(p.fileName)
                                .arg(getErrorLabel(r)));
                    }

                    p.avCodecContext->hw_device_ctx =
                        av_buffer_ref(hwDeviceCtx);
                    p.avCodecContext->hw_frames_ctx =
                        av_buffer_ref(hwFramesRef);

                    // Private keeps its own references so it can release
                    // them independently of the codec context.
                    p.avHWDeviceCtx = hwDeviceCtx;
                    p.avHWFramesCtx = hwFramesRef;
                }
#endif // _WIN32

                r = avcodec_open2(p.avCodecContext, avCodec, &codecOptions);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: avcodec_open2 Could not open "
                                       "video codec - {1}")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                av_dict_free(&codecOptions);

                if (p.avCodecContext->codec && p.avCodecContext->codec->name)
                {
                    if (auto logSystem = _logSystem.lock())
                    {
                        logSystem->print(
                            "tl::io::ffmpeg::Plugin::Write",
                            string::Format("{0}: Opened codec {1}.")
                                .arg(p.fileName)
                                .arg(p.avCodecContext->codec->name));
                    }
                }

                r = avcodec_parameters_from_context(
                    p.avVideoStream->codecpar, p.avCodecContext);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format(
                            "{0}: avcodec_parameters_from_context - {1}")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                _attach_stream_hdr_metadata(p.avVideoStream);
                p.avVideoStream->time_base = {rational.second, rational.first};
                p.avVideoStream->avg_frame_rate = {
                    rational.first, rational.second};
                if (profile == Profile::VP9)
                {

                    if (pix_fmt == AV_PIX_FMT_YUVA420P)
                    {
                        av_dict_set(
                            &p.avVideoStream->metadata, "alpha_mode", "1", 0);

                        const std::string& extension =
                            string::toLower(p.path.getExtension());

                        // \bug: this does not add alpha_mode to the .webm
                        //       or .mp4 container
                        if (extension != ".mkv" && extension != ".mk3d")
                        {
                            throw std::runtime_error(
                                "To save with an alpha "
                                "channel you need a .mkv "
                                "or .mk3d movie extension");
                        }
                    }
                }

                for (const auto& i : p.info.tags)
                {
                    av_dict_set(
                        &p.avFormatContext->metadata, i.first.c_str(),
                        i.second.c_str(), 0);
                }

                p.videoStartTime = p.info.videoTime->start_time();
                // Set timecode
                option = p.options.find("timecode");
                if (option != p.options.end())
                {
                    std::stringstream ss(option->second);
                    std::string timecode;
                    ss >> timecode;
                    r = av_dict_set(
                        &p.avFormatContext->metadata, "timecode",
                        timecode.c_str(), 0);
                    if (r < 0)
                        throw std::runtime_error(
                            string::Format("Could not set timecode to {1}")
                                .arg(timecode));

                    opentime::ErrorStatus errorStatus;
                    const OTIO_NS::RationalTime time =
                        OTIO_NS::RationalTime::from_timecode(
                            timecode, p.info.videoTime->duration().rate(),
                            &errorStatus);
                    if (!opentime::is_error(errorStatus))
                    {
                        p.videoStartTime = time.floor();
                    }
                }

                p.avPacket = av_packet_alloc();
                if (!p.avPacket)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate packet")
                            .arg(p.fileName));
                }

                p.avFrame = av_frame_alloc();
                if (!p.avFrame)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate main video frame")
                            .arg(p.fileName));
                }
                p.avFrame->format = p.avSwPixelFormat;
                p.avFrame->width = p.avVideoStream->codecpar->width;
                p.avFrame->height = p.avVideoStream->codecpar->height;

                r = av_frame_get_buffer(p.avFrame, 0);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: av_frame_get_buffer - {1}")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                if (p.useVAAPI || p.useD3D12VA)
                {
                    // Real hardware surface handed to the encoder; filled
                    // in each writeVideo() call via
                    // av_hwframe_get_buffer()/av_hwframe_transfer_data()
                    // from p.avFrame above.
                    p.avHwFrame = av_frame_alloc();
                    if (!p.avHwFrame)
                    {
                        throw std::runtime_error(
                            string::Format(
                                "{0}: Cannot allocate hardware frame")
                                .arg(p.fileName));
                    }
                }

                p.avFrame2 = av_frame_alloc();
                if (!p.avFrame2)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate aux. video frame")
                            .arg(p.fileName));
                }
                switch (videoInfo.pixelType)
                {
                case image::PixelType::L_U8:
                    p.avPixelFormatIn = AV_PIX_FMT_GRAY8;
                    break;
                case image::PixelType::RGB_U8:
                    p.avPixelFormatIn = AV_PIX_FMT_RGB24;
                    break;
                case image::PixelType::RGBA_U8:
                    p.avPixelFormatIn = AV_PIX_FMT_RGBA;
                    break;
                case image::PixelType::L_U16:
                    p.avPixelFormatIn = AV_PIX_FMT_GRAY16;
                    break;
                case image::PixelType::RGB_U16:
                    p.avPixelFormatIn = AV_PIX_FMT_RGB48;
                    break;
                case image::PixelType::RGBA_U16:
                    p.avPixelFormatIn = AV_PIX_FMT_RGBA64;
                    break;
                default:
                    throw std::runtime_error(
                        string::Format("{0}: Incompatible pixel type")
                            .arg(p.fileName));
                    break;
                }
                p.swsContext = sws_alloc_context();
                if (!p.swsContext)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate context")
                            .arg(p.fileName));
                }
                av_opt_set_defaults(p.swsContext);
                r = av_opt_set_int(
                    p.swsContext, "srcw", videoInfo.size.w,
                    AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(
                    p.swsContext, "srch", videoInfo.size.h,
                    AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(
                    p.swsContext, "src_format", p.avPixelFormatIn,
                    AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(
                    p.swsContext, "dstw", videoInfo.size.w,
                    AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(
                    p.swsContext, "dsth", videoInfo.size.h,
                    AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(
                    p.swsContext, "dst_format", p.avSwPixelFormat,
                    AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(
                    p.swsContext, "sws_flags", swsScaleFlags,
                    AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(
                    p.swsContext, "threads", 0, AV_OPT_SEARCH_CHILDREN);
                r = sws_init_context(p.swsContext, nullptr, nullptr);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot initialize sws context")
                            .arg(p.fileName));
                }

                // Handle matrices and color space details
                int in_full, out_full, brightness, contrast, saturation;
                const int *inv_table, *table;

                sws_getColorspaceDetails(
                    p.swsContext, (int**)&inv_table, &in_full, (int**)&table,
                    &out_full, &brightness, &contrast, &saturation);

                inv_table = sws_getCoefficients(p.avCodecContext->colorspace);
                table = sws_getCoefficients(AVCOL_SPC_BT709);

                // We use the full range, and we set -color_range to 2
                // ( as we set AV_COL_RANGE_JPEG )
                in_full = (p.avCodecContext->color_range == AVCOL_RANGE_JPEG);
                out_full = (p.avCodecContext->color_range == AVCOL_RANGE_JPEG);

                sws_setColorspaceDetails(
                    p.swsContext, inv_table, in_full, table, out_full,
                    brightness, contrast, saturation);
            }

            if (p.avFormatContext->nb_streams == 0)
            {
                throw std::runtime_error(
                    string::Format("{0}: No video or audio streams.")
                        .arg(p.fileName));
            }

            av_dump_format(p.avFormatContext, 0, p.fileName.c_str(), 1);

            r = avio_open(
                &p.avFormatContext->pb, p.fileName.c_str(), AVIO_FLAG_WRITE);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: avio_open - {1}")
                                             .arg(p.fileName)
                                             .arg(getErrorLabel(r)));
            }

            r = avformat_write_header(p.avFormatContext, NULL);
            if (r < 0)
            {
                throw std::runtime_error(
                    string::Format("{0}: avformat_write_header - {1}")
                        .arg(p.fileName)
                        .arg(getErrorLabel(r)));
            }

            p.opened = true;
        }

        Write::Write() :
            _p(new Private)
        {
        }

        Write::~Write()
        {
            TLRENDER_P();

            if (p.opened)
            {
                // We need to enclose this in a try block as _encode can throw
                try
                {
                    if (p.avAudioCodecContext)
                    {
                        _flushAudio();

                        _encode(
                            p.avAudioCodecContext, p.avAudioStream, nullptr,
                            p.avAudioPacket);
                    }

                    if (p.avCodecContext)
                    {
                        _encode(
                            p.avCodecContext, p.avVideoStream, nullptr,
                            p.avPacket);
                    }
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR(e.what());
                }

                int r = av_write_trailer(p.avFormatContext);
                if (r != 0)
                {
                    LOG_ERROR(
                        string::Format("{0}: avformat_write_trailer - {1}")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }
            }

            if (p.swsContext)
            {
                sws_freeContext(p.swsContext);
            }
            if (p.avHwFrame)
            {
                av_frame_free(&p.avHwFrame);
            }
            if (p.avHWFramesCtx)
            {
                av_buffer_unref(&p.avHWFramesCtx);
            }
            if (p.avHWDeviceCtx)
            {
                av_buffer_unref(&p.avHWDeviceCtx);
            }
            if (p.avFrame2)
            {
                av_frame_free(&p.avFrame2);
            }
            if (p.avFrame)
            {
                av_frame_free(&p.avFrame);
            }
            if (p.avAudioFrame)
            {
                av_frame_free(&p.avAudioFrame);
            }
            if (p.avPacket)
            {
                av_packet_free(&p.avPacket);
            }
            if (p.avAudioPacket)
            {
                av_packet_free(&p.avAudioPacket);
            }
            if (p.avAudioFifo)
            {
                av_audio_fifo_free(p.avAudioFifo);
                p.avAudioFifo = nullptr;
            }
            if (p.avAudioCodecContext)
            {
                avcodec_free_context(&p.avAudioCodecContext);
            }
            if (p.avCodecContext)
            {
                avcodec_free_context(&p.avCodecContext);
            }
            if (p.avFormatContext && p.avFormatContext->pb)
            {
                avio_closep(&p.avFormatContext->pb);
            }
            if (p.avFormatContext)
            {
                avformat_free_context(p.avFormatContext);
            }
        }

        std::shared_ptr<Write> Write::create(
            const file::Path& path, const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::writeVideo(
            const OTIO_NS::RationalTime& time,
            const std::shared_ptr<image::Image>& image, const io::Options&)
        {
            TLRENDER_P();

            const auto& info = image->getInfo();
            av_image_fill_arrays(
                p.avFrame2->data, p.avFrame2->linesize, image->getData(),
                p.avPixelFormatIn, info.size.w, info.size.h,
                info.layout.alignment);

            // Flip the image vertically.
            switch (info.pixelType)
            {
            case image::PixelType::L_U8:
            case image::PixelType::L_U16:
            case image::PixelType::RGB_U8:
            case image::PixelType::RGB_U16:
            case image::PixelType::RGBA_U8:
            case image::PixelType::RGBA_U16:
            {
                const size_t channelCount =
                    image::getChannelCount(info.pixelType);
                for (size_t i = 0; i < channelCount; i++)
                {
                    p.avFrame2->data[i] +=
                        p.avFrame2->linesize[i] * (info.size.h - 1);
                    p.avFrame2->linesize[i] = -p.avFrame2->linesize[i];
                }
                break;
            }
            case image::PixelType::YUV_420P_U8:
            case image::PixelType::YUV_422P_U8:
            case image::PixelType::YUV_444P_U8:
            case image::PixelType::YUV_420P_U16:
            case image::PixelType::YUV_422P_U16:
            case image::PixelType::YUV_444P_U16:
                //! \bug How do we flip YUV data?
                throw std::runtime_error(
                    string::Format("{0}: Incompatible pixel type")
                        .arg(p.fileName));
                break;
            default:
                throw std::runtime_error(
                    string::Format("{0}: Incompatible pixel type")
                        .arg(p.fileName));
                break;
            }

            int r = av_frame_make_writable(p.avFrame);
            if (r < 0)
            {
                throw std::runtime_error(
                    string::Format(
                        "Could not make video frame writable at time {0}.")
                        .arg(time));
            }

            sws_scale(
                p.swsContext, (uint8_t const* const*)p.avFrame2->data,
                p.avFrame2->linesize, 0, p.avVideoStream->codecpar->height,
                p.avFrame->data, p.avFrame->linesize);

            const auto timeRational = time::toRational(p.avSpeed);
            p.avFrame->pts = av_rescale_q(
                time.value() - p.videoStartTime.value(),
                {timeRational.second, timeRational.first},
                p.avVideoStream->time_base);

            auto hdrData = image->getHDR();
            if (hdrData)
            {
                p.hasHDR = true;
                p.hdr = *hdrData;

                if (string::toLower(p.path.getExtension()) != ".mkv")
                {
                    throw std::runtime_error(
                            "Saving videos with HDR data per frame "
                            "requires VPX and a .mkv container");
                }
            }
            else
            {
                p.hasHDR = false;
            }

            if (p.useVAAPI || p.useD3D12VA)
            {
                const char* hwName = p.useD3D12VA ? "Direct3D 12" : "VAAPI";

                av_frame_unref(p.avHwFrame);
                r = av_hwframe_get_buffer(p.avHWFramesCtx, p.avHwFrame, 0);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format(
                            "{0}: Cannot get {1} hw frame buffer - {2}")
                            .arg(p.fileName)
                            .arg(hwName)
                            .arg(getErrorLabel(r)));
                }

                r = av_hwframe_transfer_data(p.avHwFrame, p.avFrame, 0);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format(
                            "{0}: Cannot upload frame to {1} surface "
                            "- {2}")
                            .arg(p.fileName)
                            .arg(hwName)
                            .arg(getErrorLabel(r)));
                }
                p.avHwFrame->pts = p.avFrame->pts;

                _encode(
                    p.avCodecContext, p.avVideoStream, p.avHwFrame,
                    p.avPacket);
            }
            else
            {
                _encode(
                    p.avCodecContext, p.avVideoStream, p.avFrame,
                    p.avPacket);
            }
        }

        void Write::writeAudio(
            const OTIO_NS::TimeRange& inTimeRange,
            const std::shared_ptr<audio::Audio>& audioIn, const io::Options&)
        {
            TLRENDER_P();

            if (!audioIn || !p.avAudioFifo || audioIn->getSampleCount() == 0)
                return;

            const auto& info = audioIn->getInfo();
            if (!info.isValid())
                return;


            int r = 0;
            const auto timeRange = OTIO_NS::TimeRange(
                inTimeRange.start_time().rescaled_to(p.sampleRate),
                inTimeRange.duration().rescaled_to(p.sampleRate));

            int fifoSize = av_audio_fifo_size(p.avAudioFifo);

            if (timeRange.start_time().value() >=
                p.totalSamples + p.audioStartSamples + fifoSize)
            {
                // If this is the start of the saving, store the start time.
                if (p.totalSamples == 0)
                {
                    p.audioStartSamples = timeRange.start_time().value();
                }
                else
                {
                    // // Warn if there's a gap in the audio timeline
                    // LOG_WARNING(
                    //     string::Format("Audio gap detected at {0} samples")
                    //     .arg(timeRange.start_time().value()));
                }

                auto audioResampled = audioIn;
                // Resample audio
                if (p.resample)
                {
                    audioResampled = p.resample->process(audioIn);
                }

                // Most codecs need non-interleaved audio.
                std::shared_ptr<audio::Audio> audio;
                if (p.avAudioPlanar)
                    audio = planarDeinterleave(audioResampled);
                else
                    audio = audioResampled;

                uint8_t* data = audio->getData();

                if (p.avAudioPlanar)
                {
                    const size_t channels = audio->getChannelCount();
                    const size_t stride = audio->getByteCount() / channels;
                    for (size_t i = 0; i < channels; ++i)
                    {
                        p.flatPointers[i] = data + i * stride;
                    }
                }
                else
                {
                    p.flatPointers[0] = data;
                }

                const size_t sampleCount = audio->getSampleCount();

                r = av_audio_fifo_write(
                    p.avAudioFifo,
                    reinterpret_cast<void**>(p.flatPointers.data()),
                    sampleCount);

                fifoSize = av_audio_fifo_size(p.avAudioFifo);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("Could not write to fifo buffer at {0}.")
                            .arg(timeRange));
                }
                if (r != sampleCount)
                {
                    throw std::runtime_error(
                        string::Format(
                            "Could not write all samples fifo buffer at {0}.")
                            .arg(timeRange));
                }
            }

            const AVRational ratio = {1, p.avAudioCodecContext->sample_rate};

            const int frameSize = p.avAudioCodecContext->frame_size;
            while (fifoSize >= frameSize)
            {
                r = av_frame_make_writable(p.avAudioFrame);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format(
                            "Could not make audio frame writable at time {0}.")
                            .arg(timeRange));
                }

                r = av_audio_fifo_read(
                    p.avAudioFifo,
                    reinterpret_cast<void**>(p.avAudioFrame->extended_data),
                    frameSize);

                fifoSize = av_audio_fifo_size(p.avAudioFifo);

                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("Could not read from audio fifo buffer "
                                       "at {0}.")
                            .arg(timeRange));
                }

                p.avAudioFrame->pts = av_rescale_q(
                    p.totalSamples, ratio, p.avAudioCodecContext->time_base);
                p.avAudioFrame->duration = av_rescale_q(
                    frameSize, ratio, p.avAudioCodecContext->time_base);

                _encode(
                    p.avAudioCodecContext, p.avAudioStream, p.avAudioFrame,
                    p.avAudioPacket);

                p.totalSamples += frameSize;
            }
        }

        void Write::_flushAudio()
        {
            TLRENDER_P();

            // If FIFO still has some data, send it
            const AVRational ratio = {1, p.avAudioCodecContext->sample_rate};
            int fifoSize = av_audio_fifo_size(p.avAudioFifo);
            if (fifoSize > 0)
            {
                int r = av_frame_make_writable(p.avAudioFrame);
                if (r < 0)
                {
                    LOG_ERROR("Could not make p.avAudioFrame writable");
                    return;
                }

                int frameSize = fifoSize;
                if (p.avAudioCodecContext->codec_id != AV_CODEC_ID_PCM_S16LE &&
                    p.avAudioCodecContext->frame_size > 0)
                {
                    frameSize = std::min(fifoSize, p.avAudioCodecContext->frame_size);
                }
                p.avAudioFrame->nb_samples = frameSize;
                        r = av_audio_fifo_read(
                    p.avAudioFifo,
                    reinterpret_cast<void**>(p.avAudioFrame->extended_data),
                    fifoSize);
                if (r < 0)
                {
                    LOG_ERROR("Could not read from fifo at end");
                    return;
                }

                p.avAudioFrame->pts = av_rescale_q(
                    p.totalSamples, ratio, p.avAudioCodecContext->time_base);

                _encode(
                    p.avAudioCodecContext, p.avAudioStream, p.avAudioFrame,
                    p.avAudioPacket);

            }
        }
        void Write::_attach_stream_hdr_metadata(AVStream* stream)
        {
            TLRENDER_P();

            if (!stream || !p.hasHDR)
                return;

            AVCodecParameters* par = stream->codecpar;

            // --- Mastering display metadata ---
            AVPacketSideData* sd = av_packet_side_data_new(
                &par->coded_side_data, &par->nb_coded_side_data,
                AV_PKT_DATA_MASTERING_DISPLAY_METADATA,
                sizeof(AVMasteringDisplayMetadata), 0);
            if (sd && sd->data)
            {
                AVMasteringDisplayMetadata* mdm =
                    (AVMasteringDisplayMetadata*)sd->data;
                memset(mdm, 0, sizeof(*mdm));

                const float rx = p.hdr.primaries[0].x, ry = p.hdr.primaries[0].y;
                const float gx = p.hdr.primaries[1].x, gy = p.hdr.primaries[1].y;
                const float bx = p.hdr.primaries[2].x, by = p.hdr.primaries[2].y;
                const float wx = p.hdr.primaries[3].x, wy = p.hdr.primaries[3].y;

                mdm->display_primaries[0][0] = av_d2q(rx, 100000);
                mdm->display_primaries[0][1] = av_d2q(ry, 100000);
                mdm->display_primaries[1][0] = av_d2q(gx, 100000);
                mdm->display_primaries[1][1] = av_d2q(gy, 100000);
                mdm->display_primaries[2][0] = av_d2q(bx, 100000);
                mdm->display_primaries[2][1] = av_d2q(by, 100000);
                mdm->white_point[0] = av_d2q(wx, 100000);
                mdm->white_point[1] = av_d2q(wy, 100000);
                mdm->has_primaries = 1;

                float min_lum = p.hdr.displayMasteringLuminance.getMin();
                if (min_lum <= 0.F)
                    min_lum = 1.F;
                float max_lum = p.hdr.displayMasteringLuminance.getMax();

                mdm->max_luminance = av_d2q(max_lum, 10000);
                mdm->min_luminance = av_d2q(min_lum, 10000);
                mdm->has_luminance = 1;
            }

            // --- Content light level metadata ---
            sd = av_packet_side_data_new(
                &par->coded_side_data, &par->nb_coded_side_data,
                AV_PKT_DATA_CONTENT_LIGHT_LEVEL,
                sizeof(AVContentLightMetadata), 0);
            if (sd && sd->data)
            {
                AVContentLightMetadata* clm = (AVContentLightMetadata*)sd->data;
                clm->MaxCLL = p.hdr.maxCLL;
                clm->MaxFALL = p.hdr.maxFALL;
            }
        }

        //! Set video header metadata.
        void Write::setHDR(const image::HDRData& hdrData)
        {
            TLRENDER_P();

            p.hasHDR = true;
            p.hdr = hdrData;
        }

        void Write::_attach_frame_hdr_metadata(AVFrame *frame)
        {
            TLRENDER_P();

            AVFrameSideData* sd = nullptr;

            // --- Mastering display metadata ---
            // Remove any existing side data first: since `frame` is reused
            // across calls, an existing side-data buffer may still be
            // referenced (not exclusively owned) by the encoder for a
            // previously-sent frame. Mutating it in place would corrupt
            // that earlier frame's metadata. Always allocate a fresh,
            // independently-owned buffer instead.
            av_frame_remove_side_data(frame, AV_FRAME_DATA_MASTERING_DISPLAY_METADATA);
            sd = av_frame_new_side_data(frame,
                                        AV_FRAME_DATA_MASTERING_DISPLAY_METADATA,
                                        sizeof(AVMasteringDisplayMetadata));

            if (sd && sd->data)
            {
                AVMasteringDisplayMetadata *mdm =
                    (AVMasteringDisplayMetadata*) sd->data;
                memset(mdm, 0, sizeof(*mdm));

                const float rx = p.hdr.primaries[0].x;
                const float ry = p.hdr.primaries[0].y;

                const float gx = p.hdr.primaries[1].x;
                const float gy = p.hdr.primaries[1].y;

                const float bx = p.hdr.primaries[2].x;
                const float by = p.hdr.primaries[2].y;

                const float wx = p.hdr.primaries[3].x;
                const float wy = p.hdr.primaries[3].y;

                // Convert to rationals
                mdm->display_primaries[0][0] = av_d2q(rx, 100000); // Rx
                mdm->display_primaries[0][1] = av_d2q(ry, 100000); // Ry

                mdm->display_primaries[1][0] = av_d2q(gx, 100000); // Gx
                mdm->display_primaries[1][1] = av_d2q(gy, 100000); // Gy

                mdm->display_primaries[2][0] = av_d2q(bx, 100000); // Bx
                mdm->display_primaries[2][1] = av_d2q(by, 100000); // By

                mdm->white_point[0] = av_d2q(wx, 100000);
                mdm->white_point[1] = av_d2q(wy, 100000);

                mdm->has_primaries = 1;

                float min_lum = p.hdr.displayMasteringLuminance.getMin();
                if (min_lum <= 0.F)
                    min_lum = 1.F;
                float max_lum = p.hdr.displayMasteringLuminance.getMax();

                mdm->max_luminance = av_d2q(max_lum, 10000);
                mdm->min_luminance = av_d2q(min_lum, 10000);

                mdm->has_luminance = 1;
            }

            // --- Content light level metadata ---
            // See note above: always drop + recreate rather than reuse.
            av_frame_remove_side_data(frame, AV_FRAME_DATA_CONTENT_LIGHT_LEVEL);
            sd = av_frame_new_side_data(
                frame, AV_FRAME_DATA_CONTENT_LIGHT_LEVEL,
                sizeof(AVContentLightMetadata));
            if (sd && sd->data)
            {
                AVContentLightMetadata *clm = (AVContentLightMetadata*)
                                              sd->data;
                clm->MaxCLL  = p.hdr.maxCLL; // peak per-pixel light level
                clm->MaxFALL = p.hdr.maxFALL;  // frame average light level
            }

            // --- HDR 10+ metadata ---
            if (p.hdr.sceneMax[0] > 0.F)
            {
                // See note above: always drop + recreate rather than reuse.
                av_frame_remove_side_data(frame, AV_FRAME_DATA_DYNAMIC_HDR_PLUS);
                sd = av_frame_new_side_data(
                    frame, AV_FRAME_DATA_DYNAMIC_HDR_PLUS,
                    sizeof(AVDynamicHDRPlus));
                if (sd && sd->data)
                {
                    AVDynamicHDRPlus *data = (AVDynamicHDRPlus*) sd->data;
                    // Zero out the struct to prevent junk data on reuse
                    memset(data, 0, sizeof(AVDynamicHDRPlus));

                    data->application_version = 1;
                    data->num_windows = 1; // Required for params to be valid

                    AVHDRPlusColorTransformParams *p_params = data->params;

                    // Scale values back down by 10000 and map to rationals
                    p_params->maxscl[0] = av_d2q(p.hdr.sceneMax[0] / 10000.F, 100000);
                    p_params->maxscl[1] = av_d2q(p.hdr.sceneMax[1] / 10000.F, 100000);
                    p_params->maxscl[2] = av_d2q(p.hdr.sceneMax[2] / 10000.F, 100000);
                    p_params->average_maxrgb = av_d2q(p.hdr.sceneAvg / 10000.F, 100000);

                    // The reader only kept the highest percentile as a fallback max.
                    // We leave distribution blocks at 0 to avoid writing fabricated data.
                    p_params->num_distribution_maxrgb_percentiles = 0;

                    // Tone mapping
                    if (p.hdr.ootf.numAnchors > 0)
                    {
                        p_params->tone_mapping_flag = 1;
                        data->targeted_system_display_maximum_luminance =
                            av_d2q(p.hdr.ootf.targetLuma, 10000);
                        p_params->knee_point_x = av_d2q(p.hdr.ootf.kneeX, 10000);
                        p_params->knee_point_y = av_d2q(p.hdr.ootf.kneeY, 10000);

                        p_params->num_bezier_curve_anchors = p.hdr.ootf.numAnchors;
                        for (int i = 0; i < p.hdr.ootf.numAnchors && i < 15; i++)
                        {
                            p_params->bezier_curve_anchors[i] = av_d2q(p.hdr.ootf.anchors[i], 10000);
                        }
                    }
                    else
                    {
                        p_params->tone_mapping_flag = 0;
                    }

                    // FFmpeg 9.0.1 new parameters.
                    // For now, set them to 1 to avoid division by zero
                    // errors.
                    p_params->fraction_bright_pixels.num = 1;
                    p_params->fraction_bright_pixels.den = 1;
                    p_params->color_saturation_mapping_flag = 0;
                    p_params->color_saturation_weight.num = 1;
                    p_params->color_saturation_weight.den = 1;
                }
            }

            // --- Dolby Vision metadata ---
#ifdef TLRENDER_DOVI
            if (p.hdr.isDolbyVision)
            {
                size_t dovi_size = 0;
                // av_dovi_metadata_alloc respects the internal ABI limits of FFmpeg for DOVI
                AVDOVIMetadata *dovi_alloc = av_dovi_metadata_alloc(&dovi_size);
                if (dovi_alloc)
                {
                    // See note above: always drop + recreate rather than reuse.
                    av_frame_remove_side_data(frame, AV_FRAME_DATA_DOVI_METADATA);
                    sd = av_frame_new_side_data(frame, AV_FRAME_DATA_DOVI_METADATA, dovi_size);
                    if (sd && sd->data)
                    {
                        // Copy the properly zeroed/allocated flat struct into the side data memory
                        std::memcpy(sd->data, dovi_alloc, dovi_size);
                        AVDOVIMetadata *metadata = (AVDOVIMetadata*)sd->data;

                        AVDOVIRpuDataHeader *header = av_dovi_get_header(metadata);
                        if (header) {
                            header->disable_residual_flag = 1;
                        }

                        AVDOVIColorMetadata *color = av_dovi_get_color(metadata);
                        if (color) {
                            // \@todo: To properly reverse `source_min_pq` & `source_max_pq`,
                            // you need the mathematical inverse of your `dolby_rescale` function.
                            //
                            // Example:
                            // color->source_min_pq = (uint16_t)(dolby_inverse_rescale(p.hdr.displayMasteringLuminance.getMin()) * 4095.0f);
                            // color->source_max_pq = (uint16_t)(dolby_inverse_rescale(p.hdr.displayMasteringLuminance.getMax()) * 4095.0f);
                        }
                    }
                    av_free(dovi_alloc);
                }
            }
#endif

        }

        void Write::_encode(
            AVCodecContext* context, const AVStream* stream,
            AVFrame* frame, AVPacket* packet)
        {
            TLRENDER_P();

            if (p.hasHDR && frame && p.avVideoStream == stream)
            {
                _attach_frame_hdr_metadata(frame);
            }

            int r = avcodec_send_frame(context, frame);
            if (r < 0)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot send frame - {1}")
                        .arg(p.fileName)
                        .arg(getErrorLabel(r)));
            }

            while (r >= 0)
            {
                r = avcodec_receive_packet(context, packet);

                if (r == AVERROR_EOF || r == AVERROR(EAGAIN))
                {
                    return;
                }
                else if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot receive packet - {1}")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                packet->stream_index = stream->index; // Needed

                r = av_interleaved_write_frame(p.avFormatContext, packet);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot write frame - {1}")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }
                av_packet_unref(packet);
            }
        }

    } // namespace ffmpeg
} // namespace tl

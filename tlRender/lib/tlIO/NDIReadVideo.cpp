// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024 Gonzalo Garramuño
// All rights reserved.

#include <tlIO/FFmpegMacros.h>
#include <tlIO/NDIReadPrivate.h>

#include <tlDevice/NDI/NDIUtil.h>

#include <tlCore/StringFormat.h>

extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <rapidxml/rapidxml.hpp>

namespace
{
    const char* kModule = "ndi";
}

namespace tl
{
    namespace ndi
    {
        namespace
        {

            bool canCopy(AVPixelFormat in, AVPixelFormat out)
            {
                return (
                    in == out &&
                    (AV_PIX_FMT_RGBA64 == in || AV_PIX_FMT_RGB24 == in ||
                     AV_PIX_FMT_RGBA == in || AV_PIX_FMT_YUV420P == in));
            }
        } // namespace

        ReadVideo::ReadVideo(
            const std::string& fileName, const NDIlib_source_t& NDIsource,
            const NDIlib_recv_create_t& recv_desc,
            const NDIlib_video_frame_t& v,
            const std::weak_ptr<log::System>& logSystem,
            const Options& options) :
            _fileName(fileName),
            _logSystem(logSystem),
            _options(options)
        {
            _tags["otioClipName"] = _fileName;
            double fps = v.frame_rate_N / static_cast<double>(v.frame_rate_D);
            double startTime = 0.0;
            double lastTime = kNDI_MOVIE_DURATION * fps;
            _timeRange = otime::TimeRange(
                otime::RationalTime(startTime, fps),
                otime::RationalTime(lastTime, fps));

            NDI_recv = NDIlib_recv_create(&recv_desc);
            if (!NDI_recv)
                throw std::runtime_error("Could not create NDI audio receiver");

            _from_ndi(v);
        }

        ReadVideo::~ReadVideo()
        {
            if (_swsContext)
            {
                sws_freeContext(_swsContext);
            }
            if (_avFrame2)
            {
                av_frame_free(&_avFrame2);
            }
            if (_avFrame)
            {
                av_frame_free(&_avFrame);
            }

            stop();
        }

        const bool ReadVideo::isValid() const
        {
            return NDI_recv;
        }

        const image::Info& ReadVideo::getInfo() const
        {
            return _info;
        }

        const otime::TimeRange& ReadVideo::getTimeRange() const
        {
            return _timeRange;
        }

        void ReadVideo::_from_ndi(const NDIlib_video_frame_t& video_frame)
        {

            bool init = false;

            float pixelAspectRatio = 1.F;
            if (video_frame.picture_aspect_ratio == 0.F)
                pixelAspectRatio = 1.F / video_frame.xres * video_frame.yres;

            if (_info.size.w != video_frame.xres ||
                _info.size.h != video_frame.yres ||
                _info.size.pixelAspectRatio != pixelAspectRatio)
            {
                init = true;
            }

            _info.size.w = video_frame.xres;
            _info.size.h = video_frame.yres;
            _info.size.pixelAspectRatio = pixelAspectRatio;
            _info.layout.mirror.y = true;
            _info.videoLevels = image::VideoLevels::FullRange;

            std::string transferName, matrixName, primariesName;
            if (video_frame.p_metadata)
            {
                rapidxml::xml_document<> doc;
                doc.parse<0>((char*)video_frame.p_metadata);

                // Get root node
                rapidxml::xml_node<>* root = doc.first_node("ndi_color_info");

                // Get attributes
                rapidxml::xml_attribute<>* attr_transfer =
                    root->first_attribute("transfer");
                rapidxml::xml_attribute<>* attr_matrix =
                    root->first_attribute("matrix");
                rapidxml::xml_attribute<>* attr_primaries =
                    root->first_attribute("primaries");
                transferName = attr_transfer->value();
                matrixName = attr_matrix->value();
                primariesName = attr_primaries->value();
            }

            if (_ndiFourCC != video_frame.FourCC ||
                _ndiStride != video_frame.line_stride_in_bytes ||
                _ndiPrimariesName != primariesName ||
                _ndiTransferName != transferName ||
                _ndiMatrixName != matrixName)
            {
                init = true;
                _ndiPrimariesName = primariesName;
                _ndiTransferName = transferName;
                _ndiMatrixName = matrixName;
                _ndiFourCC = video_frame.FourCC;
                _ndiStride = video_frame.line_stride_in_bytes;

                std::string msg =
                    string::Format("NDI stream is {0}.")
                        .arg(ndi::FourCCString(video_frame.FourCC));
                LOG_STATUS(msg);

                switch (video_frame.FourCC)
                {
                case NDIlib_FourCC_type_UYVY:
                    // YCbCr color space packed, not planar using 4:2:2. (works)
                    _avInputPixelFormat = AV_PIX_FMT_UYVY422;
                    _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                    _info.pixelType = image::PixelType::RGB_U8;
                    break;
                case NDIlib_FourCC_type_UYVA:
                    // YCbCr color space packed, not planar using 4:2:2:4.
                    _avInputPixelFormat = AV_PIX_FMT_UYVY422;
                    _avOutputPixelFormat = AV_PIX_FMT_RGBA;
                    _info.pixelType = image::PixelType::RGBA_U8;
                    break;
                case NDIlib_FourCC_type_P216:
                    // This is a 16bpp version of NV12 (semi-planar 4:2:2).
                    _avInputPixelFormat = AV_PIX_FMT_P216LE;
                    _avOutputPixelFormat = AV_PIX_FMT_YUV422P16LE;
                    _info.pixelType = image::PixelType::YUV_422P_U16;
                    // _avOutputPixelFormat = AV_PIX_FMT_RGB48;
                    // _info.pixelType = image::PixelType::RGB_U16;
                    break;
                case NDIlib_FourCC_type_PA16:
                    // This is 4:2:2:4 in 16bpp.
                    _avInputPixelFormat = AV_PIX_FMT_RGBA64;
                    _avOutputPixelFormat = AV_PIX_FMT_RGBA64;
                    _info.pixelType = image::PixelType::RGBA_U16;
                    break;
                case NDIlib_FourCC_type_YV12:
                    // Planar 8bit 4:2:0 YUV video format.
                    _avInputPixelFormat = AV_PIX_FMT_YUV420P;
                    _info.pixelType = image::PixelType::YUV_420P_U8;
                    _avOutputPixelFormat = _avInputPixelFormat;
                    break;
                case NDIlib_FourCC_type_RGBA:
                    _avInputPixelFormat = AV_PIX_FMT_RGBA;
                    _info.pixelType = image::PixelType::RGBA_U8;
                    _avOutputPixelFormat = _avInputPixelFormat;
                    break;
                case NDIlib_FourCC_type_BGRA:
                    _avInputPixelFormat = AV_PIX_FMT_BGRA;
                    _avOutputPixelFormat = AV_PIX_FMT_RGBA;
                    _info.pixelType = image::PixelType::RGBA_U8;
                    break;
                case NDIlib_FourCC_type_RGBX:
                    _avInputPixelFormat = AV_PIX_FMT_RGB24;
                    _info.pixelType = image::PixelType::RGB_U8;
                    _avOutputPixelFormat = _avInputPixelFormat;
                    break;
                case NDIlib_FourCC_type_BGRX:
                    _avInputPixelFormat = AV_PIX_FMT_BGR24;
                    _info.pixelType = image::PixelType::RGB_U8;
                    _avOutputPixelFormat = AV_PIX_FMT_RGB24;
                    break;
                case NDIlib_FourCC_type_I420:
                    // @todo: Not tested yet, this is 4:2:0 YUV with UV reversed
                    LOG_WARNING("I420 pixel format not tested");
                    _avInputPixelFormat = AV_PIX_FMT_YUV420P;
                    _avOutputPixelFormat = _avInputPixelFormat;
                    _info.pixelType = image::PixelType::YUV_420P_U8;
                    break;
                default:
                    throw std::runtime_error("Unsupported pixel type");
                }

                image::HDRData hdrData;
                image::EOTFType eotf = image::EOTF_BT709;
                if (transferName == "bt_601")
                    eotf = image::EOTF_BT601;
                else if (transferName == "bt_2020")
                {
                    eotf = image::EOTF_BT2020;
                }
                else if (transferName == "bt_2100_hlg")
                {
                    eotf = image::EOTF_BT2100_HLG;
                }
                else if (transferName == "bt_2100_pq")
                {
                    eotf = image::EOTF_BT2100_PQ;
                }

                // Stablish primaries
                switch (eotf)
                {
                case image::EOTFType::EOTF_BT601:
                    // This is just for NTSC
                    hdrData.primaries[image::HDRPrimaries::Red].x = 0.630F;
                    hdrData.primaries[image::HDRPrimaries::Red].y = 0.340F;
                    hdrData.primaries[image::HDRPrimaries::Green].x = 0.310F;
                    hdrData.primaries[image::HDRPrimaries::Green].y = 0.595F;
                    hdrData.primaries[image::HDRPrimaries::Blue].x = 0.155F;
                    hdrData.primaries[image::HDRPrimaries::Blue].y = 0.070F;
                    hdrData.primaries[image::HDRPrimaries::White].x = 0.3127F;
                    hdrData.primaries[image::HDRPrimaries::White].y = 0.3290F;
                    break;
                case image::EOTFType::EOTF_BT709:
                    hdrData.primaries[image::HDRPrimaries::Red].x = 0.640F;
                    hdrData.primaries[image::HDRPrimaries::Red].y = 0.330F;
                    hdrData.primaries[image::HDRPrimaries::Green].x = 0.300F;
                    hdrData.primaries[image::HDRPrimaries::Green].y = 0.600F;
                    hdrData.primaries[image::HDRPrimaries::Blue].x = 0.150F;
                    hdrData.primaries[image::HDRPrimaries::Blue].y = 0.060F;
                    hdrData.primaries[image::HDRPrimaries::White].x = 0.3127F;
                    hdrData.primaries[image::HDRPrimaries::White].y = 0.3290F;
                    break;
                case image::EOTFType::EOTF_BT2020:
                case image::EOTFType::EOTF_BT2100_HLG:
                case image::EOTFType::EOTF_BT2100_PQ:
                    hdrData.primaries[image::HDRPrimaries::Red].x = 0.708F;
                    hdrData.primaries[image::HDRPrimaries::Red].y = 0.292F;
                    hdrData.primaries[image::HDRPrimaries::Green].x = 0.170F;
                    hdrData.primaries[image::HDRPrimaries::Green].y = 0.797F;
                    hdrData.primaries[image::HDRPrimaries::Blue].x = 0.131F;
                    hdrData.primaries[image::HDRPrimaries::Blue].y = 0.046F;
                    hdrData.primaries[image::HDRPrimaries::White].x = 0.3127F;
                    hdrData.primaries[image::HDRPrimaries::White].y = 0.3290F;
                    break;
                default:
                    break;
                }

                hdrData.eotf = eotf;
                bool hasHDR = eotf > image::EOTF_BT709;
                if (hasHDR)
                {
                    _tags["hdr"] = nlohmann::json(hdrData).dump();
                }
            }

            if (init)
            {
                start();
            }

            av_image_fill_arrays(
                _avFrame->data, _avFrame->linesize, video_frame.p_data,
                _avInputPixelFormat, _info.size.w, _info.size.h, 1);

            auto image = image::Image::create(_info);
            image->setTags(_tags);
            _copy(image);
            _buffer.push_back(image);
        }

        bool ReadVideo::process(const otime::RationalTime& currentTime)
        {
            bool out = true;

            if (_buffer.size() < _options.videoBufferSize)
            {
                int decoding = _decode(currentTime);
                if (decoding < 0)
                    LOG_ERROR("Error decoding video stream");
                out = false;
            }

            return out;
        }

        int ReadVideo::_decode(const otime::RationalTime& time)
        {
            int out = 0;
            NDIlib_video_frame_t video_frame;
            NDIlib_frame_type_e type;

            {
                std::stringstream ss;
                ss << time;
                _tags["otioClipTime"] = ss.str();
            }

            while (out == 0 && NDI_recv)
            {
                type = NDIlib_recv_capture(
                    NDI_recv, &video_frame, nullptr, nullptr, 50);
                if (type == NDIlib_frame_type_error)
                {
                    out = -1;
                }
                else if (type == NDIlib_frame_type_video)
                {
                    _from_ndi(video_frame);
                    NDIlib_recv_free_video(NDI_recv, &video_frame);
                    out = 1;
                }
                else if (type == NDIlib_frame_type_status_change)
                {
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

        void ReadVideo::stop()
        {
            if (NDI_recv)
                NDIlib_recv_destroy(NDI_recv);
            NDI_recv = nullptr;
        }

        void ReadVideo::start()
        {
            if (_avFrame)
            {
                av_frame_free(&_avFrame);
            }
            if (!_avFrame)
            {
                _avFrame = av_frame_alloc();
                if (!_avFrame)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate frame")
                            .arg(_fileName));
                }
            }

            if (!canCopy(_avInputPixelFormat, _avOutputPixelFormat))
            {
                if (_avFrame2)
                {
                    av_frame_free(&_avFrame2);
                }

                _avFrame2 = av_frame_alloc();
                if (!_avFrame2)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate frame")
                            .arg(_fileName));
                }

                if (_swsContext)
                {
                    sws_freeContext(_swsContext);
                }

                _swsContext = sws_alloc_context();
                if (!_swsContext)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate context")
                            .arg(_fileName));
                }
                av_opt_set_defaults(_swsContext);
                int r = av_opt_set_int(
                    _swsContext, "srcw", _info.size.w, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(
                    _swsContext, "srch", _info.size.h, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(
                    _swsContext, "src_format", _avInputPixelFormat,
                    AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(
                    _swsContext, "dstw", _info.size.w, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(
                    _swsContext, "dsth", _info.size.h, AV_OPT_SEARCH_CHILDREN);
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

                // Handle matrices and color space details
                int in_full, out_full, brightness, contrast, saturation;
                const int *inv_table, *table;

                sws_getColorspaceDetails(
                    _swsContext, (int**)&inv_table, &in_full, (int**)&table,
                    &out_full, &brightness, &contrast, &saturation);

                if (_info.size.w > 1920 || _info.size.h > 1080)
                {
                    inv_table = sws_getCoefficients(SWS_CS_BT2020);
                }
                else if (_info.size.w > 720 || _info.size.h > 576)
                {
                    inv_table = sws_getCoefficients(SWS_CS_ITU709);
                }
                else
                {
                    inv_table = sws_getCoefficients(SWS_CS_ITU601);
                }

                table = sws_getCoefficients(SWS_CS_ITU709);

                in_full = 0;
                out_full = 1;

                sws_setColorspaceDetails(
                    _swsContext, inv_table, in_full, table, out_full,
                    brightness, contrast, saturation);
            }
        }

        void ReadVideo::_copy(std::shared_ptr<image::Image>& image)
        {
            const auto& info = image->getInfo();
            const std::size_t w = info.size.w;
            const std::size_t h = info.size.h;
            uint8_t* const data = image->getData();
            if (canCopy(_avInputPixelFormat, _avOutputPixelFormat))
            {
                const uint8_t* const data0 = _avFrame->data[0];
                const int linesize0 = _avFrame->linesize[0];
                switch (_avInputPixelFormat)
                {
                case AV_PIX_FMT_RGB24:
                    for (std::size_t i = 0; i < h; ++i)
                    {
                        std::memcpy(
                            data + w * 3 * i, data0 + linesize0 * 3 * i, w * 3);
                    }
                    break;
                case AV_PIX_FMT_RGBA:
                    for (std::size_t i = 0; i < h; ++i)
                    {
                        std::memcpy(
                            data + w * 4 * i, data0 + linesize0 * 4 * i, w * 4);
                    }
                    break;
                case AV_PIX_FMT_RGBA64:
                    if (_ndiFourCC == NDIlib_FourCC_type_PA16)
                    {
                        uint16_t* p_y = (uint16_t*)data0;
                        const uint16_t* p_uv = p_y + w * h;
                        const uint16_t* p_alpha = p_uv + w * h;
                        uint16_t* rgba = (uint16_t*)data;

                        // Determine BT.601 or BT.709 based on resolution
                        bool useBT709 = (w >= 1280 && h >= 720);

                        // BT.601 coefficients
                        int Kr601 = 299;                  // 0.299 * 1000
                        int Kb601 = 114;                  // 0.114 * 1000
                        int Kg601 = 1000 - Kr601 - Kb601; // 0.587 * 1000

                        // BT.709 coefficients
                        int Kr709 = 2126;                  // 0.2126 * 10000
                        int Kb709 = 722;                   // 0.0722 * 10000
                        int Kg709 = 10000 - Kr709 - Kb709; // 0.7152 * 10000

                        for (int y = 0; y < h; ++y)
                        {
                            for (int x = 0; x < w; ++x)
                            {
                                const int yw = y * w;
                                const int index_y = yw + x;
                                const int index_uv =
                                    yw +
                                    (x / 2) * 2; // UV is subsampled (4:2:2)
                                const int index_alpha = index_y;

                                // Extract Y, U, V, and Alpha
                                int Y = p_y[index_y];
                                int U =
                                    (p_uv[index_uv] -
                                     32768); // Center U around 0
                                int V =
                                    (p_uv[index_uv + 1] -
                                     32768); // Center V around 0
                                int A = p_alpha[index_alpha];

                                // Convert YUV to RGB
                                int R, G, B;

                                if (useBT709)
                                {
                                    R = (Y * 10000 + V * 14746) / 10000;
                                    G = (Y * 10000 - U * 3363 - V * 6140) /
                                        10000;
                                    B = (Y * 10000 + U * 17933) / 10000;
                                }
                                else
                                {
                                    R = (Y * 1000 + V * 1402) / 1000;
                                    G = (Y * 1000 - U * 344 - V * 714) / 1000;
                                    B = (Y * 1000 + U * 1772) / 1000;
                                }

                                // Clamp values to valid U16 range
                                R = std::clamp(R, 0, 65535);
                                G = std::clamp(G, 0, 65535);
                                B = std::clamp(B, 0, 65535);
                                A = std::clamp(A, 0, 65535);

                                // Store as RGBA_U16
                                int rgba_index = (yw + x) * 4;
                                rgba[rgba_index] = static_cast<uint16_t>(R);
                                rgba[rgba_index + 1] = static_cast<uint16_t>(G);
                                rgba[rgba_index + 2] = static_cast<uint16_t>(B);
                                rgba[rgba_index + 3] = static_cast<uint16_t>(A);
                            }
                        }
                    }
                    else
                    {
                        for (std::size_t i = 0; i < h; ++i)
                        {
                            std::memcpy(
                                data + w * 8 * i, data0 + linesize0 * 8 * i,
                                w * 8);
                        }
                    }
                    break;
                case AV_PIX_FMT_YUV420P:
                {
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    const uint8_t* const data1 = _avFrame->data[1];
                    const uint8_t* const data2 = _avFrame->data[2];
                    const int linesize1 = _avFrame->linesize[1];
                    const int linesize2 = _avFrame->linesize[2];
                    for (std::size_t i = 0; i < h; ++i)
                    {
                        std::memcpy(data + w * i, data0 + linesize0 * i, w);
                    }

                    if (_ndiFourCC != NDIlib_FourCC_type_I420)
                    {
                        for (std::size_t i = 0; i < h2; ++i)
                        {
                            std::memcpy(
                                data + (w * h) + w2 * i, data1 + linesize1 * i,
                                w2);
                            std::memcpy(
                                data + (w * h) + (w2 * h2) + w2 * i,
                                data2 + linesize2 * i, w2);
                        }
                    }
                    else
                    {
                        for (std::size_t i = 0; i < h2; ++i)
                        {
                            std::memcpy(
                                data + (w * h) + w2 * i, data2 + linesize1 * i,
                                w2);
                            std::memcpy(
                                data + (w * h) + (w2 * h2) + w2 * i,
                                data1 + linesize2 * i, w2);
                        }
                    }
                    break;
                }
                default:
                    break;
                }
            }
            else
            {
                // Fill destination avFrame
                av_image_fill_arrays(
                    _avFrame2->data, _avFrame2->linesize, data,
                    _avOutputPixelFormat, w, h, 1);

                // Do some NDI conversion that FFmpeg does not support.
                // I420 is YUV with U an V planar planes swapped.
                if (_ndiFourCC == NDIlib_FourCC_type_I420)
                {
                    const size_t tmp = _avFrame->linesize[1];
                    _avFrame->linesize[1] = _avFrame->linesize[2];
                    _avFrame->linesize[2] = tmp;
                }

                // Do the conversion with FFmpeg
                sws_scale(
                    _swsContext, (uint8_t const* const*)_avFrame->data,
                    _avFrame->linesize, 0, h, _avFrame2->data,
                    _avFrame2->linesize);

                //
                // Extract the A of UYVA manually, as FFmpeg does not support
                // it.
                //
                if (_ndiFourCC == NDIlib_FourCC_type_UYVA)
                {
                    uint8_t* const data0 = _avFrame->data[0];
                    const size_t stride = w * sizeof(uint8_t) * 2;
                    uint8_t* inP = data0 + stride * h;
                    uint8_t* outP = data;
                    for (int y = 0; y < h; ++y)
                    {
                        for (int x = 0; x < w; ++x)
                        {
                            const int rgbaIndex = (y * w + x) * 4;
                            outP[rgbaIndex + 3] = *inP;
                            ++inP;
                        }
                    }
                }
            }

            image->setTags(_tags);
        }
    } // namespace ndi
} // namespace tl

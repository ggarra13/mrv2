// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Image.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/Locale.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>

namespace tl
{
    namespace image
    {
        math::Box2i getBox(float aspect, const math::Box2i& box)
        {
            math::Box2i out;
            const math::Size2i boxSize = box.getSize();
            const float boxAspect = boxSize.getAspect();
            if (boxAspect > aspect)
            {
                out = math::Box2i(
                    box.min.x + boxSize.w / 2.F - (boxSize.h * aspect) / 2.F,
                    box.min.y, boxSize.h * aspect, boxSize.h);
            }
            else
            {
                out = math::Box2i(
                    box.min.x,
                    box.min.y + boxSize.h / 2.F - (boxSize.w / aspect) / 2.F,
                    boxSize.w, boxSize.w / aspect);
            }
            return out;
        }

        TLRENDER_ENUM_IMPL(
            PixelType, "None",

            "L_U8", "L_U16", "L_U32", "L_F16", "L_F32",

            "LA_U8", "LA_U16", "LA_U32", "LA_F16", "LA_F32",

            "RGB_U8", "RGB_U10", "RGB_U16", "RGB_U32", "RGB_F16", "RGB_F32",

            "RGBA_U8", "RGBA_U16", "RGBA_U32", "RGBA_F16", "RGBA_F32",

            "YUV_420P_U8", "YUV_422P_U8", "YUV_444P_U8",

            "YUV_420P_U10", "YUV_422P_U10", "YUV_444P_U10",
            
            "YUV_420P_U12", "YUV_422P_U12", "YUV_444P_U12",
            
            "YUV_420P_U16", "YUV_422P_U16", "YUV_444P_U16",

            "ARGB_4444_Premult");
        TLRENDER_ENUM_SERIALIZE_IMPL(PixelType);

        TLRENDER_ENUM_IMPL(YUVCoefficients, "REC709", "BT2020");
        TLRENDER_ENUM_SERIALIZE_IMPL(YUVCoefficients);

        math::Vector4f getYUVCoefficients(YUVCoefficients value)
        {
            //! References:
            //! * https://www.itu.int/rec/R-REC-BT.709
            //! * https://www.itu.int/rec/R-REC-BT.2020
            //! * https://gist.github.com/yohhoy/dafa5a47dade85d8b40625261af3776a
            //!
            //!     Y  = a * R + b * G + c * B
            //!     Cb = (B - Y) / d
            //!     Cr = (R - Y) / e
            //!
            //!     R = Y + e * Cr
            //!     G = Y - (a * e / b) * Cr - (c * d / b) * Cb
            //!     B = Y + d * Cb
            //!
            //!        BT.601   BT.709   BT.2020
            //!     ----------------------------
            //!     a  0.299    0.2126   0.2627
            //!     b  0.587    0.7152   0.6780
            //!     c  0.114    0.0722   0.0593
            //!     d  1.772    1.8556   1.8814
            //!     e  1.402    1.5748   1.4746
            //!
            const std::array<
                math::Vector4f, static_cast<size_t>(YUVCoefficients::Count)>
                data = {
                    math::Vector4f(1.5748, 0.468124273, 0.187324273, 1.8556),
                    math::Vector4f(1.4746, 0.6780, 0.0593, 1.8814)};
            return data[static_cast<size_t>(value)];
        }

        TLRENDER_ENUM_IMPL(VideoLevels, "FullRange", "LegalRange");
        TLRENDER_ENUM_SERIALIZE_IMPL(VideoLevels);

        int getChannelCount(PixelType value)
        {
            const std::array<int, static_cast<size_t>(PixelType::Count)>
                values = {0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3,
                3, 3, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4};
            return values[static_cast<size_t>(value)];
        }

        int getBitDepth(PixelType value)
        {
            const std::array<int, static_cast<size_t>(PixelType::Count)>
                values = {0,  8,  16, 32, 16, 32, 8,  16, 32, 16,
                          32, 8,  10, 16, 32, 16, 32, 8,  16, 32,
                          16, 32, 8,  8,  8,  16, 16, 16, 16, 16, 16,
                          16, 16, 16, 4};
            return values[static_cast<size_t>(value)];
        }

        PixelType getIntType(std::size_t channelCount, std::size_t bitDepth)
        {
            PixelType out = PixelType::kNone;
            switch (channelCount)
            {
            case 1:
                switch (bitDepth)
                {
                case 8:
                    out = PixelType::L_U8;
                    break;
                case 16:
                    out = PixelType::L_U16;
                    break;
                case 32:
                    out = PixelType::L_U32;
                    break;
                }
                break;
            case 2:
                switch (bitDepth)
                {
                case 8:
                    out = PixelType::LA_U8;
                    break;
                case 16:
                    out = PixelType::LA_U16;
                    break;
                case 32:
                    out = PixelType::LA_U32;
                    break;
                }
                break;
            case 3:
                switch (bitDepth)
                {
                case 8:
                    out = PixelType::RGB_U8;
                    break;
                case 10:
                    out = PixelType::RGB_U10;
                    break;
                case 16:
                    out = PixelType::RGB_U16;
                    break;
                case 32:
                    out = PixelType::RGB_U32;
                    break;
                }
                break;
            case 4:
                switch (bitDepth)
                {
                case 8:
                    out = PixelType::RGBA_U8;
                    break;
                case 16:
                    out = PixelType::RGBA_U16;
                    break;
                case 32:
                    out = PixelType::RGBA_U32;
                    break;
                }
                break;
            }
            return out;
        }

        PixelType getFloatType(std::size_t channelCount, std::size_t bitDepth)
        {
            PixelType out = PixelType::kNone;
            switch (channelCount)
            {
            case 1:
                switch (bitDepth)
                {
                case 16:
                    out = PixelType::L_F16;
                    break;
                case 32:
                    out = PixelType::L_F32;
                    break;
                }
                break;
            case 2:
                switch (bitDepth)
                {
                case 16:
                    out = PixelType::LA_F16;
                    break;
                case 32:
                    out = PixelType::LA_F32;
                    break;
                }
                break;
            case 3:
                switch (bitDepth)
                {
                case 16:
                    out = PixelType::RGB_F16;
                    break;
                case 32:
                    out = PixelType::RGB_F32;
                    break;
                }
                break;
            case 4:
                switch (bitDepth)
                {
                case 16:
                    out = PixelType::RGBA_F16;
                    break;
                case 32:
                    out = PixelType::RGBA_F32;
                    break;
                }
                break;
            }
            return out;
        }

        PixelType
        getClosest(PixelType value, const std::vector<PixelType>& types)
        {
            std::map<size_t, PixelType> diff;
            for (const auto& type : types)
            {
                diff
                    [abs(getChannelCount(value) - getChannelCount(type)) +
                     abs(getBitDepth(value) - getBitDepth(type))] = type;
            }
            return !diff.empty() ? diff.begin()->second : PixelType::kNone;
        }

        size_t getAlignedByteCount(size_t value, size_t alignment)
        {
            return (value / alignment * alignment) +
                   (value % alignment != 0 ? alignment : 0);
        }

        std::size_t getDataByteCount(const Info& info)
        {
            std::size_t out = 0;
            const size_t w = info.size.w;
            const size_t h = info.size.h;
            const size_t alignment = info.layout.alignment;
            switch (info.pixelType)
            {
            case PixelType::L_U8:
                out = getAlignedByteCount(w, alignment) * h;
                break;
            case PixelType::L_U16:
                out = getAlignedByteCount(w * 2, alignment) * h;
                break;
            case PixelType::L_U32:
                out = getAlignedByteCount(w * 4, alignment) * h;
                break;
            case PixelType::L_F16:
                out = getAlignedByteCount(w * 2, alignment) * h;
                break;
            case PixelType::L_F32:
                out = getAlignedByteCount(w * 4, alignment) * h;
                break;

            case PixelType::LA_U8:
                out = getAlignedByteCount(w * 2, alignment) * h;
                break;
            case PixelType::LA_U16:
                out = getAlignedByteCount(w * 2 * 2, alignment) * h;
                break;
            case PixelType::LA_U32:
                out = getAlignedByteCount(w * 2 * 4, alignment) * h;
                break;
            case PixelType::LA_F16:
                out = getAlignedByteCount(w * 2 * 2, alignment) * h;
                break;
            case PixelType::LA_F32:
                out = getAlignedByteCount(w * 2 * 4, alignment) * h;
                break;

            case PixelType::RGB_U8:
                out = getAlignedByteCount(w * 3, alignment) * h;
                break;
            case PixelType::RGB_U10:
                out = getAlignedByteCount(w * 4, alignment) * h;
                break;
            case PixelType::RGB_U16:
                out = getAlignedByteCount(w * 3 * 2, alignment) * h;
                break;
            case PixelType::RGB_U32:
                out = getAlignedByteCount(w * 3 * 4, alignment) * h;
                break;
            case PixelType::RGB_F16:
                out = getAlignedByteCount(w * 3 * 2, alignment) * h;
                break;
            case PixelType::RGB_F32:
                out = getAlignedByteCount(w * 3 * 4, alignment) * h;
                break;

            case PixelType::RGBA_U8:
                out = getAlignedByteCount(w * 4, alignment) * h;
                break;
            case PixelType::RGBA_U16:
                out = getAlignedByteCount(w * 4 * 2, alignment) * h;
                break;
            case PixelType::RGBA_U32:
                out = getAlignedByteCount(w * 4 * 4, alignment) * h;
                break;
            case PixelType::RGBA_F16:
                out = getAlignedByteCount(w * 4 * 2, alignment) * h;
                break;
            case PixelType::RGBA_F32:
                out = getAlignedByteCount(w * 4 * 4, alignment) * h;
                break;

            //! \todo Is YUV data aligned?
            case PixelType::YUV_420P_U8:
                out = w * h + (w / 2 * h / 2) + (w / 2 * h / 2);
                break;
            case PixelType::YUV_422P_U8:
                out = w * h + (w / 2 * h) + (w / 2 * h);
                break;
            case PixelType::YUV_444P_U8:
                out = w * h * 3;
                break;
            case PixelType::YUV_420P_U10:
                out = (w * h + (w / 2 * h / 2) + (w / 2 * h / 2)) * 2;
                break;
            case PixelType::YUV_422P_U10:
                out = (w * h + (w / 2 * h) + (w / 2 * h)) * 2;
                break;
            case PixelType::YUV_444P_U10:
                out = (w * h * 3) * 2;
                break;
            case PixelType::YUV_420P_U12:
                out = (w * h + (w / 2 * h / 2) + (w / 2 * h / 2)) * 2;
                break;
            case PixelType::YUV_422P_U12:
                out = (w * h + (w / 2 * h) + (w / 2 * h)) * 2;
                break;
            case PixelType::YUV_444P_U12:
                out = (w * h * 3) * 2;
                break;
            case PixelType::YUV_420P_U16:
                out = (w * h + (w / 2 * h / 2) + (w / 2 * h / 2)) * 2;
                break;
            case PixelType::YUV_422P_U16:
                out = (w * h + (w / 2 * h) + (w / 2 * h)) * 2;
                break;
            case PixelType::YUV_444P_U16:
                out = (w * h * 3) * 2;
                break;

            case PixelType::ARGB_4444_Premult:
                out = w * h * 4 * 2;
                break;

            default:
                break;
            }
            return out;
        }

        void Image::_init(const Info& info, const bool ownsData)
        {
            _info = info;
            _owns = ownsData;
            _dataByteCount = image::getDataByteCount(info);
            if (ownsData)
            {
                //! \bug Allocate a bit of extra space since FFmpeg sws_scale()
                //! seems to be reading past the end?
                _data = new uint8_t[_dataByteCount + 16];
            }
        }

        Image::Image()
        {
        }

        Image::~Image()
        {
            if (_owns)
                delete [] _data;
        }

        std::shared_ptr<Image> Image::create(const Info& info)
        {
            auto out = std::shared_ptr<Image>(new Image);
            out->_init(info, true);
            return out;
        }

        std::shared_ptr<Image>
        Image::create(const Size& size, PixelType pixelType)
        {
            return create(Info(size, pixelType));
        }

        std::shared_ptr<Image> Image::create(int w, int h, PixelType pixelType)
        {
            return create(Info(w, h, pixelType));
        }

        std::shared_ptr<Image> Image::create(
            const Info& info,
            const std::shared_ptr<AVFrame> _avFrame,
            const uint8_t* planes[3],
            const int linesize[3])
        {
            auto out = std::shared_ptr<Image>(new Image);
            out->_init(info, false);
            out->_planar = true;
            for (int i = 0; i < 3; ++i)
            {
                out->_planes[i] = planes[i];
                out->_linesize[i] = linesize[i];
            }
            out->_avFrame = _avFrame;
            return out;
        }
        
        void Image::setTags(const Tags& value)
        {
            _tags = value;
        }

        void Image::zero()
        {
            std::memset(_data, 0, _dataByteCount);
        }

        void to_json(nlohmann::json& json, const Size& value)
        {
            json = {value.w, value.h};
        }

        void from_json(const nlohmann::json& json, Size& value)
        {
            json.at(0).get_to(value.w);
            json.at(1).get_to(value.h);
        }

        std::ostream& operator<<(std::ostream& os, const Size& value)
        {
            os << value.w << "x" << value.h;
            if (value.pixelAspectRatio != 1.F)
            {
                os << ":" << value.pixelAspectRatio;
            }
            return os;
        }

        std::istream& operator>>(std::istream& is, Size& out)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, ':');
            if (0 == split.size() || split.size() > 2)
            {
                throw error::ParseError();
            }
            if (2 == split.size())
            {
                locale::SetAndRestore saved;
                out.pixelAspectRatio = std::stof(split[1]);
            }
            split = string::split(split[0], 'x');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            out.w = std::stoi(split[0]);
            out.h = std::stoi(split[1]);
            return is;
        }
    } // namespace image
} // namespace tl

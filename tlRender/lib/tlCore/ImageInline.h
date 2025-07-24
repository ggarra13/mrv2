// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tuple>

namespace tl
{
    namespace image
    {
        constexpr Size::Size() {}

        constexpr Size::Size(int w, int h, float pixelAspectRatio) :
            w(w),
            h(h),
            pixelAspectRatio(pixelAspectRatio)
        {
        }

        constexpr bool Size::isValid() const
        {
            return w > 0 && h > 0;
        }

        constexpr float Size::getAspect() const
        {
            return h > 0 ? (w / static_cast<float>(h) * pixelAspectRatio) : 0;
        }

        constexpr bool Size::operator==(const Size& other) const
        {
            return w == other.w && h == other.h &&
                   pixelAspectRatio == other.pixelAspectRatio;
        }

        constexpr bool Size::operator!=(const Size& other) const
        {
            return !(*this == other);
        }

        inline bool Size::operator<(const Size& other) const
        {
            const int widthScaled = static_cast<int>(w * pixelAspectRatio);
            const int otherWidthScaled =
                static_cast<int>(other.w * other.pixelAspectRatio);
            return std::tie(widthScaled, h) <
                   std::tie(otherWidthScaled, other.h);
        }

        constexpr bool U10_MSB::operator==(const U10_MSB& value) const
        {
            return value.r == r && value.g == g && value.b == b;
        }

        constexpr bool U10_MSB::operator!=(const U10_MSB& value) const
        {
            return !(*this == value);
        }

        constexpr bool U10_LSB::operator==(const U10_LSB& value) const
        {
            return value.r == r && value.g == g && value.b == b;
        }

        constexpr bool U10_LSB::operator!=(const U10_LSB& value) const
        {
            return !(*this == value);
        }

        constexpr Mirror::Mirror() {}

        constexpr Mirror::Mirror(bool x, bool y) :
            x(x),
            y(y)
        {
        }

        constexpr bool Mirror::operator==(const Mirror& other) const
        {
            return other.x == x && other.y == y;
        }

        constexpr bool Mirror::operator!=(const Mirror& other) const
        {
            return !(other == *this);
        }

        inline Layout::Layout() {}

        inline Layout::Layout(
            const Mirror& mirror, int alignment, memory::Endian endian) :
            mirror(mirror),
            alignment(alignment),
            endian(endian)
        {
        }

        constexpr bool Layout::operator==(const Layout& other) const
        {
            return other.mirror == mirror && other.alignment == alignment &&
                   other.endian == endian;
        }

        constexpr bool Layout::operator!=(const Layout& other) const
        {
            return !(other == *this);
        }

        inline Info::Info() {}

        inline Info::Info(const Size& size, PixelType pixelType) :
            size(size),
            pixelType(pixelType)
        {
        }

        inline Info::Info(int w, int h, PixelType pixelType) :
            size(w, h),
            pixelType(pixelType)
        {
        }

        inline bool Info::isValid() const
        {
            return size.isValid() && pixelType != PixelType::kNone;
        }

        inline bool Info::operator==(const Info& other) const
        {
            return name == other.name && compression == other.compression &&
                   size == other.size && pixelType == other.pixelType &&
                   videoLevels == other.videoLevels &&
                   yuvCoefficients == other.yuvCoefficients &&
                   layout == other.layout;
        }

        inline bool Info::operator!=(const Info& other) const
        {
            return !(*this == other);
        }

        inline const Info& Image::getInfo() const
        {
            return _info;
        }

        inline const Size& Image::getSize() const
        {
            return _info.size;
        }

        inline int Image::getWidth() const
        {
            return _info.size.w;
        }

        inline int Image::getHeight() const
        {
            return _info.size.h;
        }

        inline float Image::getAspect() const
        {
            return _info.size.getAspect();
        }

        inline void Image::setPixelAspectRatio(const float x)
        {
            _info.size.pixelAspectRatio = x;
        }

        inline PixelType Image::getPixelType() const
        {
            return _info.pixelType;
        }

        inline bool Image::isValid() const
        {
            return _info.isValid();
        }

        inline const Tags& Image::getTags() const
        {
            return _tags;
        }

        inline size_t Image::getDataByteCount() const
        {
            return _dataByteCount;
        }

        inline const uint8_t* Image::getData() const
        {
            return _data;
        }

        inline uint8_t* Image::getData()
        {
            return _data;
        }
    } // namespace image
} // namespace tl

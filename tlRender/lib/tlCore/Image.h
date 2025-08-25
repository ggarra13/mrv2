// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Box.h>
#include <tlCore/Memory.h>
#include <tlCore/Range.h>
#include <tlCore/Util.h>

#include <half.h>

#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

extern "C"
{
    struct AVFrame;
}

namespace tl
{
    namespace image
    {
        //! \name Sizes
        ///@{

        //! Image size.
        class Size
        {
        public:
            constexpr Size();
            constexpr Size(int w, int h, float pixelAspectRatio = 1.F);

            int w = 0;
            int h = 0;
            float pixelAspectRatio = 1.F;

            //! Is the size valid?
            constexpr bool isValid() const;

            //! Get the aspect ratio.
            constexpr float getAspect() const;

            constexpr bool operator==(const Size&) const;
            constexpr bool operator!=(const Size&) const;
            bool operator<(const Size&) const;
        };

        //! Get a box with the given aspect ratio that fits within
        //! the given box.
        math::Box2i getBox(float aspect, const math::Box2i&);

        ///@}

        //! \name Pixel Types
        ///@{

        //! Image pixel types.
        enum class PixelType {
            kNone,

            L_U8,
            L_U16,
            L_U32,
            L_F16,
            L_F32,

            LA_U8,
            LA_U16,
            LA_U32,
            LA_F16,
            LA_F32,

            RGB_U8,
            RGB_U10,
            RGB_U16,
            RGB_U32,
            RGB_F16,
            RGB_F32,

            RGBA_U8,
            RGBA_U16,
            RGBA_U32,
            RGBA_F16,
            RGBA_F32,

            YUV_420P_U8,
            YUV_422P_U8,
            YUV_444P_U8,

            YUV_420P_U10,
            YUV_422P_U10,
            YUV_444P_U10,
            
            YUV_420P_U12,
            YUV_422P_U12,
            YUV_444P_U12,
            
            YUV_420P_U16,
            YUV_422P_U16,
            YUV_444P_U16,

            ARGB_4444_Premult,

            Count,
            First = kNone
        };
        TLRENDER_ENUM(PixelType);
        TLRENDER_ENUM_SERIALIZE(PixelType);

        typedef uint8_t U8_T;
        typedef uint16_t U10_T;
        typedef uint16_t U12_T;
        typedef uint16_t U16_T;
        typedef uint32_t U32_T;
        typedef half F16_T;
        typedef float F32_T;

        const math::Range<U8_T> U8Range(
            std::numeric_limits<U8_T>::min(), std::numeric_limits<U8_T>::max());
        const math::Range<U10_T> U10Range(0, 1023);
        const math::Range<U12_T> U12Range(0, 4095);
        const math::Range<U16_T> U16Range(
            std::numeric_limits<U16_T>::min(),
            std::numeric_limits<U16_T>::max());
        const math::Range<U32_T> U32Range(
            std::numeric_limits<U32_T>::min(),
            std::numeric_limits<U32_T>::max());
        const math::Range<F16_T> F16Range(0.F, 1.F);
        const math::Range<F32_T> F32Range(0.F, 1.F);

        //! Video levels.
        enum class VideoLevels {
            FullRange,
            LegalRange,

            Count,
            First = FullRange
        };
        TLRENDER_ENUM(VideoLevels);
        TLRENDER_ENUM_SERIALIZE(VideoLevels);

        //! YUV coefficients.
        enum class YUVCoefficients {
            REC709,
            BT2020,

            Count,
            First = REC709
        };
        TLRENDER_ENUM(YUVCoefficients);
        TLRENDER_ENUM_SERIALIZE(YUVCoefficients);

        //! Get YUV coefficients.
        math::Vector4f getYUVCoefficients(YUVCoefficients);

        //! 10-bit MSB pixel data.
        struct U10_MSB
        {
            uint32_t r : 10;
            uint32_t g : 10;
            uint32_t b : 10;
            uint32_t pad : 2;

            constexpr bool operator==(const U10_MSB&) const;
            constexpr bool operator!=(const U10_MSB&) const;
        };

        //! 10-bit LSB pixel data.
        struct U10_LSB
        {
            uint32_t pad : 2;
            uint32_t b : 10;
            uint32_t g : 10;
            uint32_t r : 10;

            constexpr bool operator==(const U10_LSB&) const;
            constexpr bool operator!=(const U10_LSB&) const;
        };
#if defined(TLRENDER_ENDIAN_MSB)
        typedef U10_MSB U10;
#else  // TLRENDER_ENDIAN_MSB
        typedef U10_LSB U10;
#endif // TLRENDER_ENDIAN_MSB

        //! Get the number of channels for the given pixel type.
        int getChannelCount(PixelType);

        //! Get the bit-depth for the given pixel type.
        int getBitDepth(PixelType);

        //! Determine the integer pixel type for a given channel count and bit
        //! depth.
        PixelType getIntType(std::size_t channelCount, std::size_t bitDepth);

        //! Determine the floating point pixel type for a given channel count
        //! and bit depth.
        PixelType getFloatType(std::size_t channelCount, std::size_t bitDepth);

        //! Get the closest pixel type for the given pixel type.
        PixelType getClosest(PixelType, const std::vector<PixelType>&);

        ///@}

        //! Image mirroring.
        class Mirror
        {
        public:
            constexpr Mirror();
            constexpr Mirror(bool x, bool y);

            bool x = false;
            bool y = false;

            constexpr bool operator==(const Mirror&) const;
            constexpr bool operator!=(const Mirror&) const;
        };

        //! Image data layout.
        class Layout
        {
        public:
            Layout();
            Layout(
                const Mirror& mirror, int alignment = 1,
                memory::Endian endian = memory::getEndian());

            Mirror mirror;
            int alignment = 1;
            memory::Endian endian = memory::getEndian();

            constexpr bool operator==(const Layout&) const;
            constexpr bool operator!=(const Layout&) const;
        };

        //! Image information.
        class Info
        {
        public:
            Info();
            Info(const Size&, PixelType);
            Info(int w, int h, PixelType);

            std::string name = "Default";
            std::string compression = "Unknown";
            bool isLossyCompression = false;
            bool isValidDeepCompression = false;
            int compressionNumScanlines = 0;
            Size size;
            PixelType pixelType = PixelType::kNone;
            VideoLevels videoLevels = VideoLevels::FullRange;
            YUVCoefficients yuvCoefficients = YUVCoefficients::REC709;
            Layout layout;

            //! Is the information valid?
            bool isValid() const;

            bool operator==(const Info&) const;
            bool operator!=(const Info&) const;
        };

        //! Get the number of bytes required to align data.
        size_t getAlignedByteCount(size_t value, size_t alignment);

        //! Get the number of bytes used to store image data.
        std::size_t getDataByteCount(const Info&);

        //! Image tags.
        typedef std::map<std::string, std::string> Tags;

        //! Image.
        class Image : public std::enable_shared_from_this<Image>
        {
            TLRENDER_NON_COPYABLE(Image);

        protected:
            void _init(const Info&, const bool ownsData);

            Image();

        public:
            ~Image();

            //! Create a new image.
            static std::shared_ptr<Image> create(const Info&);

            //! Create a new image.
            static std::shared_ptr<Image> create(const Size&, PixelType);

            //! Create a new image.
            static std::shared_ptr<Image> create(int w, int h, PixelType);

            //! Create a new image that references external data without
            //! copying it.
            static std::shared_ptr<Image> create(const Info& info,
                                                 const std::shared_ptr<AVFrame> avFrame,
                                                 const uint8_t* planes[3],
                                                 const int linesize[3]);

            //! Get the image information.
            const Info& getInfo() const;

            //! Get the image size.
            const Size& getSize() const;

            //! Get the image width.
            int getWidth() const;

            //! Get the image height.
            int getHeight() const;

            //! Get the aspect ratio.
            float getAspect() const;

            //! Set the pixel aspect ratio.
            void setPixelAspectRatio(float x);

            //! Get the image pixel type.
            PixelType getPixelType() const;

            //! Is the image valid?
            bool isValid() const;

            //! Get the image tags.
            const Tags& getTags() const;

            //! Set the image tags.
            void setTags(const Tags&);

            //! Get the number of bytes used to store the image data.
            size_t getDataByteCount() const;

            //! Get the image data.
            const uint8_t* getData() const;

            //! Get the image data.
            uint8_t* getData();

            //! Zero the image data.
            void zero();

            //! Fill in the image data with NANs or 255.
            void fill();
            
            //! Get number of planes.
            int getPlaneCount() const
                {
                    return _planar ? 3 : 1;
                }

            //! Get pointer to a specific plane.
            const uint8_t* getPlaneData(int index) const
                {
                    return _planar ? _planes[index] :
                        (index == 0 ? _data : nullptr);
                }

            //! Get linesize for a specific plane.
            int getLineSize(int index) const
                {
                    return _planar ? _linesize[index] : (_data ? _info.size.w : 0);
                }
            
        private:
            Info _info;
            Tags _tags;
            
            std::shared_ptr<AVFrame> _avFrame;
            const uint8_t* _planes[3] = { nullptr, nullptr, nullptr };
            int _linesize[3] = { 0, 0, 0 };
            bool _planar = false;
            
            uint8_t* _data = nullptr;
            size_t _dataByteCount = 0;
            bool _owns = false;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Size&);

        void from_json(const nlohmann::json&, Size&);

        std::ostream& operator<<(std::ostream&, const Size&);

        std::istream& operator>>(std::istream&, Size&);

        ///@}
    } // namespace image
} // namespace tl

#include <tlCore/ImageInline.h>

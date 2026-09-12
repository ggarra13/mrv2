// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvFl/mrvIO.h"

#include "mrvImage/mrvOperations.h"

namespace
{
    const char* kModule = "img.";
}

namespace
{
    // Saturating cast: integers clamp to their min/max, floats just cast.
    // (Works for uint8_t/uint16_t/int16_t/etc. and float/half-like types.)
    template <class T, class Acc>
    inline T sat_cast(Acc v)
    {
        if constexpr (std::numeric_limits<T>::is_integer)
        {
            const Acc lo = static_cast<Acc>(std::numeric_limits<T>::min());
            const Acc hi = static_cast<Acc>(std::numeric_limits<T>::max());
            v = std::clamp(v, lo, hi);
            return static_cast<T>(std::lround(v)); // round-to-nearest
        } else {
            // Floating-point (float/double/half): no clamp by default
            return static_cast<T>(v);
        }
    }

    template <typename T>
    inline T lerp(const T& a, const T& b, float t)
    {
        return a + (b - a) * t;
    }
}

namespace mrv
{

    // Generic pixel scaling (assumes operator+,-,* are defined for T)
    template <typename T>
    void scaleImageLinear(
        const T* src, std::size_t width, std::size_t height,
        T* dst, std::size_t W, std::size_t H, std::size_t channels,
        bool align_corners = false)
    {
        assert(src && dst);
        assert(width > 0 && height > 0 && W > 0 && H > 0 && channels > 0);

        using Acc = double; // accumulator precision

        const Acc xScale = (align_corners && W > 1) ?
                           Acc(width - 1) / Acc(W - 1) :
                           Acc(width)     / Acc(W);
        const Acc yScale = (align_corners && H > 1) ?
                           Acc(height - 1) / Acc(H - 1) :
                           Acc(height)     / Acc(H);

        for (std::size_t y = 0; y < H; ++y)
        {
            const Acc sy = align_corners ?
                           (y * yScale) :
                           ((y + Acc(0.5)) * yScale - Acc(0.5));
            const Acc syc = std::clamp(sy, Acc(0), Acc(height - 1));
            const std::size_t y0 = static_cast<std::size_t>(std::floor(syc));
            const std::size_t y1 = std::min(y0 + 1, height - 1);
            const Acc fy = syc - y0;

            for (std::size_t x = 0; x < W; ++x)
            {
                const Acc sx = align_corners ?
                               (x * xScale) :
                               ((x + Acc(0.5)) * xScale - Acc(0.5));
                const Acc sxc = std::clamp(sx, Acc(0), Acc(width - 1));
                const std::size_t x0 = static_cast<std::size_t>(
                    std::floor(sxc));
                const std::size_t x1 = std::min(x0 + 1, width - 1);
                const Acc fx = sxc - x0;

                const std::size_t dstBase = (y * W + x) * channels;

                for (std::size_t c = 0; c < channels; ++c) {
                    const T p00 = src[(y0 * width + x0) * channels + c];
                    const T p10 = src[(y0 * width + x1) * channels + c];
                    const T p01 = src[(y1 * width + x0) * channels + c];
                    const T p11 = src[(y1 * width + x1) * channels + c];

                    const Acc a00 = static_cast<Acc>(p00);
                    const Acc a10 = static_cast<Acc>(p10);
                    const Acc a01 = static_cast<Acc>(p01);
                    const Acc a11 = static_cast<Acc>(p11);

                    const Acc top    = lerp(a00, a10, fx);
                    const Acc bottom = lerp(a01, a11, fx);
                    const Acc value  = lerp(top, bottom, fy);

                    dst[dstBase + c] = sat_cast<T, Acc>(value);
                }
            }
        }
    }


    void scaleImageLinear(const std::shared_ptr<image::Image> source,
                          std::shared_ptr<image::Image> scaled)
    {
        if (source->getPixelType() != scaled->getPixelType())
        {
            throw std::runtime_error("scaleImageLinear source and output images don't match pixelType");
        }


        int numChannels = image::getChannelCount(source->getPixelType());
        switch(source->getPixelType())
        {
        case image::PixelType::L_U8:
        case image::PixelType::LA_U8:
        case image::PixelType::RGB_U8:
        case image::PixelType::RGBA_U8:
            scaleImageLinear(source->getData(),
                             source->getWidth(),
                             source->getHeight(),
                             scaled->getData(),
                             scaled->getWidth(),
                             scaled->getHeight(),
                             numChannels);
            break;
        case image::PixelType::L_U16:
        case image::PixelType::LA_U16:
        case image::PixelType::RGB_U16:
        case image::PixelType::RGBA_U16:
            scaleImageLinear(reinterpret_cast<uint16_t*>(source->getData()),
                             source->getWidth(),
                             source->getHeight(),
                             reinterpret_cast<uint16_t*>(scaled->getData()),
                             scaled->getWidth(),
                             scaled->getHeight(),
                             numChannels);
            break;
        case image::PixelType::L_F16:
        case image::PixelType::LA_F16:
        case image::PixelType::RGB_F16:
        case image::PixelType::RGBA_F16:
            scaleImageLinear(reinterpret_cast<half*>(source->getData()),
                             source->getWidth(),
                             source->getHeight(),
                             reinterpret_cast<half*>(scaled->getData()),
                             scaled->getWidth(),
                             scaled->getHeight(),
                             numChannels);
            break;
        case image::PixelType::L_F32:
        case image::PixelType::LA_F32:
        case image::PixelType::RGB_F32:
        case image::PixelType::RGBA_F32:
            scaleImageLinear(reinterpret_cast<float*>(source->getData()),
                             source->getWidth(),
                             source->getHeight(),
                             reinterpret_cast<float*>(scaled->getData()),
                             scaled->getWidth(),
                             scaled->getHeight(),
                             numChannels);
            break;
        default:
            throw std::runtime_error("scaleImageLinear: Unhandled pixel type");
        }
    }

    void flipImageInY(const std::shared_ptr<image::Image> image)
    {
        const image::PixelType pixelType = image->getPixelType();
        const size_t width = image->getWidth();
        const size_t height = image->getHeight();
        switch(pixelType)
        {
        case image::PixelType::RGBA_U8:
            flipImageInY<uint8_t>(
                reinterpret_cast<uint8_t*>(image->getData()),
                width, height, 4);
            break;
        case image::PixelType::RGB_U16:
            flipImageInY<uint16_t>(
                reinterpret_cast<uint16_t*>(image->getData()),
                width, height, 3);
            break;
        case image::PixelType::RGBA_U16:
            flipImageInY<uint16_t>(
                reinterpret_cast<uint16_t*>(image->getData()),
                width, height, 4);
            break;
        case image::PixelType::RGB_F16:
            flipImageInY<half>(
                reinterpret_cast<half*>(image->getData()),
                width, height, 3);
            break;
        case image::PixelType::RGBA_F16:
            flipImageInY<half>(
                reinterpret_cast<half*>(image->getData()),
                width, height, 4);
            break;
        case image::PixelType::RGB_F32:
            flipImageInY<float>(
                reinterpret_cast<float*>(image->getData()),
                width, height, 3);
            break;
        case image::PixelType::RGBA_F32:
            flipImageInY<float>(
                reinterpret_cast<float*>(image->getData()),
                width, height, 4);
            break;
        default:
            LOG_ERROR("Unknown buffer for flipImageInY info pixel type " << pixelType);
            break;
        }
    }

    void composite_RGBA_U8(std::shared_ptr<image::Image>& dest,
                           std::shared_ptr<image::Image>& src)
    {
        const image::PixelType pixelType = dest->getPixelType();
        const size_t width = dest->getWidth();
        const size_t height = dest->getHeight();
        switch(pixelType)
        {
        case image::PixelType::RGB_U8:
            compositeImageOverNoAlpha<uint8_t>(
                reinterpret_cast<uint8_t*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGB_U16:
            compositeImageOverNoAlpha<uint16_t>(
                reinterpret_cast<uint16_t*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGBA_U16:
            compositeImageOver<uint16_t>(
                reinterpret_cast<uint16_t*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGB_F16:
            compositeImageOverNoAlpha<half>(
                reinterpret_cast<half*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGBA_F16:
            compositeImageOver<half>(
                reinterpret_cast<half*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGB_F32:
            compositeImageOverNoAlpha<float>(
                reinterpret_cast<float*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGBA_F32:
            compositeImageOver<float>(
                reinterpret_cast<float*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGBA_U8:
            compositeImageOver<uint8_t>(
                reinterpret_cast<uint8_t*>(dest->getData()),
                src->getData(),
                width, height);
            break;
        default:
            LOG_ERROR("Unknown buffer for composite info pixel type " << pixelType);
            break;
        }
    }

    void convert_RGBA_to_RGB_U8(std::shared_ptr<image::Image>& dest,
                                   std::shared_ptr<image::Image>& src)
    {
        if (dest->getPixelType() != image::PixelType::RGB_U8)
        {
            LOG_ERROR("convert_RGBA_U8_to_RGB_U8: dest image not RGB_U8");
            return;
        }
        switch(src->getPixelType())
        {
        case image::PixelType::RGB_U8:
        {
            uint8_t* d  = dest->getData();
            const uint8_t* s  = src->getData();
            memcpy(d, s, src->getDataByteCount());
            break;
        }
        case image::PixelType::RGB_U16:
        {
            uint8_t* d  = dest->getData();
            const uint16_t* s = reinterpret_cast<uint16_t*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s / 65535; ++s;
                *d++ = *s / 65535; ++s;
                *d++ = *s / 65535; ++s;
            }
            break;
        }
        case image::PixelType::RGB_F16:
        {
            uint8_t* d  = dest->getData();
            const half* s = reinterpret_cast<half*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
            }
            break;
        }
        case image::PixelType::RGB_F32:
        {
            uint8_t* d  = dest->getData();
            const float* s = reinterpret_cast<float*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
            }
            break;
        }
        case image::PixelType::RGBA_U8:
        {
            uint8_t* d  = dest->getData();
            const uint8_t* s  = src->getData();
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s++;
                *d++ = *s++;
                *d++ = *s++;
                s++;
            }
            break;
        }
        case image::PixelType::RGBA_U16:
        {
            uint8_t* d  = dest->getData();
            const uint16_t* s = reinterpret_cast<uint16_t*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s / 65535; ++s;
                *d++ = *s / 65535; ++s;
                *d++ = *s / 65535; ++s;
                s++;
            }
            break;
        }
        case image::PixelType::RGBA_F16:
        {
            uint8_t* d  = dest->getData();
            const half* s = reinterpret_cast<half*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                s++;
            }
            break;
        }
        case image::PixelType::RGBA_F32:
        {
            uint8_t* d  = dest->getData();
            const float* s = reinterpret_cast<float*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                s++;
            }
            break;
        }
        default:
            LOG_ERROR("convert_RGBA_U8_to_RGB_U8: src image not RGBA");
            break;
        }
    }

}

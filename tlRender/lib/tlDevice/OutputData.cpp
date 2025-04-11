// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston/Gonzalo Garramulo
// All rights reserved.

#include <tlDevice/OutputData.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>

namespace tl
{
    struct YUVCoefficients8bits
    {
        uint8_t kr, kg, kb;
    };

    YUVCoefficients8bits getYUVCoefficients8bits(int width, int height)
    {
        if (width > 720 && height > 576)
        {
            return {55, 183, 18}; // BT.709 standard (scaled to 256)
        }
        else
        {
            return {77, 150, 29}; // BT.601 standard (scaled to 256)
        }
    }

    struct YUVCoefficients16bits
    {
        uint16_t kr, kg, kb;
    };

    YUVCoefficients16bits getYUVCoefficients16bits(int width, int height)
    {
        if (width > 720 && height > 576)
        {
            // BT.709 standard (16-bit scale)
            return {
                (uint16_t)(0.2126 * 65535), // kr
                (uint16_t)(0.7152 * 65535), // kg
                (uint16_t)(0.0722 * 65535)  // kb
            };
        }
        else
        {
            // BT.601 standard (16-bit scale)
            return {
                (uint16_t)(0.299 * 65535), // kr
                (uint16_t)(0.587 * 65535), // kg
                (uint16_t)(0.114 * 65535)  // kb
            };
        }
    }

    namespace device
    {
        bool DisplayMode::operator==(const DisplayMode& other) const
        {
            return name == other.name && size == other.size &&
                   frameRate == other.frameRate;
        }

        TLRENDER_ENUM_IMPL(
            PixelType, "None", "8BitBGRA", "8BitYUV", "10BitRGB", "10BitRGBX",
            "10BitRGBXLE",
            //"10BitYUV",
            "12BitRGB", "12BitRGBLE", "8BitUYVA", "16BitP216", "16BitPA16",
            "8BitI420", "8BitBGRX", "8BitRGBA", "8BitRGBX");
        TLRENDER_ENUM_SERIALIZE_IMPL(PixelType);

        size_t getRowByteCount(int size, PixelType pixelType)
        {
            size_t out = 0;
            switch (pixelType)
            {
            case PixelType::_8BitRGBX:
            case PixelType::_8BitBGRX:
            case PixelType::_8BitRGBA:
            case PixelType::_8BitBGRA:
                out = size * 32 / 8;
                break;
            case PixelType::_8BitYUV:
            case PixelType::_8BitI420:
                out = size * 16 / 8;
                break;
            case PixelType::_10BitRGB:
            case PixelType::_10BitRGBX:
            case PixelType::_10BitRGBXLE:
                out = ((size + 63) / 64) * 256;
                break;
            // case PixelType::_10BitYUV:
            //     out = ((size + 47) / 48) * 128;
            //     break;
            case PixelType::_12BitRGB:
            case PixelType::_12BitRGBLE:
                out = size * 36 / 8;
                break;
            case PixelType::_8BitUYVA:
                out = size * 32 / 8;
                break;
            case PixelType::_16BitP216:
                out = size * 48 / 8;
                break;
            case PixelType::_16BitPA16:
                out = size * 64 / 8;
                break;
            case PixelType::None:
            case PixelType::Count:
                break;
            }
            return out;
        }

        image::PixelType getColorBuffer(PixelType value)
        {
            const std::array<
                image::PixelType, static_cast<size_t>(PixelType::Count)>
                data = {
                    image::PixelType::None,
                    image::PixelType::RGBA_U8, // 8BitBGRA
                    image::PixelType::RGBA_U8, // 8BitYUV
                    image::PixelType::RGB_U16, // 10BitRGB
                    image::PixelType::RGB_U16, // 10BitRGBX
                    image::PixelType::RGB_U16, // 10BitRGBXLE
                    // image::PixelType::RGB_U10,  // 10BitYUV
                    image::PixelType::RGB_U16,  // 12BitRGB
                    image::PixelType::RGB_U16,  // 12BitRGBLE
                    image::PixelType::RGBA_U8,  // 8BitUYVA
                    image::PixelType::RGB_U16,  // 16BitP216 (4:2:2 16bits YUV
                                                // sans alpha)
                    image::PixelType::RGBA_U16, // 16BitPA16 (4:2:2 16bits YUV
                                                // with alpha)
                    image::PixelType::RGB_U8, // 8BitI420  (4:2:2 8bits YUV with
                                              // UV reversed)
                    image::PixelType::RGB_U8, // 8BitBGRX
                    image::PixelType::RGBA_U8, // 8BitRGBA
                    image::PixelType::RGB_U8,  // 8BitRGBX
                };
            return data[static_cast<size_t>(value)];
        }

        size_t getPackPixelsSize(const math::Size2i& size, PixelType pixelType)
        {
            size_t out = 0;
            switch (pixelType)
            {
            case PixelType::_8BitBGRA:
            case PixelType::_8BitBGRX:
            case PixelType::_8BitRGBA:
            case PixelType::_8BitRGBX:
            case PixelType::_8BitYUV:
            case PixelType::_8BitUYVA:
            case PixelType::_8BitI420:
            case PixelType::_16BitP216:
            case PixelType::_16BitPA16:
                // case PixelType::_10BitYUV:
                out = getDataByteCount(size, pixelType);
                break;
            case PixelType::_10BitRGB:
            case PixelType::_10BitRGBX:
            case PixelType::_10BitRGBXLE:
            case PixelType::_12BitRGB:
            case PixelType::_12BitRGBLE:
                out = size.w * size.h * 3 * sizeof(uint16_t);
                break;
            case PixelType::None:
                break;
            case PixelType::Count:
                break;
            }
            return out;
        }

        size_t getDataByteCount(const math::Size2i& size, PixelType pixelType)
        {
            return getRowByteCount(size.w, pixelType) * size.h;
        }

        bool DeviceInfo::operator==(const DeviceInfo& other) const
        {
            return name == other.name && displayModes == other.displayModes &&
                   pixelTypes == other.pixelTypes &&
                   hdrMetaData == other.hdrMetaData;
        }

        bool DeviceInfo::operator!=(const DeviceInfo& other) const
        {
            return !(*this == other);
        }

        TLRENDER_ENUM_IMPL(Option, "None", "444SDIVideoOutput");
        TLRENDER_ENUM_SERIALIZE_IMPL(Option);

        bool DeviceConfig::operator==(const DeviceConfig& other) const
        {
            return deviceIndex == other.deviceIndex &&
                   displayModeIndex == other.displayModeIndex &&
                   pixelType == other.pixelType &&
                   boolOptions == other.boolOptions &&
                   noAudio == other.noAudio && noMetadata == other.noMetadata;
        }

        bool DeviceConfig::operator!=(const DeviceConfig& other) const
        {
            return !(*this == other);
        }

        TLRENDER_ENUM_IMPL(HDRMode, "None", "FromFile", "Custom");
        TLRENDER_ENUM_SERIALIZE_IMPL(HDRMode);

        std::shared_ptr<image::HDRData>
        getHDRData(const timeline::VideoData& videoData)
        {
            std::shared_ptr<image::HDRData> out;
            for (const auto& layer : videoData.layers)
            {
                if (layer.image)
                {
                    const auto& tags = layer.image->getTags();
                    const auto k = tags.find("hdr");
                    if (k != tags.end())
                    {
                        out =
                            std::shared_ptr<image::HDRData>(new image::HDRData);
                        try
                        {
                            auto json = nlohmann::json::parse(k->second);
                            from_json(json, *out);
                        }
                        catch (const std::exception&)
                        {
                        }
                        break;
                    }
                }
            }
            return out;
        }

        void copyPackPixels(
            void* outP, const void* inP, const math::Size2i& size,
            PixelType pixelType)
        {
            const int width = size.w;
            const int height = size.h;
            const size_t rowByteCount = getRowByteCount(width, pixelType);
            switch (pixelType)
            {
            case PixelType::_10BitRGB:
                for (int y = 0; y < height; ++y)
                {
                    const uint16_t* in16 = (const uint16_t*)inP + y * width * 3;
                    uint8_t* out8 = (uint8_t*)outP + y * rowByteCount;
                    for (int x = 0; x < width; ++x)
                    {
                        const uint16_t r10 = in16[0] >> 6;
                        const uint16_t g10 = in16[1] >> 6;
                        const uint16_t b10 = in16[2] >> 6;
                        out8[3] = b10;
                        out8[2] = (b10 >> 8) | (g10 << 2);
                        out8[1] = (g10 >> 6) | (r10 << 4);
                        out8[0] = r10 >> 4;

                        in16 += 3;
                        out8 += 4;
                    }
                }
                break;
            case PixelType::_10BitRGBX:
                for (int y = 0; y < height; ++y)
                {
                    const uint16_t* in16 = (const uint16_t*)inP + y * width * 3;
                    uint8_t* out8 = (uint8_t*)outP + y * rowByteCount;
                    for (int x = 0; x < width; ++x)
                    {
                        const uint16_t r10 = in16[0] >> 6;
                        const uint16_t g10 = in16[1] >> 6;
                        const uint16_t b10 = in16[2] >> 6;
                        out8[3] = b10 << 2;
                        out8[2] = (b10 >> 6) | (g10 << 4);
                        out8[1] = (g10 >> 4) | (r10 << 6);
                        out8[0] = r10 >> 2;

                        in16 += 3;
                        out8 += 4;
                    }
                }
                break;
            case PixelType::_10BitRGBXLE:
                for (int y = 0; y < height; ++y)
                {
                    const uint16_t* in16 = (const uint16_t*)inP + y * width * 3;
                    uint8_t* out8 = (uint8_t*)outP + y * rowByteCount;
                    for (int x = 0; x < width; ++x)
                    {
                        const uint16_t r10 = in16[0] >> 6;
                        const uint16_t g10 = in16[1] >> 6;
                        const uint16_t b10 = in16[2] >> 6;
                        out8[0] = b10 << 2;
                        out8[1] = (b10 >> 6) | (g10 << 4);
                        out8[2] = (g10 >> 4) | (r10 << 6);
                        out8[3] = r10 >> 2;

                        in16 += 3;
                        out8 += 4;
                    }
                }
                break;
            case PixelType::_12BitRGB:
                for (int y = 0; y < height; ++y)
                {
                    const uint16_t* in16 = (const uint16_t*)inP + y * width * 3;
                    uint8_t* out8 = (uint8_t*)outP + y * rowByteCount;
                    for (int x = 0; x < width; x += 8)
                    {
                        uint16_t r12 = in16[0] >> 4;
                        uint16_t g12 = in16[1] >> 4;
                        uint16_t b12 = in16[2] >> 4;
                        out8[0 + (3 - 0)] = r12;
                        out8[0 + (3 - 1)] = r12 >> 8;
                        out8[0 + (3 - 1)] |= g12 << 4;
                        out8[0 + (3 - 2)] = g12 >> 4;
                        out8[0 + (3 - 3)] = b12;
                        out8[4 + (3 - 0)] = b12 >> 8;

                        r12 = in16[3] >> 4;
                        g12 = in16[4] >> 4;
                        b12 = in16[5] >> 4;
                        out8[4 + (3 - 0)] |= r12 << 4;
                        out8[4 + (3 - 1)] = r12 >> 4;
                        out8[4 + (3 - 2)] = g12;
                        out8[4 + (3 - 3)] = g12 >> 8;
                        out8[4 + (3 - 3)] |= b12 << 4;
                        out8[8 + (3 - 0)] = b12 >> 4;

                        r12 = in16[6] >> 4;
                        g12 = in16[7] >> 4;
                        b12 = in16[8] >> 4;
                        out8[8 + (3 - 1)] = r12;
                        out8[8 + (3 - 2)] = r12 >> 8;
                        out8[8 + (3 - 2)] |= g12 << 4;
                        out8[8 + (3 - 3)] = g12 >> 4;
                        out8[12 + (3 - 0)] = b12;
                        out8[12 + (3 - 1)] = b12 >> 8;

                        r12 = in16[9] >> 4;
                        g12 = in16[10] >> 4;
                        b12 = in16[11] >> 4;
                        out8[12 + (3 - 1)] |= r12 << 4;
                        out8[12 + (3 - 2)] = r12 >> 4;
                        out8[12 + (3 - 3)] = g12;
                        out8[16 + (3 - 0)] = g12 >> 8;
                        out8[16 + (3 - 0)] |= b12 << 4;
                        out8[16 + (3 - 1)] = b12 >> 4;

                        r12 = in16[12] >> 4;
                        g12 = in16[13] >> 4;
                        b12 = in16[14] >> 4;
                        out8[16 + (3 - 2)] = r12;
                        out8[16 + (3 - 3)] = r12 >> 8;
                        out8[16 + (3 - 3)] |= g12 << 4;
                        out8[20 + (3 - 0)] = g12 >> 4;
                        out8[20 + (3 - 1)] = b12;
                        out8[20 + (3 - 2)] = b12 >> 8;

                        r12 = in16[15] >> 4;
                        g12 = in16[16] >> 4;
                        b12 = in16[17] >> 4;
                        out8[20 + (3 - 2)] |= r12 << 4;
                        out8[20 + (3 - 3)] = r12 >> 4;
                        out8[24 + (3 - 0)] = g12;
                        out8[24 + (3 - 1)] = g12 >> 8;
                        out8[24 + (3 - 1)] |= b12 << 4;
                        out8[24 + (3 - 2)] = b12 >> 4;

                        r12 = in16[18] >> 4;
                        g12 = in16[19] >> 4;
                        b12 = in16[20] >> 4;
                        out8[24 + (3 - 3)] = r12;
                        out8[28 + (3 - 0)] = r12 >> 8;
                        out8[28 + (3 - 0)] |= g12 << 4;
                        out8[28 + (3 - 1)] = g12 >> 4;
                        out8[28 + (3 - 2)] = b12;
                        out8[28 + (3 - 3)] = b12 >> 8;

                        r12 = in16[21] >> 4;
                        g12 = in16[22] >> 4;
                        b12 = in16[23] >> 4;
                        out8[28 + (3 - 3)] |= r12 << 4;
                        out8[32 + (3 - 0)] = r12 >> 4;
                        out8[32 + (3 - 1)] = g12;
                        out8[32 + (3 - 2)] = g12 >> 8;
                        out8[32 + (3 - 2)] |= b12 << 4;
                        out8[32 + (3 - 3)] = b12 >> 4;

                        in16 += 8 * 3;
                        out8 += 36;
                    }
                }
                break;
            case PixelType::_12BitRGBLE:
                for (int y = 0; y < height; ++y)
                {
                    const uint16_t* in16 = (const uint16_t*)inP + y * width * 3;
                    uint8_t* out8 = (uint8_t*)outP + y * rowByteCount;
                    for (int x = 0; x < width; x += 8)
                    {
                        uint16_t r12 = in16[0] >> 4;
                        uint16_t g12 = in16[1] >> 4;
                        uint16_t b12 = in16[2] >> 4;
                        out8[0 + 0] = r12;
                        out8[0 + 1] = r12 >> 8;
                        out8[0 + 1] |= g12 << 4;
                        out8[0 + 2] = g12 >> 4;
                        out8[0 + 3] = b12;
                        out8[4 + 0] = b12 >> 8;

                        r12 = in16[3] >> 4;
                        g12 = in16[4] >> 4;
                        b12 = in16[5] >> 4;
                        out8[4 + 0] |= r12 << 4;
                        out8[4 + 1] = r12 >> 4;
                        out8[4 + 2] = g12;
                        out8[4 + 3] = g12 >> 8;
                        out8[4 + 3] |= b12 << 4;
                        out8[8 + 0] = b12 >> 4;

                        r12 = in16[6] >> 4;
                        g12 = in16[7] >> 4;
                        b12 = in16[8] >> 4;
                        out8[8 + 1] = r12;
                        out8[8 + 2] = r12 >> 8;
                        out8[8 + 2] |= g12 << 4;
                        out8[8 + 3] = g12 >> 4;
                        out8[12 + 0] = b12;
                        out8[12 + 1] = b12 >> 8;

                        r12 = in16[9] >> 4;
                        g12 = in16[10] >> 4;
                        b12 = in16[11] >> 4;
                        out8[12 + 1] |= r12 << 4;
                        out8[12 + 2] = r12 >> 4;
                        out8[12 + 3] = g12;
                        out8[16 + 0] = g12 >> 8;
                        out8[16 + 0] |= b12 << 4;
                        out8[16 + 1] = b12 >> 4;

                        r12 = in16[12] >> 4;
                        g12 = in16[13] >> 4;
                        b12 = in16[14] >> 4;
                        out8[16 + 2] = r12;
                        out8[16 + 3] = r12 >> 8;
                        out8[16 + 3] |= g12 << 4;
                        out8[20 + 0] = g12 >> 4;
                        out8[20 + 1] = b12;
                        out8[20 + 2] = b12 >> 8;

                        r12 = in16[15] >> 4;
                        g12 = in16[16] >> 4;
                        b12 = in16[17] >> 4;
                        out8[20 + 2] |= r12 << 4;
                        out8[20 + 3] = r12 >> 4;
                        out8[24 + 0] = g12;
                        out8[24 + 1] = g12 >> 8;
                        out8[24 + 1] |= b12 << 4;
                        out8[24 + 2] = b12 >> 4;

                        r12 = in16[18] >> 4;
                        g12 = in16[19] >> 4;
                        b12 = in16[20] >> 4;
                        out8[24 + 3] = r12;
                        out8[28 + 0] = r12 >> 8;
                        out8[28 + 0] |= g12 << 4;
                        out8[28 + 1] = g12 >> 4;
                        out8[28 + 2] = b12;
                        out8[28 + 3] = b12 >> 8;

                        r12 = in16[21] >> 4;
                        g12 = in16[22] >> 4;
                        b12 = in16[23] >> 4;
                        out8[28 + 3] |= r12 << 4;
                        out8[32 + 0] = r12 >> 4;
                        out8[32 + 1] = g12;
                        out8[32 + 2] = g12 >> 8;
                        out8[32 + 2] |= b12 << 4;
                        out8[32 + 3] = b12 >> 4;

                        in16 += 8 * 3;
                        out8 += 36;
                    }
                }
                break;
            case PixelType::_8BitI420:
            {
                const size_t stride = width * sizeof(uint8_t);
                uint8_t* in_y = (uint8_t*)inP;
                uint8_t* in_v = in_y + stride * height;
                uint8_t* in_u = in_v + (stride / 2) * (height / 2);

                uint8_t* out_y = (uint8_t*)outP;
                uint8_t* out_v = in_y + stride * height;
                uint8_t* out_u = in_v + (stride / 2) * (height / 2);

                // Copy Y plane
                memcpy(out_y, in_y, height * stride);

                for (int i = 0; i < (stride / 2) * (height / 2); ++i)
                {
                    out_u[i] = in_v[i];
                    out_v[i] = in_u[i];
                }
                break;
            }
            case PixelType::_8BitUYVA:
            {
                const size_t stride = width * 2; // UYVY is tightly packed
                uint8_t* rgba = (uint8_t*)inP;
                uint8_t* p_uyvy = (uint8_t*)outP;
                uint8_t* p_alpha = p_uyvy + stride * height;

                YUVCoefficients8bits coeffs =
                    getYUVCoefficients8bits(width, height);
                uint8_t kr = coeffs.kr, kg = coeffs.kg, kb = coeffs.kb;

                for (int y = 0; y < height; ++y)
                {
                    for (int x = 0; x < width; x += 2)
                    {
                        // Load RGBA values
                        uint8_t r0 = rgba[(y * width + x) * 4 + 0];
                        uint8_t g0 = rgba[(y * width + x) * 4 + 1];
                        uint8_t b0 = rgba[(y * width + x) * 4 + 2];
                        uint8_t a0 = rgba[(y * width + x) * 4 + 3];
                        uint8_t r1 = rgba[(y * width + x + 1) * 4 + 0];
                        uint8_t g1 = rgba[(y * width + x + 1) * 4 + 1];
                        uint8_t b1 = rgba[(y * width + x + 1) * 4 + 2];
                        uint8_t a1 = rgba[(y * width + x + 1) * 4 + 3];

                        // Convert to YUV (Integer Calculation with Proper
                        // Scaling)
                        uint8_t y0 = std::min(
                            235,
                            std::max(
                                16, ((kr * r0 + kg * g0 + kb * b0 + 128) >> 8) +
                                        16));
                        uint8_t y1 = std::min(
                            235,
                            std::max(
                                16, ((kr * r1 + kg * g1 + kb * b1 + 128) >> 8) +
                                        16));
                        int u = std::min(
                            240, std::max(
                                     16, ((((int)(-kr) * r0 - kg * g0 +
                                            (255 - kb) * b0 + 128) >>
                                           8) +
                                          (((int)(-kr) * r1 - kg * g1 +
                                            (255 - kb) * b1 + 128) >>
                                           8)) / 2 +
                                             128));
                        int v = std::min(
                            240,
                            std::max(
                                16,
                                ((((255 - kr) * r0 - kg * g0 - kb * b0 + 128) >>
                                  8) +
                                 (((255 - kr) * r1 - kg * g1 - kb * b1 + 128) >>
                                  8)) / 2 +
                                    128));
                        // Store UYVY format
                        int uyvy_index = y * stride + x * 2;
                        p_uyvy[uyvy_index + 0] = (uint8_t)u;
                        p_uyvy[uyvy_index + 1] = y0;
                        p_uyvy[uyvy_index + 2] = (uint8_t)v;
                        p_uyvy[uyvy_index + 3] = y1;

                        // Store Alpha plane
                        int alpha_index = y * stride / 2 + x;
                        p_alpha[alpha_index + 0] = a0;
                        p_alpha[alpha_index + 1] = a1;
                    }
                }
                break;
            }
            case PixelType::_8BitBGRX:
                for (int y = 0; y < height; ++y)
                {
                    const uint8_t* in8 = (const uint8_t*)inP + y * width * 3;
                    uint8_t* out8 = (uint8_t*)outP + y * rowByteCount;
                    for (int x = 0; x < width; ++x)
                    {
                        out8[0] = in8[2];
                        out8[1] = in8[1];
                        out8[2] = in8[0];
                        out8[3] = 255;

                        in8 += 3;
                        out8 += 4;
                    }
                }
                break;
            case PixelType::_16BitP216:
            {
                // This is a 4:2:2 buffer in semi-planar format with full 16bpp
                // color precision. This is formed from two buffers in memory,
                // the first is a 16bpp luminance buffer and the second is a
                // buffer of U,V pairs in memory. This can be considered as a
                // 16bpp version of NV12. For instance, if you have an image
                // with p_data and stride, then the planes are located as
                // follows: uint16_t *p_y = (uint16_t*)p_data; uint16_t *p_uv =
                // (uint16_t*)(p_data + stride*height); As a matter of
                // illustration, a completely packed image would have stride as
                // width*sizeof(uint16_t).
                uint16_t* p_y = reinterpret_cast<uint16_t*>(outP);
                uint16_t* p_uv =
                    reinterpret_cast<uint16_t*>(p_y + width * height);

                for (int y = 0; y < height; ++y)
                {
                    const uint16_t* rgba_row =
                        reinterpret_cast<const uint16_t*>(
                            reinterpret_cast<const uint8_t*>(inP) +
                            y * rowByteCount);

                    for (int x = 0; x < width; ++x)
                    {
                        // Extract RGBA components (assuming RGBA order)
                        uint16_t r = rgba_row[3 * x + 0];
                        uint16_t g = rgba_row[3 * x + 1];
                        uint16_t b = rgba_row[3 * x + 2];

                        // Integer based YUV conversion. BT.709 example.
                        int32_t y_val = (77 * r + 150 * g + 29 * b) >>
                                        8; // Right shift by 8 == divide by 256.
                        int32_t u_val =
                            (((-43 * r - 84 * g + 127 * b) >> 8) + 32768);
                        int32_t v_val =
                            (((127 * r - 106 * g - 21 * b) >> 8) + 32768);

                        // Clamping
                        y_val = std::clamp(y_val, 0, 65535);
                        u_val = std::clamp(u_val, 0, 65535);
                        v_val = std::clamp(v_val, 0, 65535);

                        const int index = y * width + x;
                        p_y[index] = static_cast<uint16_t>(y_val);

                        if (x % 2 == 0)
                        {
                            p_uv[index] = static_cast<uint16_t>(u_val);
                            p_uv[index + 1] = static_cast<uint16_t>(v_val);
                        }
                    }
                }
                break;
            }
            case PixelType::_16BitPA16:
            {
                // This is a 4:2:2 buffer in semi-planar format with full 16bpp
                // color precision. This is formed from two buffers in memory,
                // the first is a 16bpp luminance buffer and the second is a
                // buffer of U,V pairs in memory. This can be considered as a
                // 16bpp version of NV12. For instance, if you have an image
                // with p_data and stride, then the planes are located as
                // follows: uint16_t *p_y = (uint16_t*)p_data; uint16_t *p_uv =
                // (uint16_t*)(p_data + stride*height); uint16_t *p_alpha =
                // (uint16_t*)(p_uv + stride*height); As a matter of
                // illustration, a completely packed image would have stride as
                // width*sizeof(uint16_t).
                uint16_t* p_y = reinterpret_cast<uint16_t*>(outP);
                uint16_t* p_uv =
                    reinterpret_cast<uint16_t*>(p_y + width * height);
                uint16_t* p_alpha =
                    reinterpret_cast<uint16_t*>(p_uv + width * height);

                for (int y = 0; y < height; ++y)
                {
                    const uint16_t* rgba_row =
                        reinterpret_cast<const uint16_t*>(
                            reinterpret_cast<const uint8_t*>(inP) +
                            y * rowByteCount);

                    for (int x = 0; x < width; ++x)
                    {
                        uint16_t r = rgba_row[4 * x + 0];
                        uint16_t g = rgba_row[4 * x + 1];
                        uint16_t b = rgba_row[4 * x + 2];
                        uint16_t a = rgba_row[4 * x + 3];

                        // Integer based YUV conversion. BT.709 coefficients
                        // should be fine.
                        int32_t y_val = (77 * r + 150 * g + 29 * b) >>
                                        8; // Right shift by 8 == divide by 256.
                        int32_t u_val =
                            (((-43 * r - 84 * g + 127 * b) >> 8) + 32768);
                        int32_t v_val =
                            (((127 * r - 106 * g - 21 * b) >> 8) + 32768);

                        // Clamping
                        y_val = std::clamp(y_val, 0, 65535);
                        u_val = std::clamp(u_val, 0, 65535);
                        v_val = std::clamp(v_val, 0, 65535);

                        const int index = y * width + x;
                        p_y[index] = static_cast<uint16_t>(y_val);
                        p_alpha[index] = a;

                        if (x % 2 == 0)
                        {
                            p_uv[index] = static_cast<uint16_t>(u_val);
                            p_uv[index + 1] = static_cast<uint16_t>(v_val);
                        }
                    }
                }
                break;
            }
            case PixelType::_8BitRGBX:
                for (int y = 0; y < height; ++y)
                {
                    const uint8_t* in8 = (const uint8_t*)inP + y * width * 3;
                    uint8_t* out8 = (uint8_t*)outP + y * rowByteCount;
                    for (int x = 0; x < width; ++x)
                    {
                        out8[0] = in8[0];
                        out8[1] = in8[1];
                        out8[2] = in8[2];
                        out8[3] = 255;

                        in8 += 3;
                        out8 += 4;
                    }
                }
                break;
            case PixelType::_8BitBGRA:
            case PixelType::_8BitRGBA:
            case PixelType::_8BitYUV:
                memcpy(outP, inP, getDataByteCount(size, pixelType));
                break;
            default:
                throw std::runtime_error("Unhandled output device pixel type");
                break;
            }
        }

    } // namespace device
} // namespace tl

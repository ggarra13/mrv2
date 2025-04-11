// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlDevice/NDI/NDIUtil.h>

#include <array>

namespace tl
{
    namespace ndi
    {
        std::string FourCCString(const NDIlib_FourCC_video_type_e type)
        {
            std::string out;
            switch (type)
            {
            case NDIlib_FourCC_video_type_UYVY:
                out = "UYVY";
                break;
            case NDIlib_FourCC_video_type_UYVA:
                out = "UYVA";
                break;
            case NDIlib_FourCC_video_type_P216:
                out = "P216";
                break;
            case NDIlib_FourCC_video_type_PA16:
                out = "PA16";
                break;
            case NDIlib_FourCC_video_type_YV12:
                out = "YV12";
                break;
            case NDIlib_FourCC_video_type_I420:
                out = "I420";
                break;
            case NDIlib_FourCC_video_type_NV12:
                out = "NV12";
                break;
            case NDIlib_FourCC_video_type_BGRA:
                out = "BGRA";
                break;
            case NDIlib_FourCC_video_type_BGRX:
                out = "BGRX";
                break;
            case NDIlib_FourCC_video_type_RGBA:
                out = "RGBA";
                break;
            case NDIlib_FourCC_video_type_RGBX:
                out = "RGBX";
                break;
            case NDIlib_FourCC_video_type_max:
            default:
                out = "Unknown FourCC";
                break;
            }
            return out;
        }

        NDIlib_FourCC_video_type_e toNDI(device::PixelType value)
        {
            const std::array<
                NDIlib_FourCC_video_type_e,
                static_cast<size_t>(device::PixelType::Count)>
                data = {
                    NDIlib_FourCC_video_type_max,
                    NDIlib_FourCC_video_type_BGRA, // Planar 8bit, 4:4:4:4 video
                                                   // format.
                    NDIlib_FourCC_type_YV12,       // Planar YUV
                    NDIlib_FourCC_video_type_max,  // _10BitRGB,
                    NDIlib_FourCC_video_type_max,  // _10BitRGBX,
                    NDIlib_FourCC_video_type_max,  // _10BitRGBXLE
                    // NDIlib_FourCC_video_type_max,  // _10BitYUV,
                    NDIlib_FourCC_video_type_max,  // _12BitRGB,
                    NDIlib_FourCC_video_type_max,  // _12BitRGBLE
                    NDIlib_FourCC_video_type_UYVA, // 4:2:2:4 (yuv + alpha in 8
                                                   // bps)
                    NDIlib_FourCC_video_type_P216, // 4:2:2 in 16bpp
                    NDIlib_FourCC_video_type_PA16, // 4:2:2:4 (yuv + alpha in 16
                                                   // bps)
                    NDIlib_FourCC_video_type_I420, // 4:2:0 (as YV12 reversed)
                    NDIlib_FourCC_video_type_BGRX, // 8 bit, 4:4:4 (blue, green,
                                                   // red, 255 packed in 32bits
                    NDIlib_FourCC_video_type_RGBA, // 4:4:4:4 red, green, blue,
                                                   // alpha
                    NDIlib_FourCC_video_type_RGBX, // 4:4:4 red, green, blue,
                                                   // 255
                };
            return data[static_cast<size_t>(value)];
        }

        device::PixelType fromNDI(NDIlib_FourCC_video_type_e value)
        {
            device::PixelType out = device::PixelType::None;
            switch (value)
            {
            case NDIlib_FourCC_video_type_BGRA:
                out = device::PixelType::_8BitBGRA;
                break;
            case NDIlib_FourCC_type_YV12:
                out = device::PixelType::_8BitYUV;
                break;
            case NDIlib_FourCC_video_type_UYVA:
                out = device::PixelType::_8BitUYVA;
                break;
            case NDIlib_FourCC_video_type_P216:
                out = device::PixelType::_16BitP216;
                break;
            case NDIlib_FourCC_video_type_PA16:
                out = device::PixelType::_16BitPA16;
                break;
            case NDIlib_FourCC_video_type_I420:
                out = device::PixelType::_8BitI420;
                break;
            case NDIlib_FourCC_video_type_BGRX:
                out = device::PixelType::_8BitBGRX;
            case NDIlib_FourCC_video_type_RGBA:
                out = device::PixelType::_8BitRGBA;
            case NDIlib_FourCC_video_type_RGBX:
                out = device::PixelType::_8BitRGBX;
                break;
            default:
                throw std::runtime_error(
                    "fromNDI device::PixelType unsupported");
                break;
            }
            return out;
        }

        bool validSize(const math::Size2i& size)
        {
            bool out = false;
            if (size.w % 2 == 0)
                out = true;
            if (size.w == 1920 && size.h == 1080)
                out = true;
            return out;
        }

        device::PixelType getOutputType(device::PixelType value)
        {
            device::PixelType out = device::PixelType::None;
            switch (value)
            {
            case device::PixelType::_8BitBGRA:
            case device::PixelType::_8BitUYVA:
            case device::PixelType::_16BitP216:
            case device::PixelType::_16BitPA16:
            case device::PixelType::_8BitI420:
            case device::PixelType::_8BitBGRX:
            case device::PixelType::_8BitRGBA:
            case device::PixelType::_8BitRGBX:
                out = value;
                break;
            // case device::PixelType::_10BitYUV:
            //     out = device::PixelType::_10BitRGBXLE;
            //     break;
            case device::PixelType::_8BitYUV: // \@bug: this one is broken on
                                              // NDI
                out = device::PixelType::_8BitUYVA;
                break;
            case device::PixelType::_10BitRGB:
            case device::PixelType::_10BitRGBX:
            case device::PixelType::_10BitRGBXLE:
            case device::PixelType::_12BitRGB:
            case device::PixelType::_12BitRGBLE:
            case device::PixelType::None:
            case device::PixelType::Count:
                throw std::runtime_error(
                    "getOutputType device::PixelType unsupported");
                break;
            }
            return out;
        }
    } // namespace ndi
} // namespace tl

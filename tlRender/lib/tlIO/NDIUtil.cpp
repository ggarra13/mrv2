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
    } // namespace ndi
} // namespace tl

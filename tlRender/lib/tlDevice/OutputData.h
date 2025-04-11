// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Video.h>

#include <tlCore/HDR.h>
#include <tlCore/Size.h>
#include <tlCore/Time.h>

namespace tl
{
    namespace device
    {
        //! Display mode.
        struct DisplayMode
        {
            std::string name;
            math::Size2i size;
            otime::RationalTime frameRate;

            bool operator==(const DisplayMode&) const;
        };

        //! Pixel types.
        //!
        //! \bug Disable 10-bit YUV since the BMD conversion function
        //! shows artifacts.
        enum class PixelType {
            None,
            _8BitBGRA, // BMD and NDI
            _8BitYUV,  // BMD and NDI
            // These are BMD formats
            _10BitRGB,    // BMD
            _10BitRGBX,   // BMD
            _10BitRGBXLE, // BMD
            //_10BitYUV,
            _12BitRGB,   // BMD
            _12BitRGBLE, // BMD
            // These are NDI formats
            _8BitUYVA,  // NDI
            _16BitP216, // NDI
            _16BitPA16, // NDI
            _8BitI420,  // NDI
            _8BitBGRX,  // NDI
            _8BitRGBA,  // NDI
            _8BitRGBX,  // NDI
            Count,
            First = None
        };
        TLRENDER_ENUM(PixelType);
        TLRENDER_ENUM_SERIALIZE(PixelType);

        //! Get the color buffer type.
        image::PixelType getColorBuffer(PixelType value);

        //! Get the pack pixels buffer size.
        size_t getPackPixelsSize(const math::Size2i&, PixelType);

        //! Get the number of bytes used to store a row of pixel data.
        size_t getRowByteCount(int, PixelType);

        //! Get the number of bytes used to storepixel data.
        size_t getDataByteCount(const math::Size2i&, PixelType);

        //! Copy the pack pixels.
        void copyPackPixels(void*, const void*, const math::Size2i&, PixelType);

        //! Device information.
        struct DeviceInfo
        {
            std::string name;
            std::vector<DisplayMode> displayModes;
            std::vector<PixelType> pixelTypes;
            size_t minVideoPreroll = 0;
            bool hdrMetaData = false;
            size_t maxAudioChannels = 0;

            bool operator==(const DeviceInfo&) const;
            bool operator!=(const DeviceInfo&) const;
        };

        //! Device options.
        enum class Option {
            None,
            _444SDIVideoOutput,

            Count,
            First = None
        };
        TLRENDER_ENUM(Option);
        TLRENDER_ENUM_SERIALIZE(Option);

        //! Device boolean options.
        typedef std::map<Option, bool> BoolOptions;

        //! Device configuration.
        struct DeviceConfig
        {
            int deviceIndex = -1;
            int displayModeIndex = -1;
            PixelType pixelType = PixelType::None;
            BoolOptions boolOptions;
            bool noAudio = false;
            bool noMetadata = false;

            bool operator==(const DeviceConfig&) const;
            bool operator!=(const DeviceConfig&) const;
        };

        //! HDR mode.
        enum class HDRMode {
            None,
            FromFile,
            Custom,

            Count,
            First = None
        };
        TLRENDER_ENUM(HDRMode);
        TLRENDER_ENUM_SERIALIZE(HDRMode);

        //! Get HDR data from timeline video data.
        std::shared_ptr<image::HDRData> getHDRData(const timeline::VideoData&);
    } // namespace device
} // namespace tl

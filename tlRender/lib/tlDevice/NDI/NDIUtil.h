// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <cstddef>

#include <tlDevice/NDI/NDI.h>
#include <tlDevice/OutputData.h>

#include <tlCore/Image.h>

#include <string>

namespace tl
{
    namespace ndi
    {
        //! Convert to NDI.
        NDIlib_FourCC_video_type_e toNDI(device::PixelType);

        //! Convert from device::PixelType.
        device::PixelType fromNDI(NDIlib_FourCC_audio_type_e);

        bool validSize(const math::Size2i& size);

        // //! Get a label.
        // std::string getVideoConnectionLabel(BMDVideoConnection);

        // //! Get a label.
        // std::string getAudioConnectionLabel(BMDAudioConnection);

        // //! Get a label.
        // std::string getDisplayModeLabel(BMDDisplayMode);

        // //! Get a label.
        // std::string getPixelFormatLabel(BMDPixelFormat);

        // //! Get a label.
        // std::string
        // getOutputFrameCompletionResultLabel(BMDOutputFrameCompletionResult);

        //! Get the output pixel type.
        device::PixelType getOutputType(device::PixelType);

    } // namespace ndi
} // namespace tl

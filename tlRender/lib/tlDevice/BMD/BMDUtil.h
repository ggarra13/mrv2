// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDData.h>

#include <tlGL/GL.h>

#include <tlCore/Image.h>

#ifndef NOMINMAX
#    define NOMINMAX
#endif // NOMINMAX
#include "DeckLinkAPI.h"

#include <string>

namespace tl
{
    namespace bmd
    {
        //! Convert to BMD.
        BMDPixelFormat toBMD(PixelType);

        //! Convert from BMD.
        PixelType fromBMD(BMDPixelFormat);

        //! Get a label.
        std::string getVideoConnectionLabel(BMDVideoConnection);

        //! Get a label.
        std::string getAudioConnectionLabel(BMDAudioConnection);

        //! Get a label.
        std::string getDisplayModeLabel(BMDDisplayMode);

        //! Get a label.
        std::string getPixelFormatLabel(BMDPixelFormat);

        //! Get a label.
        std::string
            getOutputFrameCompletionResultLabel(BMDOutputFrameCompletionResult);

        //! Get the output pixel type.
        PixelType getOutputType(PixelType);

        //! Get the color buffer type.
        image::PixelType getColorBuffer(PixelType);

        //! Get the pack pixels buffer size.
        size_t getPackPixelsSize(const math::Size2i&, PixelType);

        //! Get the pack pixels format.
        GLenum getPackPixelsFormat(PixelType);

        //! Get the pack pixels type.
        GLenum getPackPixelsType(PixelType);

        //! Get the pack pixels alignment.
        GLint getPackPixelsAlign(PixelType);

        //! Get the pack pixels endian byte swap.
        GLint getPackPixelsSwap(PixelType);

        //! Copy the pack pixels.
        void copyPackPixels(const void*, void*, const math::Size2i&, PixelType);
    } // namespace bmd
} // namespace tl

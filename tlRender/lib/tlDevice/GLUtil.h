#pragma once

#include <tlGL/GL.h>

#include <tlDevice/OutputData.h>

namespace tl
{
    namespace device
    {
        //! Get the pack pixels format.
        GLenum getPackPixelsFormat(PixelType);

        //! Get the pack pixels type.
        GLenum getPackPixelsType(PixelType);

        //! Get the pack pixels alignment.
        GLint getPackPixelsAlign(PixelType);

        //! Get the pack pixels endian byte swap.
        GLint getPackPixelsSwap(PixelType);
    } // namespace device
} // namespace tl

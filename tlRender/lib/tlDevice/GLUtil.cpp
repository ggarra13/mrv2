// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <tlDevice/OutputData.h>

#include <tlGL/GL.h>

namespace tl
{
    namespace device
    {
        GLenum getPackPixelsFormat(PixelType value)
        {
            const std::array<GLenum, static_cast<size_t>(PixelType::Count)>
                data = {
                    GL_NONE,
                    GL_BGRA, // BGRA
                    GL_BGR,  // YUV
                    GL_RGB,  // RGB
                    GL_RGB,  // RGBX
                    GL_RGB,  // RGBXLE
                    // GL_RGBA, // YUV
                    GL_RGB,  // RGB
                    GL_RGB,  // RGBLE
                    GL_RGBA, // YUVA
                    GL_RGB,  // YUV
                    GL_RGBA, // YUVA
                    GL_RGB,  // YUV
                    GL_RGB,  // BRGX
                    GL_RGBA, // RGBA
                    GL_RGB,  // RGBX
                };
            return data[static_cast<size_t>(value)];
        }

        GLenum getPackPixelsType(PixelType value)
        {
            const std::array<GLenum, static_cast<size_t>(PixelType::Count)>
                data = {
                    GL_NONE,
                    GL_UNSIGNED_BYTE,  // 8Bit
                    GL_UNSIGNED_BYTE,  // 8Bit
                    GL_UNSIGNED_SHORT, // 10Bit
                    GL_UNSIGNED_SHORT, // 10Bit
                    GL_UNSIGNED_SHORT, // 10Bit
                    // GL_UNSIGNED_INT_10_10_10_2,  // 10Bit
                    GL_UNSIGNED_SHORT, // 12Bit
                    GL_UNSIGNED_SHORT, // 12Bit
                    GL_UNSIGNED_BYTE,  // 8Bit
                    GL_UNSIGNED_SHORT, // 16Bit
                    GL_UNSIGNED_SHORT, // 16Bit
                    GL_UNSIGNED_BYTE,  // 8Bit
                    GL_UNSIGNED_BYTE,  // 8Bit
                    GL_UNSIGNED_BYTE,  // 8Bit
                    GL_UNSIGNED_BYTE,  // 8Bit
                };
            return data[static_cast<size_t>(value)];
        }

        GLint getPackPixelsAlign(PixelType value)
        {
            const std::array<GLint, static_cast<size_t>(PixelType::Count)>
                data = {
                    0,
                    4,
                    4,
                    1,
                    1,
                    1,
                    //! \bug OpenGL only allows alignment values of 1, 2, 4,
                    //! and 8.
                    // 8, // 128,
                    1,
                    1,
                    // These are NDI formats
                    2,
                    8,
                    1,
                    1,
                    4,
                    4,
                    4,
                };
            return data[static_cast<size_t>(value)];
        }

        GLint getPackPixelsSwap(PixelType value)
        {
            const std::array<GLint, static_cast<size_t>(PixelType::Count)>
                data = {
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE,
                    // GL_FALSE,
                    GL_FALSE,
                    GL_FALSE,
                    // These are NDI formats
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE,
                    GL_FALSE,
                };
            return data[static_cast<size_t>(value)];
        }
    } // namespace device
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <cstdint>
#include <cstring> // For memcpy

#include <Imath/half.h>

namespace mrv
{
    inline void flipImageInY(
        uint8_t* pixels, const size_t width, const size_t height,
        const int depth)
    {
        const size_t rowSize = width * depth;
        uint8_t* tempRow =
            new uint8_t[rowSize]; // Temporary buffer for row swapping

        for (size_t y = 0; y < height / 2; ++y)
        {
            uint8_t* topRow = pixels + y * rowSize;
            uint8_t* bottomRow = pixels + (height - y - 1) * rowSize;

            // Swap entire rows using memcpy
            std::memcpy(tempRow, topRow, rowSize);
            std::memcpy(topRow, bottomRow, rowSize);
            std::memcpy(bottomRow, tempRow, rowSize);
        }

        delete[] tempRow; // Free the temporary buffer
    }

    inline void flipImageInY(
        uint8_t* outputPixels, const uint8_t* inputPixels, const size_t width,
        const size_t height, const int depth)
    {
        // Calculate the size of a row of pixels (in bytes)
        size_t rowSize = width * depth;

        // Loop through the image height
        for (size_t y = 0; y < height; ++y)
        {
            // Calculate the indices of the rows in source and destination
            // buffers
            size_t topInputRow = y * rowSize;
            size_t topOutputRow =
                (height - y - 1) * rowSize; // Start from bottom in output

            // Copy a row from top to bottom in the output buffer
            memcpy(
                outputPixels + topOutputRow, inputPixels + topInputRow,
                rowSize);
        }
    }

    inline void flipImageInY(
        float* outputPixels, const uint8_t* inputPixels, const size_t width,
        const size_t height, const int depth)
    {
        const size_t rowSize = width * depth;

        for (size_t y = 0; y < height; ++y)
        {
            const size_t inputRow = y * rowSize;
            const size_t outputRow = (height - y - 1) * rowSize;

            // Normalize and flip vertically
            for (size_t i = 0; i < rowSize; ++i)
            {
                outputPixels[outputRow + i] =
                    static_cast<float>(inputPixels[inputRow + i]) / 255.0f;
            }
        }
    }

    inline void flipImageInY(
        Imath::half* outputPixels, const uint8_t* inputPixels,
        const size_t width, const size_t height, const int depth)
    {
        const size_t rowSize = width * depth;

        for (size_t y = 0; y < height; ++y)
        {
            const size_t inputRow = y * rowSize;
            const size_t outputRow = (height - y - 1) * rowSize;

            // Normalize and flip vertically
            for (size_t i = 0; i < rowSize; ++i)
            {
                outputPixels[outputRow + i] =
                    static_cast<float>(inputPixels[inputRow + i]) / 255.0f;
            }
        }
    }

} // namespace mrv

// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

namespace mrv
{

    inline void flipImageInY(
        uint8_t* pixels, const size_t width, const size_t height,
        const int depth)
    {
        const size_t rowSize = width * depth;
        for (size_t y = 0; y < height / 2; ++y)
        {
            const size_t topRow = y * rowSize;
            const size_t bottomRow = (height - y - 1) * rowSize;

            for (size_t i = 0; i < rowSize; ++i)
            {
                const uint8_t temp = pixels[topRow + i];
                pixels[topRow + i] = pixels[bottomRow + i];
                pixels[bottomRow + i] = temp;
            }
        }
    }

    inline void flipImageInY(
        uint8_t* outputPixels, const uint8_t* inputPixels, const size_t width,
        const size_t height, const int depth)
    {
        // Calculate the size of a row of pixels (in bytes)
        size_t rowSize = width * depth;

        // Loop through half the image height
        for (size_t y = 0; y < height; ++y)
        {
            // Calculate the indices of the rows in source and destination
            // buffers
            size_t topInputRow = y * rowSize;
            size_t bottomInputRow = (height - y - 1) * rowSize;
            size_t topOutputRow =
                (height - y - 1) * rowSize; // Start from bottom in output

            // Copy a row from top to bottom in the output buffer
            memcpy(
                outputPixels + topOutputRow, inputPixels + topInputRow,
                rowSize);
        }
    }

} // namespace mrv

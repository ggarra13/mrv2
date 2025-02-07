// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <cstddef>
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

    inline void flipImageInYToHalf(
        Imath::half* outputPixels, const uint8_t* inputPixels,
        const size_t width, const size_t height, const int depth)
    {
        const size_t inputRowSize = width * depth;

        for (size_t y = 0; y < height; ++y)
        {
            const size_t inputRow = y * inputRowSize;
            const size_t outputRow = (height - y - 1) * width * depth;

            for (size_t x = 0; x < width; ++x)
            {
                for (int c = 0; c < depth; ++c)
                {
                    const size_t inputIdx = inputRow + x * depth + c;
                    const size_t outputIdx = outputRow + x * depth + c;

                    const float tmp =
                        static_cast<float>(inputPixels[inputIdx]) / 255.0f;
                    outputPixels[outputIdx] = static_cast<Imath::half>(tmp);
                }
            }
        }
    }

    inline void flipGBRImageInY(
        Imath::half* outputPixels, const uint8_t* inputPixels,
        const size_t width, const size_t height, const int depth)
    {
        const size_t inputRowSize = width * depth;

        for (size_t y = 0; y < height; ++y)
        {
            const size_t inputRow = y * inputRowSize;
            const size_t outputRow = (height - y - 1) * width * depth;

            for (size_t x = 0; x < width; ++x)
            {
                const size_t inputIdx = inputRow + x * depth;
                const size_t outputIdx = outputRow + x * depth;

                switch (depth)
                {
                case 1: // Grayscale
                    outputPixels[outputIdx] = static_cast<Imath::half>(
                        inputPixels[inputIdx] / 255.0f);
                    break;

                case 3: // BGR to RGB
                    outputPixels[outputIdx + 0] = static_cast<Imath::half>(
                        inputPixels[inputIdx + 2] / 255.0f); // Red
                    outputPixels[outputIdx + 1] = static_cast<Imath::half>(
                        inputPixels[inputIdx + 0] / 255.0f); // Blue
                    outputPixels[outputIdx + 2] = static_cast<Imath::half>(
                        inputPixels[inputIdx + 1] / 255.0f); // Green
                    break;

                case 4: // BGRA to RGBA
                    outputPixels[outputIdx + 0] = static_cast<Imath::half>(
                        inputPixels[inputIdx + 2] / 255.0f); // Red
                    outputPixels[outputIdx + 1] = static_cast<Imath::half>(
                        inputPixels[inputIdx + 0] / 255.0f); // Blue
                    outputPixels[outputIdx + 2] = static_cast<Imath::half>(
                        inputPixels[inputIdx + 1] / 255.0f); // Green
                    outputPixels[outputIdx + 3] = static_cast<Imath::half>(
                        inputPixels[inputIdx + 3] / 255.0f); // Alpha
                    break;

                default: // Fallback for unknown formats
                    for (int c = 0; c < depth; ++c)
                    {
                        outputPixels[outputIdx + c] = static_cast<Imath::half>(
                            inputPixels[inputIdx + c] / 255.0f);
                    }
                    break;
                }
            }
        }
    }

    inline void flipGBRImageInY(
        float* outputPixels, const uint8_t* inputPixels, const size_t width,
        const size_t height, const int depth)
    {
        const size_t inputRowSize = width * depth;

        for (size_t y = 0; y < height; ++y)
        {
            const size_t inputRow = y * inputRowSize;
            const size_t outputRow = (height - y - 1) * width * depth;

            for (size_t x = 0; x < width; ++x)
            {
                const size_t inputIdx = inputRow + x * depth;
                const size_t outputIdx = outputRow + x * depth;

                switch (depth)
                {
                case 1: // Grayscale
                    outputPixels[outputIdx] =
                        static_cast<float>(inputPixels[inputIdx] / 255.0f);
                    break;

                case 3: // BGR to RGB
                    outputPixels[outputIdx + 0] = static_cast<float>(
                        inputPixels[inputIdx + 2] / 255.0f); // Red
                    outputPixels[outputIdx + 1] = static_cast<float>(
                        inputPixels[inputIdx + 0] / 255.0f); // Green
                    outputPixels[outputIdx + 2] = static_cast<float>(
                        inputPixels[inputIdx + 1] / 255.0f); // Blue
                    break;

                case 4: // BGRA to RGBA
                    outputPixels[outputIdx + 0] = static_cast<float>(
                        inputPixels[inputIdx + 2] / 255.0f); // Red
                    outputPixels[outputIdx + 1] = static_cast<float>(
                        inputPixels[inputIdx + 0] / 255.0f); // Green
                    outputPixels[outputIdx + 2] = static_cast<float>(
                        inputPixels[inputIdx + 1] / 255.0f); // Blue
                    outputPixels[outputIdx + 3] = static_cast<float>(
                        inputPixels[inputIdx + 3] / 255.0f); // Alpha
                    break;

                default: // Fallback for unknown formats
                    for (int c = 0; c < depth; ++c)
                    {
                        outputPixels[outputIdx + c] = static_cast<float>(
                            inputPixels[inputIdx + c] / 255.0f);
                    }
                    break;
                }
            }
        }
    }

} // namespace mrv

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
    template<typename T>
    inline void flipImageInY(
        T* pixels, const size_t width, const size_t height,
        const int depth)
    {
        const size_t rowSize = width * depth * sizeof(T);
        T* tempRow = new T[rowSize]; // Temporary buffer for row swapping

        for (size_t y = 0; y < height / 2; ++y)
        {
            T* topRow = pixels + y * rowSize;
            T* bottomRow = pixels + (height - y - 1) * rowSize;

            // Swap entire rows using memcpy
            std::memcpy(tempRow, topRow, rowSize);
            std::memcpy(topRow, bottomRow, rowSize);
            std::memcpy(bottomRow, tempRow, rowSize);
        }

        delete[] tempRow; // Free the temporary buffer
    }

    inline void flipImageInY(
        uint8_t* outputPixels, const uint8_t* inputPixels, const size_t width,
        const size_t height, const int inDepth, const int outDepth)
    {
        const size_t inputRowSize = width * inDepth;
        memset(outputPixels, 0, width * height * outDepth);

        for (size_t y = 0; y < height; ++y)
        {
            const size_t inputRow = y * inputRowSize;
            const size_t outputRow = (height - y - 1) * width * outDepth;

            for (size_t x = 0; x < width; ++x)
            {
                for (int c = 0; c < outDepth; ++c)
                {
                    const size_t inputIdx = inputRow + x * inDepth + c;
                    const size_t outputIdx = outputRow + x * outDepth + c;

                    const float tmp =
                        static_cast<float>(inputPixels[inputIdx]) / 255.0f;
                    outputPixels[outputIdx] = static_cast<Imath::half>(tmp);
                }
            }
        }
    }

    inline void flipImageInY(
        float* outputPixels, const uint8_t* inputPixels, const size_t width,
        const size_t height, const int inDepth, const int outDepth)
    {
        const size_t inputRowSize = width * inDepth;
        memset(outputPixels, 0, width * height * outDepth * sizeof(float));

        for (size_t y = 0; y < height; ++y)
        {
            const size_t inputRow = y * inputRowSize;
            const size_t outputRow = (height - y - 1) * width * outDepth;

            for (size_t x = 0; x < width; ++x)
            {
                for (int c = 0; c < outDepth; ++c)
                {
                    const size_t inputIdx = inputRow + x * inDepth + c;
                    const size_t outputIdx = outputRow + x * outDepth + c;

                    const float tmp =
                        static_cast<float>(inputPixels[inputIdx]) / 255.0f;
                    outputPixels[outputIdx] = static_cast<Imath::half>(tmp);
                }
            }
        }
    }

    inline void flipImageInY(
        Imath::half* outputPixels, const uint8_t* inputPixels,
        const size_t width, const size_t height, const int inDepth,
        const int outDepth)
    {
        const size_t inputRowSize = width * inDepth;
        memset(outputPixels, 0, width * height * outDepth * sizeof(Imath::half));

        for (size_t y = 0; y < height; ++y)
        {
            const size_t inputRow = y * inputRowSize;
            const size_t outputRow = (height - y - 1) * width * outDepth;

            for (size_t x = 0; x < width; ++x)
            {
                for (int c = 0; c < outDepth; ++c)
                {
                    const size_t inputIdx = inputRow + x * inDepth + c;
                    const size_t outputIdx = outputRow + x * outDepth + c;

                    const float tmp =
                        static_cast<float>(inputPixels[inputIdx]) / 255.0f;
                    outputPixels[outputIdx] = static_cast<Imath::half>(tmp);
                }
            }
        }
    }

} // namespace mrv

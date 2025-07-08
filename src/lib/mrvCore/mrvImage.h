// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring> // For memcpy
#include <cstddef>
#include <limits>

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

/**
 * @brief Composites a source RGBA_U8 image over a destination RGBA image.
 *        From Gemini.
 *
 * This function implements the "normal" blend mode (porter-duff "over" operation).
 * It correctly blends the source image onto the destination image using the source
 * alpha channel.
 *
 * The blending formula used is:
 * C_out = C_src * A_src + C_dst * (1 - A_src)
 * A_out = A_src + A_dst * (1 - A_src)
 *
 * @tparam T The data type of the destination pixel components (e.g., uint8_t, uint16_t, float).
 * @param[in,out] dstPixels Pointer to the destination image buffer (RGBA).
 * @param[in] srcPixels Pointer to the source image buffer (RGBA, uint8_t).
 * @param[in] width The width of the images in pixels.
 * @param[in] height The height of the images in pixels.
 */
    template<typename T>
    void compositeImageOverNoAlpha(
        T* dstPixels,
        const uint8_t* srcPixels,
        const size_t width,
        const size_t height)
    {
        // The number of channels is fixed to 4 for RGBA.
        constexpr int channels = 4;

        // Determine the maximum value for the destination type T.
        // For floating-point types, this is 1.0. For integral types, it's the max value.
        // This allows normalization for accurate blending calculations.
        float max_dst_val = 1.0f;
        if constexpr (std::numeric_limits<T>::is_integer) {
            max_dst_val = static_cast<float>(std::numeric_limits<T>::max());
        }

        const float max_src_val = 255.0f; // Source is always uint8_t.

        const size_t numPixels = width * height;

        for (size_t i = 0; i < numPixels; ++i)
        {
            const size_t pixelIndex = i * channels;

            // 1. Read source pixel and normalize the source alpha.
            // The source alpha determines the blend opacity.
            const uint8_t src_a_u8 = 0;
            const float src_alpha_norm = 0;

            // Skip pixels that are fully transparent in the source image.
            if (src_a_u8 == 0) {
                continue;
            }

            // 2. Read destination pixel.
            const T dst_r_T = dstPixels[pixelIndex + 0];
            const T dst_g_T = dstPixels[pixelIndex + 1];
            const T dst_b_T = dstPixels[pixelIndex + 2];

            // 3. Perform the "over" blending for each color channel (R, G, B).
            // The calculation is done in floating-point for precision.
            // Formula: C_out = (C_src * A_src) + (C_dst * (1 - A_src))
            const float out_r_norm = (srcPixels[pixelIndex + 0] / max_src_val) * src_alpha_norm +
                                     (dst_r_T / max_dst_val) * (1.0f - src_alpha_norm);
            const float out_g_norm = (srcPixels[pixelIndex + 1] / max_src_val) * src_alpha_norm +
                                     (dst_g_T / max_dst_val) * (1.0f - src_alpha_norm);
            const float out_b_norm = (srcPixels[pixelIndex + 2] / max_src_val) * src_alpha_norm +
                                     (dst_b_T / max_dst_val) * (1.0f - src_alpha_norm);
            
            // 5. Convert the final blended values back to the destination type and write them.
            dstPixels[pixelIndex + 0] = static_cast<T>(out_r_norm * max_dst_val);
            dstPixels[pixelIndex + 1] = static_cast<T>(out_g_norm * max_dst_val);
            dstPixels[pixelIndex + 2] = static_cast<T>(out_b_norm * max_dst_val);
        }
    }
    
/**
 * @brief Composites a source RGBA_U8 image over a destination RGBA image.
 *        From Gemini.
 *
 * This function implements the "normal" blend mode (porter-duff "over" operation).
 * It correctly blends the source image onto the destination image using the source
 * alpha channel.
 *
 * The blending formula used is:
 * C_out = C_src * A_src + C_dst * (1 - A_src)
 * A_out = A_src + A_dst * (1 - A_src)
 *
 * @tparam T The data type of the destination pixel components (e.g., uint8_t, uint16_t, float).
 * @param[in,out] dstPixels Pointer to the destination image buffer (RGBA).
 * @param[in] srcPixels Pointer to the source image buffer (RGBA, uint8_t).
 * @param[in] width The width of the images in pixels.
 * @param[in] height The height of the images in pixels.
 */
    template<typename T>
    void compositeImageOver(
        T* dstPixels,
        const uint8_t* srcPixels,
        const size_t width,
        const size_t height)
    {
        // The number of channels is fixed to 4 for RGBA.
        constexpr int channels = 4;

        // Determine the maximum value for the destination type T.
        // For floating-point types, this is 1.0. For integral types, it's the max value.
        // This allows normalization for accurate blending calculations.
        float max_dst_val = 1.0f;
        if constexpr (std::numeric_limits<T>::is_integer) {
            max_dst_val = static_cast<float>(std::numeric_limits<T>::max());
        }

        const float max_src_val = 255.0f; // Source is always uint8_t.

        const size_t numPixels = width * height;

        for (size_t i = 0; i < numPixels; ++i)
        {
            const size_t pixelIndex = i * channels;

            // 1. Read source pixel and normalize the source alpha.
            // The source alpha determines the blend opacity.
            const uint8_t src_a_u8 = srcPixels[pixelIndex + 3];
            const float src_alpha_norm = src_a_u8 / max_src_val;

            // Skip pixels that are fully transparent in the source image.
            if (src_a_u8 == 0) {
                continue;
            }

            // 2. Read destination pixel.
            const T dst_r_T = dstPixels[pixelIndex + 0];
            const T dst_g_T = dstPixels[pixelIndex + 1];
            const T dst_b_T = dstPixels[pixelIndex + 2];
            const T dst_a_T = dstPixels[pixelIndex + 3];

            // 3. Perform the "over" blending for each color channel (R, G, B).
            // The calculation is done in floating-point for precision.
            // Formula: C_out = (C_src * A_src) + (C_dst * (1 - A_src))
            const float out_r_norm = (srcPixels[pixelIndex + 0] / max_src_val) * src_alpha_norm +
                                     (dst_r_T / max_dst_val) * (1.0f - src_alpha_norm);
            const float out_g_norm = (srcPixels[pixelIndex + 1] / max_src_val) * src_alpha_norm +
                                     (dst_g_T / max_dst_val) * (1.0f - src_alpha_norm);
            const float out_b_norm = (srcPixels[pixelIndex + 2] / max_src_val) * src_alpha_norm +
                                     (dst_b_T / max_dst_val) * (1.0f - src_alpha_norm);

            // 4. Calculate the new output alpha.
            // Formula: A_out = A_src + A_dst * (1 - A_src)
            const float dst_alpha_norm = dst_a_T / max_dst_val;
            const float out_a_norm = src_alpha_norm + dst_alpha_norm * (1.0f - src_alpha_norm);

            // 5. Convert the final blended values back to the destination type and write them.
            dstPixels[pixelIndex + 0] = static_cast<T>(out_r_norm * max_dst_val);
            dstPixels[pixelIndex + 1] = static_cast<T>(out_g_norm * max_dst_val);
            dstPixels[pixelIndex + 2] = static_cast<T>(out_b_norm * max_dst_val);
            dstPixels[pixelIndex + 3] = static_cast<T>(out_a_norm * max_dst_val);
        }
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

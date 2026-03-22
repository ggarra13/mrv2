// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Image.h>

#include <Imath/half.h>

    
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring> // For memcpy
#include <cstddef>
#include <limits>
#include <memory>

namespace mrv
{
    using namespace tl;
    
    namespace
    {
        // HLG (BT.2100) -> Linear light (normalized to 10,000 nits = 1.0)
        inline float hlg_to_linear(float hlg, const float L_W = 100.F)
        {
            // Clamp to [0, 1] to avoid NANs and invalid ranges
            hlg = std::clamp(hlg, 0.0f, 1.0f);

            // HLG EOTF Constants (BT.2100-2)
            // L_W is the System White level (100 nits).
            // The final result is normalized to L_PEAK = 10000 nits.
            const double a = 0.17883277;
            const double b = 1.0 - 4.0 * a; // ~0.28466892
            const double c = 0.5 - 2.0 * a; // ~0.14233446
    
            // Scale factor to convert the output (normalized to L_W) to L_PEAK (10000 nits)
            // L_W / L_PEAK = 100 / 10000 = 0.01
            const double scale_factor = L_W / 10000.F; 

            double L; // Linear light value, normalized to 10000 nits = 1.0
    
            // Low-luminance segment (0 <= V <= 0.5)
            if (hlg <= 0.5)
            {
                // E_nits = L_W * 3 * V^2
                // L = (L_W / 10000) * 3 * V^2 = 0.03 * V^2
                L = 3.0 * std::pow(hlg, 2.0) * scale_factor;
            }
            // High-luminance segment (0.5 < V <= 1.0)
            else
            {
                // E_nits = (L_W / 12) * (exp((V - c) / a) + b)
                // L = (L_W / 10000) * (E_nits / L_W) * L_W / 12 * (exp... + b)
                // L = (0.01 / 12.0) * (exp((V - c) / a) + b)
                const double exp_term = std::exp((hlg - c) / a);
                L = (exp_term + b) * scale_factor / 12.0;
            }

            return static_cast<float>(L);
        }
        
        // PQ (ST2084) → Linear light (normalized to 10,000 nits = 1.0)
        inline float pq_to_linear(float pq)
        {
            // Clamp to [0, 1] to avoid NANs
            pq = std::clamp(pq, 0.0f, 1.0f);
        
            // Note: Constants are defined in terms of the formula:
            // L = ((max(0, V^(1/m2) - c1)) / (c2 - c3 * V^(1/m2)))^(1/m1)
            const double m1 = 0.1593017578125;
            const double m2 = 78.84375;
            const double c1 = 0.8359375;
            const double c2 = 18.8515625;
            const double c3 = 18.6875;

            // Compute the EOTF
            const double vp = std::pow(pq, 1.0 / m2);
            const double numerator = std::max(vp - c1, 0.0);
            const double denominator = c2 - c3 * vp;
            const double L = std::pow(numerator / denominator, 1.0 / m1);
        
            // L is already the relative linear light value where
            // 1.0 == 10000 nits.
            return static_cast<float>(L);
        }
        
        // Saturating cast: integers clamp to their min/max, floats just cast.
        // (Works for uint8_t/uint16_t/int16_t/etc. and float/half-like types.)
        template <class T, class Acc>
        inline T sat_cast(Acc v)
        {
            if constexpr (std::numeric_limits<T>::is_integer)
            {
                const Acc lo = static_cast<Acc>(std::numeric_limits<T>::min());
                const Acc hi = static_cast<Acc>(std::numeric_limits<T>::max());
                v = std::clamp(v, lo, hi);
                return static_cast<T>(std::lrint(v)); // round-to-nearest
            } else {
                // Floating-point (float/double/half): no clamp by default
                return static_cast<T>(v);
            }
        }
        
        template <typename T>
        inline T lerp(const T& a, const T& b, float t)
        {
            return a + (b - a) * t;
        }
    }

    // Generic pixel scaling (assumes operator+,-,* are defined for T)
    template <typename T>
    void scaleImageLinear(
        const T* src, std::size_t width, std::size_t height,
        T* dst, std::size_t W, std::size_t H, std::size_t channels,
        bool align_corners = false)
    {
        assert(src && dst);
        assert(width > 0 && height > 0 && W > 0 && H > 0 && channels > 0);

        using Acc = double; // accumulator precision
    
        const Acc xScale = (align_corners && W > 1) ?
                           Acc(width - 1) / Acc(W - 1) :
                           Acc(width)     / Acc(W);
        const Acc yScale = (align_corners && H > 1) ?
                           Acc(height - 1) / Acc(H - 1) :
                           Acc(height)     / Acc(H);

        for (std::size_t j = 0; j < H; ++j)
        {
            const Acc sy = align_corners ?
                           (j * yScale) :
                           ((j + Acc(0.5)) * yScale - Acc(0.5));
            const Acc syc = std::clamp(sy, Acc(0), Acc(height - 1));
            const std::size_t y0 = static_cast<std::size_t>(std::floor(syc));
            const std::size_t y1 = std::min(y0 + 1, height - 1);
            const Acc fy = syc - y0;

            for (std::size_t i = 0; i < W; ++i)
            {
                const Acc sx = align_corners ?
                               (i * xScale) :
                               ((i + Acc(0.5)) * xScale - Acc(0.5));
                const Acc sxc = std::clamp(sx, Acc(0), Acc(width - 1));
                const std::size_t x0 = static_cast<std::size_t>(
                    std::floor(sxc));
                const std::size_t x1 = std::min(x0 + 1, width - 1);
                const Acc fx = sxc - x0;

                const std::size_t dstBase = (j * W + i) * channels;

                for (std::size_t c = 0; c < channels; ++c) {
                    const T p00 = src[(y0 * width + x0) * channels + c];
                    const T p10 = src[(y0 * width + x1) * channels + c];
                    const T p01 = src[(y1 * width + x0) * channels + c];
                    const T p11 = src[(y1 * width + x1) * channels + c];
                    
                    const Acc a00 = static_cast<Acc>(p00);
                    const Acc a10 = static_cast<Acc>(p10);
                    const Acc a01 = static_cast<Acc>(p01);
                    const Acc a11 = static_cast<Acc>(p11);

                    const Acc top    = lerp(a00, a10, fx);
                    const Acc bottom = lerp(a01, a11, fx);
                    const Acc value  = lerp(top, bottom, fy);

                    dst[dstBase + c] = sat_cast<T, Acc>(value);
                }
            }
        }
    }
    
    
    template<typename T>
    inline void flipImageInY(
        T* pixels, const size_t width, const size_t height,
        const int depth)
    {
        const size_t rowSize = width * depth;
        const size_t rowByteCount = rowSize * sizeof(T);
        T* tempRow = new T[width * depth];
        
        for (size_t y = 0; y < height / 2; ++y)
        {
            T* topRow = pixels + y * rowSize;
            T* bottomRow = pixels + (height - y - 1) * rowSize;
            // Swap entire rows using memcpy
            std::memcpy(tempRow, topRow, rowByteCount);
            std::memcpy(topRow, bottomRow, rowByteCount);
            std::memcpy(bottomRow, tempRow, rowByteCount);
        }

        delete[] tempRow; // Free the temporary buffer
    }

/**
 * @brief Composites a source image onto a destination using a custom OpenGL-style blend.
 *
 * This function implements the blending logic equivalent to the OpenGL call:
 * glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA,
 * GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 *
 * The blending formulas are:
 * RGB: C_out = C_src * 1 + C_dst * (1 - A_src)
 * A:   A_out = A_src * A_src + A_dst * (1 - A_src)
 *
 * @tparam T The data type of the destination pixel components (e.g., uint8_t, float).
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
    constexpr int channels = 4;

    // Determine the maximum value for the destination type T for normalization.
    float max_dst_val = 1.0f;
    if constexpr (std::numeric_limits<T>::is_integer) {
        max_dst_val = static_cast<float>(std::numeric_limits<T>::max());
    }

    const float max_src_val = 255.0f;
    const size_t numPixels = width * height;

    for (size_t i = 0; i < numPixels; ++i)
    {
        const size_t pixelIndex = i * channels;

        // 1. Read source pixel and normalize the source alpha.
        const uint8_t src_a_u8 = srcPixels[pixelIndex + 3];
        const float src_alpha_norm = src_a_u8 / max_src_val;

        if (src_a_u8 == 0) {
            continue; // Skip fully transparent pixels.
        }

        // 2. Normalize source and destination values for calculation.
        const float src_r_norm = srcPixels[pixelIndex + 0] / max_src_val;
        const float src_g_norm = srcPixels[pixelIndex + 1] / max_src_val;
        const float src_b_norm = srcPixels[pixelIndex + 2] / max_src_val;

        const float dst_r_norm = dstPixels[pixelIndex + 0] / max_dst_val;
        const float dst_g_norm = dstPixels[pixelIndex + 1] / max_dst_val;
        const float dst_b_norm = dstPixels[pixelIndex + 2] / max_dst_val;
        
        // 3. Apply the custom blending formulas.
        // RGB: C_out = C_src * 1 + C_dst * (1 - A_src)
        const float out_r_norm = src_r_norm + dst_r_norm * (1.0f - src_alpha_norm);
        const float out_g_norm = src_g_norm + dst_g_norm * (1.0f - src_alpha_norm);
        const float out_b_norm = src_b_norm + dst_b_norm * (1.0f - src_alpha_norm);

        // 4. Convert the final blended values back to the destination type and write them.
        // We must clamp the RGB values as this blend mode can easily result in values > 1.0.
        dstPixels[pixelIndex + 0] = static_cast<T>(std::min(out_r_norm, 1.0f) * max_dst_val);
        dstPixels[pixelIndex + 1] = static_cast<T>(std::min(out_g_norm, 1.0f) * max_dst_val);
        dstPixels[pixelIndex + 2] = static_cast<T>(std::min(out_b_norm, 1.0f) * max_dst_val);
    }
}
    /**
 * @brief Composites a source image onto a destination using a custom OpenGL-style blend.
 *
 * This function implements the blending logic equivalent to the OpenGL call:
 * glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA,
 * GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 *
 * The blending formulas are:
 * RGB: C_out = C_src * 1 + C_dst * (1 - A_src)
 * A:   A_out = A_src * A_src + A_dst * (1 - A_src)
 *
 * @tparam T The data type of the destination pixel components (e.g., uint8_t, float).
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
    constexpr int channels = 4;

    // Determine the maximum value for the destination type T for normalization.
    float max_dst_val = 1.0f;
    if constexpr (std::numeric_limits<T>::is_integer) {
        max_dst_val = static_cast<float>(std::numeric_limits<T>::max());
    }

    const float max_src_val = 255.0f;
    const size_t numPixels = width * height;

    for (size_t i = 0; i < numPixels; ++i)
    {
        const size_t pixelIndex = i * channels;

        // 1. Read source pixel and normalize the source alpha.
        const uint8_t src_a_u8 = srcPixels[pixelIndex + 3];
        const float src_alpha_norm = src_a_u8 / max_src_val;

        if (src_a_u8 == 0) {
            continue; // Skip fully transparent pixels.
        }

        // 2. Normalize source and destination values for calculation.
        const float src_r_norm = srcPixels[pixelIndex + 0] / max_src_val;
        const float src_g_norm = srcPixels[pixelIndex + 1] / max_src_val;
        const float src_b_norm = srcPixels[pixelIndex + 2] / max_src_val;

        const float dst_r_norm = dstPixels[pixelIndex + 0] / max_dst_val;
        const float dst_g_norm = dstPixels[pixelIndex + 1] / max_dst_val;
        const float dst_b_norm = dstPixels[pixelIndex + 2] / max_dst_val;
        const float dst_a_norm = dstPixels[pixelIndex + 3] / max_dst_val;
        
        // 3. Apply the custom blending formulas.
        // RGB: C_out = C_src * 1 + C_dst * (1 - A_src)
        const float out_r_norm = src_r_norm + dst_r_norm * (1.0f - src_alpha_norm);
        const float out_g_norm = src_g_norm + dst_g_norm * (1.0f - src_alpha_norm);
        const float out_b_norm = src_b_norm + dst_b_norm * (1.0f - src_alpha_norm);

        // Alpha: A_out = A_src * A_src + A_dst * (1 - A_src)
        const float out_a_norm = std::pow(src_alpha_norm, 2.0f) + dst_a_norm * (1.0f - src_alpha_norm);

        // 4. Convert the final blended values back to the destination type and write them.
        // We must clamp the RGB values as this blend mode can easily result in values > 1.0.
        dstPixels[pixelIndex + 0] = static_cast<T>(std::min(out_r_norm, 1.0f) * max_dst_val);
        dstPixels[pixelIndex + 1] = static_cast<T>(std::min(out_g_norm, 1.0f) * max_dst_val);
        dstPixels[pixelIndex + 2] = static_cast<T>(std::min(out_b_norm, 1.0f) * max_dst_val);
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
    
    void flipImageInY(const std::shared_ptr<image::Image> image);
    
    void composite_RGBA_U8(std::shared_ptr<image::Image>& dest,
                           std::shared_ptr<image::Image>& source);

    void convert_RGBA_to_RGB_U8(std::shared_ptr<image::Image>& dest,
                                std::shared_ptr<image::Image>& source);
    
} // namespace mrv

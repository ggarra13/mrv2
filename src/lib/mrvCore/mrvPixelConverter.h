#pragma once

#include <cstdint>
#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

namespace mrv
{

/**
 * @brief Converts a single floating-point color channel to an unsigned integer channel.
 *
 * This function takes a floating-point value (typically in the range [0.0, 1.0]),
 * scales it to the full range of the target integer type, and clamps it to ensure
 * it doesn't overflow.
 *
 * @tparam OutputInt The target unsigned integer type (e.g., uint8_t, uint16_t).
 * @tparam InputFloat The source floating-point type (e.g., float, double).
 * @param value The floating-point channel value to convert.
 * @return The converted unsigned integer channel value.
 */
    template <typename OutputInt, typename InputFloat>
    OutputInt convert_channel(InputFloat value)
    {
        using Float = float; // promote all math to float
    
        // Get the maximum value for the target integer type.
        const Float max_out = static_cast<Float>(std::numeric_limits<OutputInt>::max());

        // Scale the float value to the integer range.
        Float scaled_value = value * max_out;

        // Clamp the value to the valid range [0, max_out].
        scaled_value = std::max(static_cast<Float>(0.0),
                                std::min(scaled_value, max_out));

        // Round to the nearest integer and cast.
        return static_cast<OutputInt>(std::round(scaled_value));
    }


// --- RGB Conversion Functions ---

/**
 * @brief Converts an array of RGB float pixels to an array of RGB unsigned integer pixels.
 *
 * @tparam OutputInt The target unsigned integer type for each channel (e.g., uint8_t).
 * @tparam InputFloat The source floating-point type for each channel (e.g., float).
 * @param output_pixels Pointer to the beginning of the destination RGB integer data.
 * @param input_pixels Pointer to the beginning of the source RGB float data.
 * @param pixel_count The total number of pixels to convert.
 */
    template <typename OutputInt, typename InputFloat>
    void convert_rgb_array(OutputInt* output_pixels,
                           const InputFloat* input_pixels,
                           size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 3; i += 3)
        {
            output_pixels[i]     = convert_channel<OutputInt>(input_pixels[i]);
            output_pixels[i + 1] = convert_channel<OutputInt>(input_pixels[i + 1]);
            output_pixels[i + 2] = convert_channel<OutputInt>(input_pixels[i + 2]);
        }
    }
    
/**
 * @brief Converts an array of RGB float pixels to an array of RGBA unsigned integer pixels.
 *
 * @tparam OutputInt The target unsigned integer type for each channel.
 * @tparam InputFloat The source floating-point type for each channel.
 * @param output_pixels Pointer to the beginning of the destination RGBA integer data.
 * @param input_pixels Pointer to the beginning of the source RGBA float data.
 * @param pixel_count The total number of pixels to convert.
 */
    template <typename OutputInt, typename InputFloat>
    void convert_rgb_to_rgba_array(OutputInt* output_pixels,
                                   const InputFloat* input_pixels,
                                   const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 3; i += 3) {
            *output_pixels++ = convert_channel<OutputInt>(*input_pixels++);
            *output_pixels++ = convert_channel<OutputInt>(*input_pixels++);
            *output_pixels++ = convert_channel<OutputInt>(*input_pixels++);
            *output_pixels++ = convert_channel<OutputInt>(1.F);
        }
    }
    
    template <>
    void convert_rgb_to_rgba_array(half* output_pixels,
                                   const float* input_pixels,
                                   const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 3; i += 3) {
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = 1.F;
        }
    }
    
    template <>
    void convert_rgb_to_rgba_array(float* output_pixels,
                                   const half* input_pixels,
                                   const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 3; i += 3) {
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = 1.F;
        }
    }

// --- RGBA Conversion Functions ---

/**
 * @brief Converts an array of RGBA float pixels to an array of RGBA unsigned integer pixels.
 *
 * @tparam OutputInt The target unsigned integer type for each channel.
 * @tparam InputFloat The source floating-point type for each channel.
 * @param output_pixels Pointer to the beginning of the destination RGBA integer data.
 * @param input_pixels Pointer to the beginning of the source RGBA float data.
 * @param pixel_count The total number of pixels to convert.
 */
    template <typename OutputInt, typename InputFloat>
    void convert_rgba_array(OutputInt* output_pixels,
                            const InputFloat* input_pixels,
                            const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 4; i += 4)
        {
            output_pixels[i]     = convert_channel<OutputInt>(input_pixels[i]);
            output_pixels[i + 1] = convert_channel<OutputInt>(input_pixels[i + 1]);
            output_pixels[i + 2] = convert_channel<OutputInt>(input_pixels[i + 2]);
            output_pixels[i + 3] = convert_channel<OutputInt>(input_pixels[i + 3]);
        }
    }
    
    
    
    template <>
    inline void convert_rgba_array<float, half>(float* output_pixels,
                                                const half* input_pixels,
                                                const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 4; ++i)
        {
            output_pixels[i] = input_pixels[i];
        }
    }
    
    template <>
    inline
    void convert_rgba_array<float, uint16_t>(float* output_pixels,
                                             const uint16_t* input_pixels,
                                             const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 4; ++i)
        {
            output_pixels[i] = input_pixels[i] / 65535.F;
        }
    }
    
    template <>
    inline
    void convert_rgba_array<float, uint8_t>(float* output_pixels,
                                            const uint8_t* input_pixels,
                                            const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 4; ++i)
        {
            output_pixels[i] = input_pixels[i] / 255.F;
        }
    }
    
    template <>
    inline void convert_rgba_array<half, float>(half* output_pixels,
                                                const float* input_pixels,
                                                const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 4; ++i)
        {
            output_pixels[i] = input_pixels[i];
        }
    }
    
    template <>
    inline void convert_rgba_array<uint16_t, uint8_t>(
        uint16_t* output_pixels,
        const uint8_t* input_pixels,
        const size_t pixel_count)
    {
        const float SCALE_FACTOR = 65535.F / 255.F;
        
        for (size_t i = 0; i < pixel_count * 4; ++i)
        {
            output_pixels[i] = (uint16_t)((float)(input_pixels[i] * SCALE_FACTOR));
        }
    }
    
    template <>
    inline void convert_rgba_array<uint8_t, uint16_t>(
        uint8_t* output_pixels,
        const uint16_t* input_pixels,
        const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 4; ++i)
        {
            output_pixels[i] = (uint8_t)(((float)input_pixels[i] / 65535.0f) *
                                         255.0f);
        }
    }
    
    template <>
    inline void convert_rgba_array<half, uint16_t>(half* output_pixels,
                                                   const uint16_t* input_pixels,
                                                   const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 4; ++i)
        {
            output_pixels[i] = input_pixels[i] / 65535.F;
        }
    }
    
    template <>
    inline void convert_rgba_array<half, uint8_t>(half* output_pixels,
                                                  const uint8_t* input_pixels,
                                                  const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count * 4; ++i)
        {
            output_pixels[i] = input_pixels[i] / 255.F;
        }
    }
    
/**
 * @brief Converts an array of RGBA float pixels to an array of RGB unsigned integer pixels.
 *
 * @tparam OutputInt The target unsigned integer type for each channel.
 * @tparam InputFloat The source floating-point type for each channel.
 * @param output_pixels Pointer to the beginning of the destination RGB integer data.
 * @param input_pixels Pointer to the beginning of the source RGBA float data.
 * @param pixel_count The total number of pixels to convert.
 */
    template <typename OutputInt, typename InputFloat>
    void convert_rgba_to_rgb_array(OutputInt* output_pixels,
                                   const InputFloat* input_pixels,
                                   const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count; ++i) {
            *output_pixels++ = convert_channel<OutputInt>(*input_pixels++);
            *output_pixels++ = convert_channel<OutputInt>(*input_pixels++);
            *output_pixels++ = convert_channel<OutputInt>(*input_pixels++);
            input_pixels++;
        }
    }

    template<>
    inline void convert_rgba_to_rgb_array<float, uint8_t>(
        float* output_pixels,
        const uint8_t* input_pixels,
        const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count; ++i) {
            *output_pixels++ = *input_pixels++ / 255.F;
            *output_pixels++ = *input_pixels++ / 255.F;
            *output_pixels++ = *input_pixels++ / 255.F;
            input_pixels++;
        }
    }
    
    template<>
    inline void convert_rgba_to_rgb_array<float, uint16_t>(
        float* output_pixels,
        const uint16_t* input_pixels,
        const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count; ++i) {
            *output_pixels++ = *input_pixels++ / 655535.F;
            *output_pixels++ = *input_pixels++ / 655535.F;
            *output_pixels++ = *input_pixels++ / 655535.F;
            input_pixels++;
        }
    }
    
    template<>
    inline void convert_rgba_to_rgb_array<half, uint8_t>(
        half* output_pixels,
        const uint8_t* input_pixels,
        const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count; ++i) {
            *output_pixels++ = *input_pixels++ / 255.F;
            *output_pixels++ = *input_pixels++ / 255.F;
            *output_pixels++ = *input_pixels++ / 255.F;
            input_pixels++;
        }
    }
    
    template<>
    inline void convert_rgba_to_rgb_array<half, uint16_t>(
        half* output_pixels,
        const uint16_t* input_pixels,
        const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count; ++i) {
            *output_pixels++ = *input_pixels++ / 655535.F;
            *output_pixels++ = *input_pixels++ / 655535.F;
            *output_pixels++ = *input_pixels++ / 655535.F;
            input_pixels++;
        }
    }
    
    template<>
    inline void convert_rgba_to_rgb_array(float* output_pixels,
                                          const half* input_pixels,
                                          const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count; ++i) {
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            input_pixels++;
        }
    }
    
    template<>
    inline void convert_rgba_to_rgb_array(half* output_pixels,
                                          const float* input_pixels,
                                          const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count; ++i) {
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            input_pixels++;
        }
    }
    
    template<>
    inline void convert_rgba_to_rgb_array(uint8_t* output_pixels,
                                          const uint16_t* input_pixels,
                                          const size_t pixel_count)
    {
        // Constant for floating-point scaling (255.0 / 65535.0)
        // We use a float literal 65535.0f to ensure float division.
        const float SCALE_FACTOR = 255.0f / 65535.0f;
    
        for (size_t i = 0; i < pixel_count; ++i) {
            *output_pixels++ = (uint8_t)((float)*input_pixels++ * SCALE_FACTOR);
            *output_pixels++ = (uint8_t)((float)*input_pixels++ * SCALE_FACTOR);
            *output_pixels++ = (uint8_t)((float)*input_pixels++ * SCALE_FACTOR);
            input_pixels++;
        }
    }
    
    template<>
    inline void convert_rgba_to_rgb_array(uint16_t* output_pixels,
                                          const uint8_t* input_pixels,
                                          const size_t pixel_count)
    {
        // Constant for floating-point scaling
        const float SCALE_FACTOR = 65535.0f / 255.0f;
    
        for (size_t i = 0; i < pixel_count; ++i) {
            *output_pixels++ = (uint16_t)((float)*input_pixels++ * SCALE_FACTOR);
            *output_pixels++ = (uint16_t)((float)*input_pixels++ * SCALE_FACTOR);
            *output_pixels++ = (uint16_t)((float)*input_pixels++ * SCALE_FACTOR);
            input_pixels++;
        }
    }
    
    template<>
    inline void convert_rgba_to_rgb_array(uint16_t* output_pixels,
                                          const uint16_t* input_pixels,
                                          const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count; ++i) {
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            input_pixels++;
        }
    }
    
    template<>
    inline void convert_rgba_to_rgb_array(uint8_t* output_pixels,
                                          const uint8_t* input_pixels,
                                          const size_t pixel_count)
    {
        for (size_t i = 0; i < pixel_count; ++i) {
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            *output_pixels++ = *input_pixels++;
            input_pixels++;
        }
    }
}

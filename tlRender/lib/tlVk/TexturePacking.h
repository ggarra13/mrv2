#include <cstdint>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstring>

#include <half.h>

namespace tl
{
    namespace vlk
    {

        /**
         * @brief Converts a standard float to an unsigned 11-bit (or 10-bit) float component.
         * * This function handles clamping, deconstructing the float, and packing 
         * it into the desired bit count for the B10G11R11 format.
         * * @param v The input float value.
         * @param mantissaBits The mantissa bit count (6 for 11-bit, 5 for 10-bit).
         * @param exponentBias The exponent bias (15 for 11-bit, 14 for 10-bit).
         * @return The unsigned integer component value.
         */
        inline
        uint32_t floatToE5M(float v, int mantissaBits, int exponentBias) {
            // B10G11R11 is unsigned, so clamp negatives to zero.
            v = std::max(0.0f, v);

            // Get the raw 32-bit representation of the float
            uint32_t f_bits;
            std::memcpy(&f_bits, &v, sizeof(float));

            // Deconstruct the standard 32-bit float:
            // Sign bit is ignored (since we clamped v >= 0)
            uint32_t f_exponent = (f_bits >> 23) & 0xFF; // 8-bit exponent (bias 127)
            uint32_t f_mantissa = f_bits & 0x7FFFFF;     // 23-bit mantissa

            // If the value is zero (exponent == 0)
            if (f_exponent == 0) {
                return 0;
            }

            // --- Special Value Check: Infinity/NaN (Handle as Clamped Max) ---
            // If the exponent is 255 (Max value in 8-bit float exponent), it's Inf/NaN
            if (f_exponent == 255) {
                // Return the max representable value (all exponent and mantissa bits set)
                return ((1 << (mantissaBits + 1)) - 1); 
            }

            // --- Regular Value Conversion ---
    
            // 1. Convert 8-bit float exponent (bias 127) to new format exponent (bias 15 or 14)
            int32_t exponent_delta = (int32_t)f_exponent - 127;
            uint32_t new_exponent = exponent_delta + exponentBias;

            // 2. Adjust Mantissa (downshift from 23 bits to 6 or 5 bits)
            // The amount to shift the mantissa is based on the difference in size.
            uint32_t mantissa_shift = 23 - mantissaBits;
            uint32_t new_mantissa = f_mantissa >> mantissa_shift;
    
            // --- Final Packing into Component ---
    
            // If the new exponent is <= 0, the value is too small (flush to zero)
            if (new_exponent <= 0) {
                return 0;
            }
    
            // If the new exponent is too large (clamped max)
            uint32_t max_exponent = (1 << (11 - mantissaBits)) - 1; // 31 for 11-bit, 31 for 10-bit
            if (new_exponent >= max_exponent) {
                new_exponent = max_exponent;
                // All mantissa bits are set for the clamped max value
                new_mantissa = (1 << mantissaBits) - 1; 
            }

            // The component is (Exponent << Mantissa_Size) | Mantissa
            return (new_exponent << mantissaBits) | new_mantissa;
        }

        /**
         * @brief Main function to pack RGB_F32 data into B10G11R11_UFLOAT_PACK32 format.
         * * @param sourceDataRGB Vector of input RGB float values (3 floats per pixel).
         * @return Vector of packed 32-bit integers (1 uint32_t per pixel).
         */
        template<typename T>
        inline
        std::vector<uint32_t> packRGB_B10G11R11(const T* sourceDataRGB,
                                                const std::size_t byteSize) {
            if (byteSize % 3 != 0) {
                // Handle error: must have R, G, B for every pixel
                return {}; 
            }

            size_t numPixels = byteSize / 3;
            std::vector<uint32_t> packedBuffer;
            packedBuffer.reserve(numPixels);

            for (size_t i = 0; i < byteSize; i += 3) {
                // Read the R, G, B components
                float r = sourceDataRGB[i + 0];
                float g = sourceDataRGB[i + 1];
                float b = sourceDataRGB[i + 2];

                // --- 1. Convert Components ---
                // R and G use 11-bit (5-bit exponent, 6-bit mantissa, bias 15)
                // B uses 10-bit (5-bit exponent, 5-bit mantissa, bias 14)
        
                // Red Component (R11)
                uint32_t packedR = floatToE5M(r, 6, 15); // 11 bits: 5 exp (16-31), 6 mant (0-15)
                // Green Component (G11)
                uint32_t packedG = floatToE5M(g, 6, 15); // 11 bits: 5 exp (16-31), 6 mant (0-15)
                // Blue Component (B10)
                uint32_t packedB = floatToE5M(b, 5, 15); // 10 bits: 5 exp (16-31), 5 mant (0-15)

                // --- 2. Bit Shift and Combine ---
        
                // Layout: [31..22] B | [21..11] G | [10..0] R
                uint32_t finalPackedValue = 
                    (packedB << 22) | // Blue is in the most significant 10 bits
                    (packedG << 11) | // Green is in the middle 11 bits
                    (packedR << 0);   // Red is in the least significant 11 bits

                packedBuffer.push_back(finalPackedValue);
            }

            return packedBuffer;
        }

    }
}

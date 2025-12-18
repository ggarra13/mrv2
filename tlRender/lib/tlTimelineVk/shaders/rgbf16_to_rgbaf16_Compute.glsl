#version 450

layout (local_size_x = 16, local_size_y = 16) in;

// Binding 0: Raw Buffer treated as 32-bit unsigned ints
layout(std430, binding = 0) readonly buffer InputBuffer {
    uint data[];
} inputBuffer;

layout(binding = 1, rgba16f) uniform writeonly image2D outputImage;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outputImage);

    if (texelCoord.x < size.x && texelCoord.y < size.y) {
        // Each pixel has 3 components of 16 bits = 48 bits total per pixel.
        // Total bits before this pixel = (y * width + x) * 48
        uint totalBits = (texelCoord.y * size.x + texelCoord.x) * 48;
        
        // Find which 32-bit index the pixel starts at
        uint wordIdx = totalBits / 32;
        uint bitOffset = totalBits % 32;

        uint r_bits, g_bits, b_bits;

        // Extracting R, G, B from the bitstream
        if (bitOffset == 0) {
            // Pixel is aligned: [ R16 | G16 ] [ B16 | ... ]
            uint word0 = inputBuffer.data[wordIdx];
            uint word1 = inputBuffer.data[wordIdx + 1];
            r_bits = word0 & 0xFFFF;
            g_bits = word0 >> 16;
            b_bits = word1 & 0xFFFF;
        } else {
            // Pixel is split: [ ... | R16 ] [ G16 | B16 ]
            uint word0 = inputBuffer.data[wordIdx];
            uint word1 = inputBuffer.data[wordIdx + 1];
            r_bits = word0 >> 16;
            g_bits = word1 & 0xFFFF;
            b_bits = word1 >> 16;
        }

        // unpackHalf2x16 takes a uint32. We put our 16 bits in the low slot.
        float r = unpackHalf2x16(r_bits).x;
        float g = unpackHalf2x16(g_bits).x;
        float b = unpackHalf2x16(b_bits).x;

        imageStore(outputImage, texelCoord, vec4(r, g, b, 1.0));
    }
}


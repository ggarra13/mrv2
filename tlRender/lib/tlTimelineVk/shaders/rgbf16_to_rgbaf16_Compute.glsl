#version 450

layout (local_size_x = 16, local_size_y = 16) in;

layout(std430, binding = 0) readonly buffer InputBuffer {
    uint data[];
} inputBuffer;

layout(binding = 1, rgba16f) uniform writeonly image2D outputImage;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outputImage);

    if (texelCoord.x >= size.x || texelCoord.y >= size.y) return;

    // Linear index of the pixel
    uint pixelIdx = texelCoord.y * size.x + texelCoord.x;
    
    // Every 2 pixels use exactly 3 uints (96 bits).
    // Word index is (pixelIdx / 2) * 3. 
    // We use (pixelIdx >> 1) * 3 to avoid slow division.
    uint baseWordIdx = (pixelIdx >> 1) * 3;
    bool isOdd = bool(pixelIdx & 1);

    uint r_bits, g_bits, b_bits;

    // Load the 3 words that contain the 2-pixel block
    uint w0 = inputBuffer.data[baseWordIdx];
    uint w1 = inputBuffer.data[baseWordIdx + 1];
    uint w2 = inputBuffer.data[baseWordIdx + 2];

    // Branchless selection using bit masking and shifts
    if (!isOdd) {
        // Pixel 0: [R0 (16) | G0 (16)] [B0 (16) | ...]
        r_bits = w0 & 0xFFFF;
        g_bits = w0 >> 16;
        b_bits = w1 & 0xFFFF;
    } else {
        // Pixel 1: [... | R1 (16)] [G1 (16) | B1 (16)]
        r_bits = w1 >> 16;
        g_bits = w2 & 0xFFFF;
        b_bits = w2 >> 16;
    }

    // Unpack half-floats
    vec3 color = vec3(
        unpackHalf2x16(r_bits).x,
        unpackHalf2x16(g_bits).x,
        unpackHalf2x16(b_bits).x
    );

    imageStore(outputImage, texelCoord, vec4(color, 1.0));
}

#version 450

layout (local_size_x = 16, local_size_y = 16) in;

// Binding 0: Raw Buffer containing float32 data as uints
layout(std430, binding = 0) readonly buffer InputBuffer {
    uint data[];
} inputBuffer;

// Changed to rgba32f to match the 32-bit precision
layout(binding = 1, rgba32f) uniform writeonly image2D outputImage;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outputImage);

    if (texelCoord.x >= size.x || texelCoord.y >= size.y) return;
    
    // Each pixel has 3 components of 32 bits = 96 bits total per pixel.
    // This corresponds to exactly 3 indices in the uint array.
    uint pixelIdx = (texelCoord.y * size.x + texelCoord.x) * 3;

    // Grab the raw bits for R, G, and B
    uint r_uint = inputBuffer.data[pixelIdx];
    uint g_uint = inputBuffer.data[pixelIdx + 1];
    uint b_uint = inputBuffer.data[pixelIdx + 2];
    
    // Convert the bit patterns directly to floats
    // This is a standard GLSL function, no extension required.
    float r = uintBitsToFloat(r_uint);
    float g = uintBitsToFloat(g_uint);
    float b = uintBitsToFloat(b_uint);

    // Write out to the RGBA image (Alpha set to 1.0)
    imageStore(outputImage, texelCoord, vec4(r, g, b, 1.0));
}

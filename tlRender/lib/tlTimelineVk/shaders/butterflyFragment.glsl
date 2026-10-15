#version 450
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D textureSampler;
layout(binding = 2) uniform sampler2D textureSamplerB;

void main()
{
    // The same half of both, the second one mirrored, so that
    // the middle of the picture is on both sides of the seam.
    if (fTexture.x < .5)
    {
        outColor = texture(textureSampler, fTexture);
    }
    else
    {
        outColor = texture(textureSamplerB,
                       vec2(1.0 - fTexture.x, fTexture.y));
    }
}

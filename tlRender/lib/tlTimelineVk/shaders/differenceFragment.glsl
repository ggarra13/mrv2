#version 450
                 
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 outColor;
                 
layout(binding = 1) uniform sampler2D textureSampler;
layout(binding = 2) uniform sampler2D textureSamplerB;
                 
void main()
{
    vec4 c = texture(textureSampler, fTexture);
    vec4 cB = texture(textureSamplerB, fTexture);
    outColor.r = abs(c.r - cB.r);
    outColor.g = abs(c.g - cB.g);
    outColor.b = abs(c.b - cB.b);
    outColor.a = max(c.a, cB.a);
}

#version 450
                 
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D textureSampler;

layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;
                 
void main()
{
     outColor = texture(textureSampler, fTexture) * pc.color;
}

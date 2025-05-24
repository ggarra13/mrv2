#version 450

layout(location = 0) in vec2 fTexture;

layout(binding = 1) uniform sampler2D textureSampler;

layout(push_constant) uniform PushConstants {
    float opacity;
} pc;

layout(location = 0) out vec4 fColor;


void main()
{
    fColor = texture(textureSampler, fTexture);
    fColor *= pc.opacity;
}

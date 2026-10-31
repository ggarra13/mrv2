#version 450

layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 fColor;

layout(binding = 0) uniform sampler2D textureSampler;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    vec4 color;
} pc;


void main()
{
    fColor = texture(textureSampler, fTexture);
    fColor *= pc.color.a;
}

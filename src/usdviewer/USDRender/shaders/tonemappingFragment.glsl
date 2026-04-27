#version 450
                 
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D textureSampler;

layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;
                 
void main()
{
    vec3 hdrColor = texture(textureSampler, fTexture).rgb * pc.color.rgb;
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    mapped = pow(mapped, vec3(1.0 / 2.2));
    outColor = vec4(mapped, pc.color.a);
}

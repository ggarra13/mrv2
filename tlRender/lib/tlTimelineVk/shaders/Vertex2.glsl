#version 450
layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vTexture;
layout(location = 0) out vec2 fTexture;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    vec4 color;
} pc;

void main()
{
    gl_Position = pc.mvp * vec4(vPos, 0.0, 1.0);
    fTexture = vTexture;
}

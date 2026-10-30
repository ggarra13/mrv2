#version 450
layout(location = 0) in vec2 vPos;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    vec4 color;
} pc;

void main()
{
    gl_Position = pc.mvp * vec4(vPos, 0.0, 1.0);
}

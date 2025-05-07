#version 450
layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vTexture;
layout(location = 0) out vec2 fTexture;
layout(set = 0, binding = 0, std140) uniform Transform {
    mat4 mvp;
} transform;

void main()
{
    gl_Position = transform.mvp * vec4(vPos, 0.0, 1.0);
    fTexture = fTexture;
}

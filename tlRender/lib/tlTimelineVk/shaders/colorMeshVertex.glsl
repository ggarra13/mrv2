#version 450
                 
layout(location = 0) in vec2 vPos;
layout(location = 1) in vec4 vColor;
layout(location = 0) out vec4 fColor;

layout(set = 0, binding = 0, std140) uniform TransformUBO {
    mat4 mvp;
} transform;
                 
void main()
{
    gl_Position = transform.mvp * vec4(vPos, 0.0, 1.0);
    fColor = vColor;
}

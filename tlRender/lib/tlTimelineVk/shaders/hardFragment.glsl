#version 450

layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 fColor;
              
layout(set = 0, binding = 1) uniform OpacityUBO {
     vec4 color;
} ubo;
              
void main()
{
     fColor = ubo.color;
}

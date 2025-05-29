#version 450
              
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 fColor;
           
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;

void main()
{
     vec2       v = fTexture - vec2(0.5, 0.5);
     float ratio  = 1- sqrt(v.x * v.x + v.y * v.y);
     float radius = 0.75;
     float feather = 0.25;
     float mult = smoothstep(radius - feather, radius + feather, ratio);
     fColor = pc.color;
     fColor.a *= mult;
}

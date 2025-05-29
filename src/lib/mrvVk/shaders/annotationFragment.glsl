#version 450
                  
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 fColor;

layout(binding = 1) uniform sampler2D textureSampler;
                  
// enum tl::render::Channels
const uint Channels_Color = 0;
const uint Channels_Red   = 1;
const uint Channels_Green = 2;
const uint Channels_Blue  = 3;
const uint Channels_Alpha = 4;
 
layout(set = 0, binding = 2) uniform ChannelsUBO {
     int value;
} channels;
 
vec4 swizzleFunc(vec4 c, int channels)
{
    // Swizzle for the channels display.
    if (Channels_Red == channels)
    {
        c.g = c.r;
        c.b = c.r;
    }
    else if (Channels_Green == channels)
    {
        c.r = c.g;
        c.b = c.g;
    }
    else if (Channels_Blue == channels)
    {
        c.r = c.b;
        c.g = c.b;
    }
    else if (Channels_Alpha == channels)
    {
        c.r = c.a;
        c.g = c.a;
        c.b = c.a;
     }
     return c;
 }
                  
void main()
{
     fColor = texture(textureSampler, fTexture);
     fColor = swizzleFunc(fColor, channels.value);
}

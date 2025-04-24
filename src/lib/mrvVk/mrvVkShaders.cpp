// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/StringFormat.h>

#include "mrvVk/mrvVkShaders.h"

namespace mrv
{
    namespace
    {
        const std::string swizzleSource = R"(
// enum tl::render::Channels
const uint Channels_Color = 0;
const uint Channels_Red   = 1;
const uint Channels_Green = 2;
const uint Channels_Blue  = 3;
const uint Channels_Alpha = 4;
 
layout(set = 0, binding = 0) uniform ChannelsUBO {
     int channels;
} channelsUBO;
 
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
 })";

const std::string stereoSource = R"(
// enum mrv::StereoOutput
const uint Stereo_Anaglyph     = 0;
const uint Stereo_Checkerboard = 1;
const uint Stereo_Scanlines    = 2;
const uint Stereo_Columns      = 3;
const uint Stereo_OpenGL       = 4;
 
layout(set = 0, binding = 0) uniform int stereo;
layout(set = 0, binding = 1) uniform int width;
layout(set = 0, binding = 2) uniform int height;
 
vec4 stereoFunc(vec4 c, vec2 st)
{
    int x = 0;
    // Swizzle for the channels display.
    if (Stereo_Scanlines == stereo)
    {
         float f = st.y * height;
         x = int( mod( f, 2 ) );
    }
    else if (Stereo_Columns == stereo)
    {
         float f2 = st.x * width;
         x = int( mod( f2, 2 ) );
    }
    else if (Stereo_Checkerboard == stereo)
    {
         float f = st.y * height;
         float f2 = st.x * width;
         x = int( mod( floor( f2 ) + floor( f ), 2 ) < 1 );
    }
    if (x == 1)
    {
         c.r = c.g = c.b = c.a = 0.0;
    }
    return c;
})";

} // namespace


std::string textureFragmentSource()
{
return R"(#version 450

layout(location = 0) in vec2 fTexture;

layout(set = 0, binding = 0) uniform OpacityUBO {
     float opacity;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D textureSampler;

layout(location = 0) out vec4 fColor;


void main()
{
    fColor = texture(textureSampler, fTexture);
    fColor *= ubo.opacity;
})";
}

std::string stereoFragmentSource()
{
    return tl::string::Format(R"(
#version 450
                  
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 fColor;

layout(set = 0, binding = 0) uniform float opacity;
                  
{0}
                  
layout(binding = 1) uniform sampler2D textureSampler;
                  
void main()
{
    fColor = texture(textureSampler, fTexture);
    fColor = stereoFunc(fColor, fTexture);
    fColor.a *= opacity;
})")
            .arg(stereoSource);
    }

    std::string annotationFragmentSource()
    {
        return tl::string::Format(R"(
#version 450
                  
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 fColor;
                  
{0}
                  
layout(binding = 0) uniform sampler2D textureSampler;
                  
void main()
{
     fColor = texture(textureSampler, fTexture);
     fColor = swizzleFunc(fColor, channelsUBO.channels);
})").arg(swizzleSource);

    }

    std::string softFragmentSource()
    {
        return R"(
#version 450
              
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 fColor;
              
layout(set = 0, binding = 0) uniform vec4  color;

void main()
{
     vec2       v = fTexture - vec2(0.5, 0.5);
     float ratio  = 1- sqrt(v.x * v.x + v.y * v.y);
     float radius = 0.75;
     float feather = 0.25;
     float mult = smoothstep(radius - feather, radius + feather, ratio);
     fColor = color;
     fColor.a *= mult;
})";
    }

    std::string hardFragmentSource()
    {
        return R"(
#version 450

layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 fColor;
              
uniform vec4  color;
              
void main()
{
     fColor = color;
})";
    }

} // namespace mrv

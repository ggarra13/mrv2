#version 450
                  
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 fColor;

layout(binding = 1) uniform sampler2D textureSampler;

layout(push_constant) uniform PushConstants {
    float opacity;
} pc;
                  // enum mrv::StereoOutput
const uint Stereo_Anaglyph     = 0;
const uint Stereo_Checkerboard = 1;
const uint Stereo_Scanlines    = 2;
const uint Stereo_Columns      = 3;
const uint Stereo_OpenGL       = 4;

layout(set = 0, binding = 3) uniform OpacityUBO {
     uniform int stereo;
     uniform int width;
     uniform int height;
} ubo; 
 
vec4 stereoFunc(vec4 c, vec2 st)
{
    int x = 0;
    // Swizzle for the channels display.
    if (Stereo_Scanlines == ubo.stereo)
    {
         float f = st.y * ubo.height;
         x = int( mod( f, 2 ) );
    }
    else if (Stereo_Columns == ubo.stereo)
    {
         float f2 = st.x * ubo.width;
         x = int( mod( f2, 2 ) );
    }
    else if (Stereo_Checkerboard == ubo.stereo)
    {
         float f = st.y * ubo.height;
         float f2 = st.x * ubo.width;
         x = int( mod( floor( f2 ) + floor( f ), 2 ) < 1 );
    }
    if (x == 1)
    {
         c.r = c.g = c.b = c.a = 0.0;
    }
    return c;
}

void main()
{
    fColor = texture(textureSampler, fTexture);
    fColor = stereoFunc(fColor, fTexture);
    fColor.a *= pc.opacity;
}

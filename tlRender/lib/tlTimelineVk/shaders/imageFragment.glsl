#version 450
                                
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 outColor;
                                
// enum tl::image::PixelType
const uint PixelType_None              = 0;
const uint PixelType_L_U8              = 1;
const uint PixelType_L_U16             = 2;
const uint PixelType_L_U32             = 3;
const uint PixelType_L_F16             = 4;
const uint PixelType_L_F32             = 5;
const uint PixelType_LA_U8             = 6;
const uint PixelType_LA_U32            = 7;
const uint PixelType_LA_U16            = 8;
const uint PixelType_LA_F16            = 9;
const uint PixelType_LA_F32            = 10;
const uint PixelType_RGB_U8            = 11;
const uint PixelType_RGB_U10           = 12;
const uint PixelType_RGB_U16           = 13;
const uint PixelType_RGB_U32           = 14;
const uint PixelType_RGB_F16           = 15;
const uint PixelType_RGB_F32           = 16;
const uint PixelType_RGBA_U8           = 17;
const uint PixelType_RGBA_U16          = 18;
const uint PixelType_RGBA_U32          = 19;
const uint PixelType_RGBA_F16          = 20;
const uint PixelType_RGBA_F32          = 21;
const uint PixelType_YUV_420P_U8       = 22;
const uint PixelType_YUV_422P_U8       = 23;
const uint PixelType_YUV_444P_U8       = 24;
const uint PixelType_YUV_420P_U16      = 25;
const uint PixelType_YUV_422P_U16      = 26;
const uint PixelType_YUV_444P_U16      = 27;
const uint PixelType_ARGB_4444_Premult = 28;
                                
// enum tl::image::VideoLevels
const uint VideoLevels_FullRange  = 0;
const uint VideoLevels_LegalRange = 1;
                                
vec4 sampleTexture(
              vec2 textureCoord,
              int pixelType,
              int videoLevels,
              vec4 yuvCoefficients,
              int imageChannels,
              sampler2D s0,
              sampler2D s1,
              sampler2D s2)
{
       vec4 c;
       if (PixelType_YUV_420P_U8 == pixelType ||
           PixelType_YUV_422P_U8 == pixelType ||
           PixelType_YUV_444P_U8 == pixelType ||
           PixelType_YUV_420P_U16 == pixelType ||
           PixelType_YUV_422P_U16 == pixelType ||
           PixelType_YUV_444P_U16 == pixelType)
       {
           if (VideoLevels_FullRange == videoLevels)
           {
                float y  = texture(s0, textureCoord).r;
                float cb = texture(s1, textureCoord).r - 0.5;
                float cr = texture(s2, textureCoord).r - 0.5;
                c.r = y + (yuvCoefficients.x * cr);
                c.g = y - (yuvCoefficients.y * cr) - (yuvCoefficients.z * cb);
                c.b = y + (yuvCoefficients.w * cb);
            }
            else if (VideoLevels_LegalRange == videoLevels)
            {
                 float y  = (texture(s0, textureCoord).r - (16.0 / 255.0)) * (255.0 / (235.0 - 16.0));
                 float cb = (texture(s1, textureCoord).r - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0)) - 0.5;
                 float cr = (texture(s2, textureCoord).r - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0)) - 0.5;
                 c.r = y + (yuvCoefficients.x * cr);
                 c.g = y - (yuvCoefficients.y * cr) - (yuvCoefficients.z * cb);
                 c.b = y + (yuvCoefficients.w * cb);
             }
             c.a = 1.0;
        }
        else
        {
             c = texture(s0, textureCoord);

             // Video levels.
             if (VideoLevels_LegalRange == videoLevels)
             {
                  c.r = (c.r - (16.0 / 255.0)) * (255.0 / (235.0 - 16.0));
                          c.g = (c.g - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0));
                          c.b = (c.b - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0));
             }
              
             // This was needed in OpenGL, but not for Vulkan
             // Swizzle for the image channels.
             // if (1 == imageChannels)
             // {
             //     c.g = c.b = c.r;
             //     c.a = 1.0;
             // }
             // else if (2 == imageChannels)
             // {
             //     c.a = c.g;
             //     c.g = c.b = c.r;
             // }
             // else if (3 == imageChannels)
             // {
             //     c.a = 1.0;
             // }
         }
         return c;
}

layout(set = 0, binding = 1, std140) uniform UBO {
                                vec4      yuvCoefficients;
                                vec4      color;
                                int       pixelType;
                                int       videoLevels;
                                int       imageChannels;
                                int       mirrorX;
                                int       mirrorY;
} ubo;

layout(binding = 2) uniform sampler2D textureSampler0;
layout(binding = 3) uniform sampler2D textureSampler1;
layout(binding = 4) uniform sampler2D textureSampler2;
                                
void main()
{
    vec2 t = fTexture;
    if (1 == ubo.mirrorX)
    {
        t.x = 1.0 - t.x;
    }
    if (0 == ubo.mirrorY)
    {
        t.y = 1.0 - t.y;
    }
    outColor = sampleTexture(t,
                             ubo.pixelType,
                             ubo.videoLevels,
                             ubo.yuvCoefficients,
                             ubo.imageChannels,
                             textureSampler0,
                             textureSampler1,
                             textureSampler2) * ubo.color;
}

#version 450

layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 outColor;

// enum tl::image::PixelType
const int PixelType_None              = 0;
const int PixelType_L_U8              = 1;
const int PixelType_L_U16             = 2;
const int PixelType_L_U32             = 3;
const int PixelType_L_F16             = 4;
const int PixelType_L_F32             = 5;

const int PixelType_LA_U8             = 6;
const int PixelType_LA_U16            = 7;
const int PixelType_LA_U32            = 8;
const int PixelType_LA_F16            = 9;
const int PixelType_LA_F32            = 10;

const int PixelType_RGB_U8            = 11;
const int PixelType_RGB_U10           = 12;
const int PixelType_RGB_U16           = 13;
const int PixelType_RGB_U32           = 14;
const int PixelType_RGB_F16           = 15;
const int PixelType_RGB_F32           = 16;

const int PixelType_RGBA_U8           = 17;
const int PixelType_RGBA_U16          = 18;
const int PixelType_RGBA_U32          = 19;
const int PixelType_RGBA_F16          = 20;
const int PixelType_RGBA_F32          = 21;

const int PixelType_YUV_420P_U8       = 22;
const int PixelType_YUV_422P_U8       = 23;
const int PixelType_YUV_444P_U8       = 24;

const int PixelType_YUV_420P_U10      = 25;
const int PixelType_YUV_422P_U10      = 26;
const int PixelType_YUV_444P_U10      = 27;

const int PixelType_YUV_420P_U12      = 28;
const int PixelType_YUV_422P_U12      = 29;
const int PixelType_YUV_444P_U12      = 30;

const int PixelType_YUV_420P_U16      = 31;
const int PixelType_YUV_422P_U16      = 32;
const int PixelType_YUV_444P_U16      = 33;

const int PixelType_YUV_420SP_U8      = 34;
const int PixelType_YUV_420SP_U16     = 35;

const int PixelType_YUV_422SP_U8      = 36;
const int PixelType_YUV_422SP_U16     = 37;

const int PixelType_ARGB_4444_Premult = 38;


// enum tl::image::VideoLevels
const uint VideoLevels_FullRange  = 0;
const uint VideoLevels_LegalRange = 1;

float getBitDepth(int pixelType)
{
    if (pixelType == PixelType_YUV_420P_U10 ||
        pixelType == PixelType_YUV_422P_U10 ||
        pixelType == PixelType_YUV_444P_U10)
    {
        return 10.0;
    }
    else if (pixelType == PixelType_YUV_420P_U12 ||
             pixelType == PixelType_YUV_422P_U12 ||
             pixelType == PixelType_YUV_444P_U12)
    {
        return 12.0;
    }
    else if (pixelType == PixelType_YUV_420P_U16 ||
             pixelType == PixelType_YUV_422P_U16 ||
             pixelType == PixelType_YUV_444P_U16 ||
             pixelType == PixelType_YUV_422SP_U16 ||
             pixelType == PixelType_YUV_420SP_U16)
    {
        return 16.0;
    }
    else if (pixelType == PixelType_ARGB_4444_Premult)
    {
        return 4.0;
    }
    else // U8 fallback
    {
        return 8.0;
    }
}

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
    float y = 0; float cr = 0; float cb = 0;

    if (pixelType >= PixelType_YUV_420P_U8 && pixelType <= PixelType_YUV_422SP_U16)
    {
        // 1. Fetch plane channels based on storage type
        if (pixelType == PixelType_YUV_420SP_U8 ||
            pixelType == PixelType_YUV_420SP_U16)
        {
            // Semi-Planar (NV12 / P010 / P016):
            y  = texture(s0, textureCoord).r;
            cb = texture(s1, textureCoord).r;
            cr = texture(s1, textureCoord).g;
        }
        else
        {
            // Fully Planar (YUV 420P/422P/444P):
            y  = texture(s0, textureCoord).r;
            cb = texture(s1, textureCoord).r;
            cr = texture(s2, textureCoord).r;

            // Normalize packed 10-bit / 12-bit in 16-bit textures
            if (pixelType == PixelType_YUV_420P_U10 ||
                pixelType == PixelType_YUV_422P_U10 ||
                pixelType == PixelType_YUV_444P_U10)
            {
                float rangeScale = 1023.0 / 65535.0;
                y  /= rangeScale;
                cb /= rangeScale;
                cr /= rangeScale;
            }
            else if (pixelType == PixelType_YUV_420P_U12 ||
                     pixelType == PixelType_YUV_422P_U12 ||
                     pixelType == PixelType_YUV_444P_U12)
            {
                float rangeScale = 4095.0 / 65535.0;
                y  /= rangeScale;
                cb /= rangeScale;
                cr /= rangeScale;
            }
        }

        // 2. Apply Video Levels (Full vs Legal Range)
        if (videoLevels == VideoLevels_FullRange)
        {
            cb -= 0.5;
            cr -= 0.5;
        }
        else if (videoLevels == VideoLevels_LegalRange)
        {
            float bitDepth = getBitDepth(pixelType);
            float maxValue = pow(2.0, bitDepth) - 1.0;
            float range = pow(2.0, bitDepth - 8.0);

            float yMin = 16.0 * range;
            float yMax = 235.0 * range;
            float cMin = 16.0 * range;
            float cMax = 240.0 * range;

            y  = clamp((y  * maxValue - yMin) / (yMax - yMin), 0.0, 1.0);
            cb = clamp((cb * maxValue - cMin) / (cMax - cMin), 0.0, 1.0) - 0.5;
            cr = clamp((cr * maxValue - cMin) / (cMax - cMin), 0.0, 1.0) - 0.5;
        }

        // 3. YUV to RGB Matrix Transformation
        c.r = y + (yuvCoefficients.x * cr);
        c.g = y - (yuvCoefficients.y * cr) - (yuvCoefficients.z * cb);
        c.b = y + (yuvCoefficients.w * cb);
        c.a = 1.0;
    }
    else
    {
        c = texture(s0, textureCoord);

        if (VideoLevels_LegalRange == videoLevels)
        {
            c.r = (c.r - (16.0 / 255.0)) * (255.0 / (235.0 - 16.0));
            c.g = (c.g - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0));
            c.b = (c.b - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0));
        }

        if (1 == imageChannels)
        {
            c.g = c.b = c.r;
            c.a = 1.0;
        }
        else if (2 == imageChannels)
        {
            c.a = c.g;
            c.g = c.b = c.r;
        }
        else if (3 == imageChannels)
        {
            c.a = 1.0;
        }
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
     // outColor.a = 1.0;
}

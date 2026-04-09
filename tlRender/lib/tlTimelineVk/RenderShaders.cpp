// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlTimelineGL/RenderPrivate.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timeline_vlk
    {
        std::string vertexDummy()
        {
            return R"(#version 450
layout(location = 0) in vec3 vPos;
layout(location = 0) out vec3 fragPosition;

layout(set = 0, binding = 0, std140) uniform Transform {
     mat4 mvp;
} transform;

void main()
{
    fragPosition = vPos;
    gl_Position = transform.mvp * vec4(vPos, 1.0);
})";
        }

        std::string fragmentDummy()
        {
            return R"(#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec4 outColor;
                  
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;       
                 
void main()
{
    // Fake a normal from position (assumes object roughly centered at origin)
    vec3 normal = normalize(inPosition);

    // Simple light direction
    vec3 lightDir = normalize(vec3(0.5, 0.5, 1.0));

    // Diffuse (Lambert)
    float diff = max(dot(normal, lightDir), 0.0);

    // Add a bit of ambient so it's not fully black
    float ambient = 0.2;

    vec3 finalColor = pc.color.rgb * (ambient + diff);

    outColor = vec4(finalColor, pc.color.a);
    outColor = pc.color;
})";
        }
        
        std::string vertexSource()
        {
            return R"(#version 450
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTexture;
layout(location = 0) out vec2 fTexture;
layout(set = 0, binding = 0, std140) uniform Transform {
     mat4 mvp;
} transform;

void main()
{
    gl_Position = transform.mvp * vec4(vPos, 1.0);
    fTexture = vTexture;
})";
        }

        std::string vertex2Source()
        {
            return R"(#version 450
layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vTexture;
layout(location = 0) out vec2 fTexture;
layout(set = 0, binding = 0, std140) uniform Transform {
    mat4 mvp;
} transform;

void main()
{
    gl_Position = transform.mvp * vec4(vPos, 0.0, 1.0);
    fTexture = vTexture;
})";
        }


        std::string vertex2NoUVsSource()
        {
            return R"(#version 450
layout(location = 0) in vec2 vPos;
layout(set = 0, binding = 0, std140) uniform Transform {
    mat4 mvp;
} transform;

void main()
{
    gl_Position = transform.mvp * vec4(vPos, 0.0, 1.0);
})";
        }
        
        std::string meshFragmentSource()
        {
            return R"(#version 450
                   
layout(location = 0) in  vec2 fTexture;
layout(location = 0) out vec4 outColor;
                  
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;       
                 
void main()
{
     outColor = pc.color;
})";
        }

        std::string colorMeshVertexSource()
        {
            return R"(#version 450
                 
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
})";
        }

        std::string colorMeshFragmentSource()
        {
            return R"(#version 450
                 
layout(location = 0) in vec4 fColor;
layout(location = 0) out vec4 outColor;
                 
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc; 

void main()
{
    outColor = fColor * pc.color;
})";
        }

        std::string textFragmentSource()
        {
            return R"(#version 450
                 
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D textureSampler;
                 
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;
                 
void main()
{
     outColor.r = pc.color.r;
     outColor.g = pc.color.g;
     outColor.b = pc.color.b;
     outColor.a = pc.color.a * texture(textureSampler, fTexture).r;
})";
        }

        std::string textureFragmentSource()
        {
            return R"(#version 450
                 
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D textureSampler;

layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;
                 
void main()
{
     outColor = texture(textureSampler, fTexture) * pc.color;
})";
        }

        const std::string videoLevels = R"(
// enum tl::image::VideoLevels
const uint VideoLevels_FullRange  = 0;
const uint VideoLevels_LegalRange = 1;
)";

        std::string imageFragmentSource()
        {
            return R"(#version 450
                                
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

const uint PixelType_YUV_420P_U10      = 25;
const uint PixelType_YUV_422P_U10      = 26;
const uint PixelType_YUV_444P_U10      = 27;

const uint PixelType_YUV_420P_U12      = 28;
const uint PixelType_YUV_422P_U12      = 29;
const uint PixelType_YUV_444P_U12      = 30;

const uint PixelType_YUV_420P_U16      = 31;
const uint PixelType_YUV_422P_U16      = 32;
const uint PixelType_YUV_444P_U16      = 33;

const uint PixelType_ARGB_4444_Premult = 34;
                                
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
             pixelType == PixelType_YUV_444P_U16)
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

       if ((pixelType >= PixelType_YUV_420P_U8 && pixelType <= PixelType_ARGB_4444_Premult))
       {

          float y  = texture(s0, textureCoord).r;
          float cb = texture(s1, textureCoord).r;
          float cr = texture(s2, textureCoord).r;

          // For 10-bit and 12-bit, ensure correct normalization
          if (pixelType == PixelType_YUV_420P_U10 ||
              pixelType == PixelType_YUV_422P_U10 ||
              pixelType == PixelType_YUV_444P_U10)
          {
            // 
            // 10-bit data may be packed in 16-bit textures, normalize to [0,1]
            float rangeScale = 1023.0 / 65535.0; // 1023 = 2^10 - 1
            y  = y / rangeScale; 
            cb = cb / rangeScale;
            cr = cr / rangeScale;
          }
          else if (pixelType == PixelType_YUV_420P_U12 ||
                   pixelType == PixelType_YUV_422P_U12 ||
                   pixelType == PixelType_YUV_444P_U12)
          {
            // 12-bit data may be packed in 16-bit textures, normalize to [0,1]
            float rangeScale = 4095.0 / 65535.0; // 1023 = 2^10 - 1
            y  = y / rangeScale; 
            cb = cb / rangeScale;
            cr = cr / rangeScale;
          }

          if (videoLevels == VideoLevels_FullRange)
          {
            cb -= 0.5;
            cr -= 0.5;
          }
          else if (videoLevels == VideoLevels_LegalRange)
          {
            float bitDepth = getBitDepth(pixelType); 
            float maxValue = pow(2.0, bitDepth) - 1.0;
            float range = pow(2.0, bitDepth - 8);

            // Legal range scaling for YUV (ITU-R BT.601/BT.709)
            float yMin = 16.0 * range;   // 16 << (bitDepth - 8)
            float yMax = 235.0 * range;  // 235 << (bitDepth - 8)
            float cMin = 16.0 * range;   // 16 << (bitDepth - 8)
            float cMax = 240.0 * range;  // 240 << (bitDepth - 8)
            
            // Scale to 0-1 range and normalize
            y = clamp((y * maxValue - yMin) / (yMax - yMin), 0.0, 1.0);
            cb = clamp((cb * maxValue - cMin) / (cMax - cMin), 0.0, 1.0) - 0.5;
            cr = clamp((cr * maxValue - cMin) / (cMax - cMin), 0.0, 1.0) - 0.5;
          }

          c.r = y + (yuvCoefficients.x * cr);
          c.g = y - (yuvCoefficients.y * cr) - (yuvCoefficients.z * cb);
          c.b = y + (yuvCoefficients.w * cb);
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
})";
        }

        std::string debandingFragmentSource(
            const float threshold,
            const float range,
            const int iterations,
            const float grain
            )
        {
            return string::Format(R"(
// Simplified pseudocode representation of the deband algorithm

const float kTHRESHOLD = {0};
const float kRANGE = {1};
const int kITERATIONS = {2};
const float kGRAIN = {3};

// PRNG helpers for randomness
float mod289(float x) { return x - floor(x / 289.0) * 289.0; }
float permute(float x) { return mod289((34.0 * x + 1.0) * x); }
float rand(float x) { return fract(x / 41.0); }

// Main sampling function per pixel
vec4 deband(sampler2D tex, vec2 pos) {
    vec3 m = vec3(pos, rand(pos.x + pos.y)) + vec3(1.0);
    float h = permute(permute(permute(m.x) + m.y) + m.z);

    ivec2 imageSize = textureSize(tex, 0);  // LOD 0 for base level
    vec2 invImageSize = vec2(1.0 / imageSize.x, 1.0 / imageSize.y);

    vec4 col = texture(tex, pos);

    for (int i = 1; i <= kITERATIONS; i++) {
        // Compute local average with increasing range (i * kRANGE)
        float range = i * kRANGE;

        float dist = rand(h) * range; h = permute(h);
        float dir = rand(h) * 6.2831853; h = permute(h);

        // Compute offset vector
        vec2 pt = dist * invImageSize;  // Normalize to texture space
        vec2 o = vec2(cos(dir), sin(dir));

        // Sample 4 points at 90-degree intervals around the pixel
        vec4 ref[4];
        ref[0] = texture(tex, pos + pt * vec2(o.x, o.y));   // SE
        ref[1] = texture(tex, pos + pt * vec2(-o.y, o.x));  // NE
        ref[2] = texture(tex, pos + pt * vec2(-o.x, -o.y)); // NW
        ref[3] = texture(tex, pos + pt * vec2(o.y, -o.x));  // SW

        // Average them
        vec4 avg = (ref[0] + ref[1] + ref[2] + ref[3]) / 4.0;

        // Compute absolute difference
        vec4 diff = abs(col - avg);

        // Dynamic threshold decreases per iteration for finer control
        float iter_threshold = kTHRESHOLD / (i * 16384.0);

        col = mix(avg, col, greaterThan(diff, vec4(iter_threshold)));
    }

    if (kGRAIN > 0) {
        // Generate a 3D noise vector cheaply
        vec3 noise = vec3(
            fract(h * 1.234),
            fract(h * 2.345),
            fract(h * 3.456)
        );
        col.rgb += (kGRAIN * (1.0 / 8192.0)) * (noise - 0.5);
    }

    return col;
}
)")
                .arg(threshold)
                .arg(range)
                .arg(iterations)
                .arg(grain);
        }
        
        std::string displayFragmentSource(
            const std::string& ocioICSDef, const std::string& ocioICS,
            const std::string& ocioDef, const std::string& ocio,
            const std::string& lutDef, const std::string& lut,
            timeline::LUTOrder lutOrder, const std::string& toneMapDef,
            const std::string& toneMap,
            const std::string& debandingDef,
            const std::string& debanding)
        {
            std::vector<std::string> args;
            args.push_back(toneMapDef);   // 0
            args.push_back(videoLevels);  // 1
            args.push_back(ocioICSDef);   // 2
            args.push_back(ocioDef);      // 3
            args.push_back(lutDef);       // 4
            args.push_back(debandingDef); // 5
            args.push_back(debanding);    // 6
            switch (lutOrder)
            {
            case timeline::LUTOrder::PreColorConfig:
                args.push_back(lut);     // 7
                args.push_back(ocioICS); // 8
                args.push_back(toneMap); // 9
                args.push_back(ocio);    // 10
                break;
            case timeline::LUTOrder::PostColorConfig:
                args.push_back(ocioICS); // 7
                args.push_back(lut);     // 8
                args.push_back(toneMap); // 9
                args.push_back(ocio);    // 10
                break;
            default:
                break;
            }
            return string::Format(R"(#version 450
                     
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 outColor;


layout(binding = 1) uniform sampler2D textureSampler;

{0}

// enum tl::timeline::Channels
const uint Channels_Color = 0;
const uint Channels_Red   = 1;
const uint Channels_Green = 2;
const uint Channels_Blue  = 3;
const uint Channels_Alpha = 4;
const uint Channels_Luma = 5;

struct Levels
{
    bool  enabled;
    float inLow;
    float inHigh;
    float gamma;
    float outLow;
    float outHigh;
};

layout(set = 0, binding = 2, std140) uniform LevelsUBO
{
  Levels data;
} uboLevels;

struct Normalize
{
    bool enabled;
    vec4 minimum;
    vec4 maximum;
};

layout(set = 0, binding = 3, std140) uniform NormalizeUBO
{
  Normalize data;
} uboNormalize;

struct Color
{
    bool  enabled;
    vec3  add;
    mat4  matrix;
    bool  invert;
};

layout(set = 0, binding = 4, std140) uniform ColorUBO
{
   Color data;
} uboColor;

// Video Levels
{1}

layout(set = 0, binding = 5, std140) uniform UBO
{
    int        channels;
    int        mirrorX;
    int        mirrorY;
    int        dither;
    float      softClip;
    int        videoLevels;
    int        invalidValues;
} ubo;

vec4 colorFunc(vec4 value, vec3 add, mat4 m)
{
    vec4 tmp;
    tmp[0] = value[0] + add[0];
    tmp[1] = value[1] + add[1];
    tmp[2] = value[2] + add[2];
    tmp[3] = 1.0;
    tmp *= m;
    tmp[3] = value[3];
    return tmp;
}

vec4 levelsFunc(vec4 value, Levels data)
{
    vec4 tmp;
    float levelsRange = data.inHigh - data.inLow;
    if (levelsRange > 0)
    {
      tmp[0] = clamp((value[0] - data.inLow) / levelsRange, 0, 1);
      tmp[1] = clamp((value[1] - data.inLow) / levelsRange, 0, 1);
      tmp[2] = clamp((value[2] - data.inLow) / levelsRange, 0, 1);
   }
    if (tmp[0] >= 0.0)
        tmp[0] = pow(tmp[0], data.gamma);
    if (tmp[1] >= 0.0)
        tmp[1] = pow(tmp[1], data.gamma);
    if (tmp[2] >= 0.0)
        tmp[2] = pow(tmp[2], data.gamma);
    value[0] = tmp[0] * data.outHigh + data.outLow;
    value[1] = tmp[1] * data.outHigh + data.outLow;
    value[2] = tmp[2] * data.outHigh + data.outLow;
    return value;
}

vec4 softClipFunc(vec4 value, float softClip)
{
    float tmp = 1.0 - softClip;
    if (value[0] > tmp)
        value[0] = tmp + (1.0 - exp(-(value[0] - tmp) 
/ softClip)) * softClip;
    if (value[1] > tmp)
        value[1] = tmp + (1.0 - exp(-(value[1] - tmp) 
/ softClip)) * softClip;
    if (value[2] > tmp)
        value[2] = tmp + (1.0 - exp(-(value[2] - tmp) 
/ softClip)) * softClip;
    return value;
}

vec4 normalizeFunc(vec4 value, Normalize data)
{
    value[0] = (value[0] - data.minimum[0]) / 
(data.maximum[0] - data.minimum[0]);
    value[1] = (value[1] - data.minimum[1]) / 
(data.maximum[1] - data.minimum[1]);
    value[2] = (value[2] - data.minimum[2]) / 
(data.maximum[2] - data.minimum[2]);
    value[3] = (value[3] - data.minimum[3]) / 
(data.maximum[3] - data.minimum[3]);
    return value;
}


// ocioICSDef
{2}

// ocioDef
{3}

// lutDef
{4}

// DefandingDef
{5}


void main()
{
    vec2 t = fTexture;
    if (1 == ubo.mirrorX)
    {
        t.x = 1.0 - t.x;
    }
    if (1 == ubo.mirrorY)
    {
        t.y = 1.0 - t.y;
    }

    // Read texture with or without debanding.
    {6}

    // Video levels.
    if (VideoLevels_LegalRange == ubo.videoLevels)
    {
        const float scale = (940.0 - 64.0) / 1023.0;
        const float offset = 64.0 / 1023.0;
        outColor.r = outColor.r * scale + offset;
        outColor.g = outColor.g * scale + offset;
        outColor.b = outColor.b * scale + offset;
    }

    // Apply color tranform to linear space and LUT (or vicecersa).
    {7}
    {8}

    // Apply color transformations in linear space.
    if (uboColor.data.enabled)
    {
        outColor = colorFunc(outColor, uboColor.data.add, uboColor.data.matrix);
        if (uboColor.data.invert)
        {
            outColor.r = 1.0 - outColor.r;
            outColor.g = 1.0 - outColor.g;
            outColor.b = 1.0 - outColor.b;
        }
    }
    if (ubo.softClip > 0.0)
    {
        outColor = softClipFunc(outColor, ubo.softClip);
    }

    // Call libplacebo tonemapping (may go to PQ space after this)
    {9}

    // Apply OCIO Display/View.
    {10}

    if (uboLevels.data.enabled)
    {
        outColor = levelsFunc(outColor, uboLevels.data);
    }
    if (uboNormalize.data.enabled)
    {
        outColor = normalizeFunc(outColor, uboNormalize.data);
    }
    if (ubo.invalidValues == 1)
    {
        if (outColor.r < 0.0 || outColor.r > 1.0 ||
            outColor.g < 0.0 || outColor.g > 1.0 ||
            outColor.b < 0.0 || outColor.b > 1.0 ||
            outColor.a < 0.0 || outColor.a > 1.0)
        {
           outColor.r = 1.0f;
           outColor.g *= 0.5f;
           outColor.b *= 0.5f;
        }
    }

    // Swizzle for the channels display.
    if (Channels_Red == ubo.channels)
    {
        outColor.g = outColor.r;
        outColor.b = outColor.r;
    }
    else if (Channels_Green == ubo.channels)
    {
        outColor.r = outColor.g;
        outColor.b = outColor.g;
    }
    else if (Channels_Blue == ubo.channels)
    {
        outColor.r = outColor.b;
        outColor.g = outColor.b;
    }
    else if (Channels_Alpha == ubo.channels)
    {
        outColor.r = outColor.a;
        outColor.g = outColor.a;
        outColor.b = outColor.a;
    }
    else if (Channels_Luma == ubo.channels)
    {
        float t = (outColor.r + outColor.g + outColor.b) / 3.0;
        outColor.r = t;
        outColor.g = t;
        outColor.b = t;
    }
})")
                .arg(args[0])
                .arg(args[1])
                .arg(args[2])
                .arg(args[3])
                .arg(args[4])
                .arg(args[5])
                .arg(args[6])
                .arg(args[7])
                .arg(args[8])
                .arg(args[9])
                .arg(args[10]);
        }

        std::string differenceFragmentSource()
        {
            return R"(#version 450
                 
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 outColor;
                 
layout(binding = 1) uniform sampler2D textureSampler;
layout(binding = 2) uniform sampler2D textureSamplerB;
                 
void main()
{
    vec4 c = texture(textureSampler, fTexture);
    vec4 cB = texture(textureSamplerB, fTexture);
    outColor.r = abs(c.r - cB.r);
    outColor.g = abs(c.g - cB.g);
    outColor.b = abs(c.b - cB.b);
    outColor.a = max(c.a, cB.a);
})";
        }


        std::string softFragmentSource()
        {
            return R"(#version 450
              
layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 fColor;
            
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;

void main()
{
     vec2       v = fTexture - vec2(0.5, 0.5);
     float ratio  = 1 - sqrt(v.x * v.x + v.y * v.y);
     float radius = 0.75;
     float feather = 0.25;
     float mult = smoothstep(radius - feather, radius + feather, ratio);
     fColor = pc.color;
     fColor.a *= mult;
})";
    }

        std::string hardFragmentSource()
        {
            return R"(#version 450

layout(location = 0) in vec2 fTexture;
layout(location = 0) out vec4 fColor;
              
layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;
              
void main()
{
     fColor = pc.color;
})";
    }

        std::string computeRGB_F16_To_RGBA_F16()
        {
            return R"(
#version 450

layout (local_size_x = 16, local_size_y = 16) in;

// Binding 0: Raw Buffer treated as 32-bit unsigned ints
layout(std430, binding = 0) readonly buffer InputBuffer {
    uint data[];
} inputBuffer;

layout(binding = 1, rgba16f) uniform writeonly image2D outputImage;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outputImage);

    if (texelCoord.x < size.x && texelCoord.y < size.y) {
        // Each pixel has 3 components of 16 bits = 48 bits total per pixel.
        // Total bits before this pixel = (y * width + x) * 48
        uint totalBits = (texelCoord.y * size.x + texelCoord.x) * 48;
        
        // Find which 32-bit index the pixel starts at
        uint wordIdx = totalBits / 32;
        uint bitOffset = totalBits % 32;

        uint r_bits, g_bits, b_bits;

        // Extracting R, G, B from the bitstream
        if (bitOffset == 0) {
            // Pixel is aligned: [ R16 | G16 ] [ B16 | ... ]
            uint word0 = inputBuffer.data[wordIdx];
            uint word1 = inputBuffer.data[wordIdx + 1];
            r_bits = word0 & 0xFFFF;
            g_bits = word0 >> 16;
            b_bits = word1 & 0xFFFF;
        } else {
            // Pixel is split: [ ... | R16 ] [ G16 | B16 ]
            uint word0 = inputBuffer.data[wordIdx];
            uint word1 = inputBuffer.data[wordIdx + 1];
            r_bits = word0 >> 16;
            g_bits = word1 & 0xFFFF;
            b_bits = word1 >> 16;
        }

        // unpackHalf2x16 takes a uint32. We put our 16 bits in the low slot.
        float r = unpackHalf2x16(r_bits).x;
        float g = unpackHalf2x16(g_bits).x;
        float b = unpackHalf2x16(b_bits).x;

        imageStore(outputImage, texelCoord, vec4(r, g, b, 1.0));
    }
}
)";
        }    

        std::string computeRGB_F32_To_RGBA_F32()
        {
            return R"(
#version 450

layout (local_size_x = 16, local_size_y = 16) in;

// Binding 0: Raw Buffer containing float32 data as uints
layout(std430, binding = 0) readonly buffer InputBuffer {
    uint data[];
} inputBuffer;

// Changed to rgba32f to match the 32-bit precision
layout(binding = 1, rgba32f) uniform writeonly image2D outputImage;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outputImage);

    if (texelCoord.x < size.x && texelCoord.y < size.y) {
        // Each pixel has 3 components of 32 bits = 96 bits total per pixel.
        // This corresponds to exactly 3 indices in the uint array.
        uint pixelIdx = (texelCoord.y * size.x + texelCoord.x) * 3;

        // Grab the raw bits for R, G, and B
        uint r_uint = inputBuffer.data[pixelIdx];
        uint g_uint = inputBuffer.data[pixelIdx + 1];
        uint b_uint = inputBuffer.data[pixelIdx + 2];

        // Convert the bit patterns directly to floats
        // This is a standard GLSL function, no extension required.
        float r = uintBitsToFloat(r_uint);
        float g = uintBitsToFloat(g_uint);
        float b = uintBitsToFloat(b_uint);

        // Write out to the RGBA image (Alpha set to 1.0)
        imageStore(outputImage, texelCoord, vec4(r, g, b, 1.0));
    }
}
)";
        }
        
        std::string computeHDRPeakDetection()
        {
#ifdef __APPLE__
            // macOS compatible version
            return R"(
#version 450
#extension GL_KHR_shader_subgroup_arithmetic : enable

#define SLICES 12
#define PQ_BITS 14
#define PQ_MAX ((1 << PQ_BITS) - 1)
#define HIST_BITS 7
#define HIST_BIAS 64
#define HIST_BINS 64

            layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

            layout(set = 0, binding = 0) uniform sampler2D img;

            struct PeakData {
                uint frame_wg_count[SLICES];
                uint frame_wg_active[SLICES];
                uint frame_sum_pq[SLICES];
                uint frame_max_pq[SLICES];
                uint frame_hist[SLICES * HIST_BINS];
            };

            // Output SSBO
            layout(std430, set = 0, binding = 1) buffer PeakBuffer {
                PeakData data;
            };

            shared uint wg_sum;
            shared uint wg_max;
            shared uint wg_active_count;
            shared uint wg_hist[HIST_BINS];

            void main() {
                uvec2 pos = gl_GlobalInvocationID.xy;
                uvec2 img_size = textureSize(img, 0);
    
                // SAFETY: All threads must reach the barrier. We use a boolean mask 
                // instead of an early 'return'.
                bool valid = (pos.x < img_size.x && pos.y < img_size.y);

                // 1. Initialize Shared Memory (All threads participate)
                uint local_idx = gl_LocalInvocationIndex;
                uint wg_size = gl_WorkGroupSize.x * gl_WorkGroupSize.y;
    
                if (local_idx == 0) {
                    wg_sum = 0;
                    wg_max = 0;
                    wg_active_count = 0;
                }
                for (uint i = local_idx; i < HIST_BINS; i += wg_size) {
                    wg_hist[i] = 0;
                }
    
                // Barrier is safe because no thread returned early
                barrier();

                uint y_pq = 0;
                bool is_active = false;

                if (valid) {
                    vec3 color = texture(img, vec2(pos) / vec2(img_size)).rgb;
                    // Use MaxRGB for better peak detection (MoltenVK compatible)
                    float luma = max(color.r, max(color.g, color.b));
        
                    // Manual PQ OETF (Linear 0..1 to PQ 0..1)
                    // Adjust these constants if your input scaling is different
                    luma = clamp(luma, 0.0, 1.0);
                    float l = pow(luma, 0.1593017578125);
                    l = (0.8359375 + 18.8515625 * l) / (1.0 + 18.6875 * l);
                    l = pow(l, 78.84375);
        
                    y_pq = uint(l * PQ_MAX + 0.5);
                    is_active = (y_pq > 0);
                }

                // 2. Subgroup Reductions
                // We use subgroupAdd(uint) which is widely supported in Metal
                uint sub_sum = subgroupAdd(y_pq);
                uint sub_max = subgroupMax(y_pq);
                uint sub_active = subgroupAdd(is_active ? 1u : 0u);

                if (subgroupElect()) {
                    atomicAdd(wg_sum, sub_sum);
                    atomicMax(wg_max, sub_max);
                    atomicAdd(wg_active_count, sub_active);
                }

                // 3. Histogram Update
                if (valid && is_active) {
                    int bin = int(y_pq >> (PQ_BITS - HIST_BITS)) - HIST_BIAS;
                    if (bin >= 0 && bin < HIST_BINS) {
                        atomicAdd(wg_hist[bin], 1u);
                    }
                }

                barrier();

                // 4. Final Global Update
                if (local_idx == 0) {
                    uint slice = (gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x) % SLICES;
                    atomicAdd(data.frame_wg_count[slice], 1u);
        
                    if (wg_active_count > 0) {
                        atomicAdd(data.frame_wg_active[slice], 1u);
                        atomicAdd(data.frame_sum_pq[slice], wg_sum / wg_active_count);
                        atomicMax(data.frame_max_pq[slice], wg_max);
                    }
                }

                // Parallel Global Histogram Write
                uint slice_offset = ((gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x) % SLICES) * HIST_BINS;
                for (uint i = local_idx; i < HIST_BINS; i += wg_size) {
                    if (wg_hist[i] > 0) {
                        atomicAdd(data.frame_hist[slice_offset + i], wg_hist[i]);
                    }
                }
            }
)";
        }
#else
            return R"(
#version 450
#extension GL_KHR_shader_subgroup_arithmetic : enable
#extension GL_KHR_shader_subgroup_ballot : enable

#define SLICES 12
#define PQ_BITS 14
#define PQ_MAX ((1 << PQ_BITS) - 1)
#define HIST_BITS 7
#define HIST_BIAS 64
#define HIST_BINS 64

// Nvidia RTX 3080 has a warp size of 32. 
// A 16x16 local size gives us exactly 8 warps per workgroup.
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform sampler2D img;

struct PeakData {
    uint frame_wg_count[SLICES];
    uint frame_wg_active[SLICES];
    uint frame_sum_pq[SLICES];
    uint frame_max_pq[SLICES];
    uint frame_hist[SLICES * HIST_BINS];
};

layout(std430, set = 0, binding = 1) buffer PeakBuffer {
    PeakData data;
};

// Luma coefficients (libplacebo style)
const vec3 LUMA_COEFFS = vec3(0.2627, 0.6780, 0.0593);

// Use shared memory for workgroup-wide reduction
shared uint wg_sum;
shared uint wg_max;
shared uint wg_active_count;
shared uint wg_hist[HIST_BINS];

void main() {
    uvec2 pos = gl_GlobalInvocationID.xy;
    uvec2 img_size = textureSize(img, 0);
    
    // On NVIDIA, early returns are fine as long as they don't bypass 
    // a barrier that other threads in the same warp need.
    bool within_bounds = (pos.x < img_size.x && pos.y < img_size.y);

    uint local_idx = gl_LocalInvocationIndex;
    uint wg_size = gl_WorkGroupSize.x * gl_WorkGroupSize.y;
    
    // 1. Fast Parallel Clear
    if (local_idx == 0) {
        wg_sum = 0;
        wg_max = 0;
        wg_active_count = 0;
    }
    for (uint i = local_idx; i < HIST_BINS; i += wg_size) {
        wg_hist[i] = 0;
    }
    
    // Sync shared memory
    barrier();

    uint y_pq = 0;
    bool is_active = false;

    if (within_bounds) {
        vec3 color = vec3(1,1,1); //texture(img, vec2(pos) / vec2(img_size)).rgb;
        // RTX 3080 handles max() extremely fast
        // float luma = dot(color, LUMA_COEFFS);  // Weighted for accuracy
        float luma = max(color.r, max(color.g, color.b));
        
        // Fast PQ OETF approximation or full precision
        luma = clamp(luma, 0.0, 1.0);
        float l = pow(luma, 0.1593017578125);
        l = (0.8359375 + 18.8515625 * l) / (1.0 + 18.6875 * l);
        l = pow(l, 78.84375);
        
        y_pq = uint(l * PQ_MAX + 0.5);
        is_active = (y_pq > 0);
    }

    // 2. Warp-Level Reduction (RTX 3080/Ampere optimized)
    // Subgroup functions map directly to NVIDIA 'shfl' and 'popc' instructions
    uint warp_sum = subgroupAdd(y_pq);
    uint warp_max = subgroupMax(y_pq);
    uint warp_active = subgroupAdd(is_active ? 1u : 0u);

    // Only one thread per warp updates shared memory (Reduces bank conflicts)
    if (subgroupElect()) {
        atomicAdd(wg_sum, warp_sum);
        atomicMax(wg_max, warp_max);
        atomicAdd(wg_active_count, warp_active);
    }

    // 3. Histogram Update with Shared Memory
    if (within_bounds && is_active) {
        int bin = int(y_pq >> (PQ_BITS - HIST_BITS)) - HIST_BIAS;
        if (bin >= 0 && bin < HIST_BINS) {
            // Ampere has high-speed shared atomics
            atomicAdd(wg_hist[bin], 1u);
        }
    }

    barrier();

    // 4. Global SSBO Update (One thread per workgroup)
    if (local_idx == 0) {
        // Use a modulo-based slice to distribute atomic pressure across the SSBO
        uint slice = (gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x) % SLICES;
        
        atomicAdd(data.frame_wg_count[slice], 1u);
        
        if (wg_active_count > 0) {
            atomicAdd(data.frame_wg_active[slice], 1u);
            // On high-end GPUs, we can calculate the average here
            atomicAdd(data.frame_sum_pq[slice], wg_sum / wg_active_count);
            atomicMax(data.frame_max_pq[slice], wg_max);
        }
    }

    // 5. Global Histogram Merge
    // Distribute the 64 bins across the 256 threads for a single coalesced write
    uint slice_offset = ((gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x) % SLICES) * HIST_BINS;
    if (local_idx < HIST_BINS) {
        uint val = wg_hist[local_idx];
        if (val > 0) {
            atomicAdd(data.frame_hist[slice_offset + local_idx], val);
        }
    }
}
)";
}
#endif
    } // namespace timeline_vlk
} // namespace tl

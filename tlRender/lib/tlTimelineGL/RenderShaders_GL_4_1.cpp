// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineGL/RenderPrivate.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timeline_gl
    {
        std::string vertexSource()
        {
            return "#version 410\n"
                   "\n"
                   "in vec3 vPos;\n"
                   "in vec2 vTexture;\n"
                   "out vec2 fTexture;\n"
                   "\n"
                   "struct Transform\n"
                   "{\n"
                   "    mat4 mvp;\n"
                   "};\n"
                   "\n"
                   "uniform Transform transform;\n"
                   "\n"
                   "void main()\n"
                   "{\n"
                   "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                   "    fTexture = vTexture;\n"
                   "}\n";
        }

        std::string meshFragmentSource()
        {
            return "#version 410\n"
                   "\n"
                   "out vec4 outColor;\n"
                   "\n"
                   "uniform vec4 color;\n"
                   "\n"
                   "void main()\n"
                   "{\n"
                   "\n"
                   "    outColor = color;\n"
                   "}\n";
        }

        std::string colorMeshVertexSource()
        {
            return "#version 410\n"
                   "\n"
                   "layout(location = 0) in vec3 vPos;\n"
                   "layout(location = 1) in vec4 vColor;\n"
                   "out vec4 fColor;\n"
                   "\n"
                   "struct Transform\n"
                   "{\n"
                   "    mat4 mvp;\n"
                   "};\n"
                   "\n"
                   "uniform Transform transform;\n"
                   "\n"
                   "void main()\n"
                   "{\n"
                   "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                   "    fColor = vColor;\n"
                   "}\n";
        }

        std::string colorMeshFragmentSource()
        {
            return "#version 410\n"
                   "\n"
                   "in vec4 fColor;\n"
                   "out vec4 outColor;\n"
                   "\n"
                   "uniform vec4 color;\n"
                   "\n"
                   "void main()\n"
                   "{\n"
                   "\n"
                   "    outColor = fColor * color;\n"
                   "}\n";
        }

        std::string textFragmentSource()
        {
            return "#version 410\n"
                   "\n"
                   "in vec2 fTexture;\n"
                   "out vec4 outColor;\n"
                   "\n"
                   "uniform vec4 color;\n"
                   "uniform sampler2D textureSampler;\n"
                   "\n"
                   "void main()\n"
                   "{\n"
                   "\n"
                   "    outColor.r = color.r;\n"
                   "    outColor.g = color.g;\n"
                   "    outColor.b = color.b;\n"
                   "    outColor.a = color.a * texture(textureSampler, "
                   "fTexture).r;\n"
                   "}\n";
        }

        std::string textureFragmentSource()
        {
            return "#version 410\n"
                   "\n"
                   "in vec2 fTexture;\n"
                   "out vec4 outColor;\n"
                   "\n"
                   "uniform vec4 color;\n"
                   "uniform sampler2D textureSampler;\n"
                   "\n"
                   "void main()\n"
                   "{\n"
                   "\n"
                   "    outColor = texture(textureSampler, fTexture) * color;\n"
                   "}\n";
        }

        namespace
        {
            const std::string pixelType =
                "// enum tl::image::PixelType\n"
                "const uint PixelType_None              = 0;\n"
                "const uint PixelType_L_U8              = 1;\n"
                "const uint PixelType_L_U16             = 2;\n"
                "const uint PixelType_L_U32             = 3;\n"
                "const uint PixelType_L_F16             = 4;\n"
                "const uint PixelType_L_F32             = 5;\n"
                "const uint PixelType_LA_U8             = 6;\n"
                "const uint PixelType_LA_U32            = 7;\n"
                "const uint PixelType_LA_U16            = 8;\n"
                "const uint PixelType_LA_F16            = 9;\n"
                "const uint PixelType_LA_F32            = 10;\n"
                "const uint PixelType_RGB_U8            = 11;\n"
                "const uint PixelType_RGB_U10           = 12;\n"
                "const uint PixelType_RGB_U16           = 13;\n"
                "const uint PixelType_RGB_U32           = 14;\n"
                "const uint PixelType_RGB_F16           = 15;\n"
                "const uint PixelType_RGB_F32           = 16;\n"
                "const uint PixelType_RGBA_U8           = 17;\n"
                "const uint PixelType_RGBA_U16          = 18;\n"
                "const uint PixelType_RGBA_U32          = 19;\n"
                "const uint PixelType_RGBA_F16          = 20;\n"
                "const uint PixelType_RGBA_F32          = 21;\n"
                "const uint PixelType_YUV_420P_U8       = 22;\n"
                "const uint PixelType_YUV_422P_U8       = 23;\n"
                "const uint PixelType_YUV_444P_U8       = 24;\n"
                "const uint PixelType_YUV_420P_U10      = 25;\n"
                "const uint PixelType_YUV_422P_U10      = 26;\n"
                "const uint PixelType_YUV_444P_U10      = 27;\n"
                "const uint PixelType_YUV_420P_U12      = 28;\n"
                "const uint PixelType_YUV_422P_U12      = 29;\n"
                "const uint PixelType_YUV_444P_U12      = 30;\n"
                "const uint PixelType_YUV_420P_U16      = 31;\n"
                "const uint PixelType_YUV_422P_U16      = 32;\n"
                "const uint PixelType_YUV_444P_U16      = 33;\n"
                "const uint PixelType_ARGB_4444_Premult = 34;\n";

            const std::string videoLevels =
                "// enum tl::image::VideoLevels\n"
                "const uint VideoLevels_FullRange  = 0;\n"
                "const uint VideoLevels_LegalRange = 1;\n";

            const std::string sampleTexture = R"(
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
       if ((pixelType >= PixelType_YUV_420P_U8 && pixelType <= PixelType_YUV_444P_U16))
       {

          float y  = texture(s0, textureCoord).r;
          float cb = texture(s1, textureCoord).r;
          float cr = texture(s2, textureCoord).r;

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
              
             // This was needed in OpenGL, but not for Vulkan
             // Swizzle for the image channels.
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
            )";
        } // namespace

        std::string imageFragmentSource()
        {
            return string::Format("#version 410\n"
                                  "\n"
                                  "in vec2 fTexture;\n"
                                  "out vec4 outColor;\n"
                                  "\n"
                                  "{0}\n"
                                  "\n"
                                  "{1}\n"
                                  "\n"
                                  "{2}\n"
                                  "\n"
                                  "uniform vec4      color;\n"
                                  "uniform int       pixelType;\n"
                                  "uniform int       videoLevels;\n"
                                  "uniform vec4      yuvCoefficients;\n"
                                  "uniform int       imageChannels;\n"
                                  "uniform int       mirrorX;\n"
                                  "uniform int       mirrorY;\n"
                                  "uniform sampler2D textureSampler0;\n"
                                  "uniform sampler2D textureSampler1;\n"
                                  "uniform sampler2D textureSampler2;\n"
                                  "\n"
                                  "void main()\n"
                                  "{\n"
                                  "    vec2 t = fTexture;\n"
                                  "    if (1 == mirrorX)\n"
                                  "    {\n"
                                  "        t.x = 1.0 - t.x;\n"
                                  "    }\n"
                                  "    if (0 == mirrorY)\n"
                                  "    {\n"
                                  "        t.y = 1.0 - t.y;\n"
                                  "    }\n"
                                  "    outColor = sampleTexture("
                                  "        t,\n"
                                  "        pixelType,\n"
                                  "        videoLevels,\n"
                                  "        yuvCoefficients,\n"
                                  "        imageChannels,\n"
                                  "        textureSampler0,\n"
                                  "        textureSampler1,\n"
                                  "        textureSampler2) *\n"
                                  "        color;\n"
                                  //"    outColor.a = 1.0;\n"
                                  "}\n")
                .arg(pixelType)
                .arg(videoLevels)
                .arg(sampleTexture);
        }

        std::string displayFragmentSource(
            const std::string& ocioICSDef, const std::string& ocioICS,
            const std::string& ocioDef, const std::string& ocio,
            const std::string& lutDef, const std::string& lut,
            timeline::LUTOrder lutOrder, const std::string& toneMapDef,
            const std::string& toneMap)
        {
            std::vector<std::string> args;
            args.push_back(videoLevels); // 0
            args.push_back(ocioICSDef);  // 1
            args.push_back(ocioDef);     // 2
            args.push_back(lutDef);      // 3
            args.push_back(toneMapDef);  // 4
            switch (lutOrder)
            {
            case timeline::LUTOrder::PreColorConfig:
                args.push_back(lut);     // 5
                args.push_back(ocioICS); // 6
                args.push_back(toneMap); // 7
                args.push_back(ocio);    // 8
                break;
            case timeline::LUTOrder::PostColorConfig:
                args.push_back(ocioICS); // 5
                args.push_back(lut);     // 6
                args.push_back(toneMap); // 7
                args.push_back(ocio);    // 8
                break;
            default:
                break;
            }
            return string::Format(
                       "#version 410\n"
                       "\n"
                       "in vec2 fTexture;\n"
                       "out vec4 outColor;\n"
                       "\n"
                       "// enum tl::timeline::Channels\n"
                       "const uint Channels_Color = 0;\n"
                       "const uint Channels_Red   = 1;\n"
                       "const uint Channels_Green = 2;\n"
                       "const uint Channels_Blue  = 3;\n"
                       "const uint Channels_Alpha = 4;\n"
                       "const uint Channels_Lumma = 5;\n"
                       "\n"
                       "struct Levels\n"
                       "{\n"
                       "    float inLow;\n"
                       "    float inHigh;\n"
                       "    float gamma;\n"
                       "    float outLow;\n"
                       "    float outHigh;\n"
                       "};\n"
                       "\n"
                       "struct EXRDisplay\n"
                       "{\n"
                       "    float v;\n"
                       "    float d;\n"
                       "    float k;\n"
                       "    float f;\n"
                       "    float g;\n"
                       "};\n"
                       "\n"
                       "struct Normalize\n"
                       "{\n"
                       "    vec4 minimum;\n"
                       "    vec4 maximum;\n"
                       "};\n"
                       "\n"
                       "{0}\n"
                       "\n"
                       "uniform sampler2D textureSampler;\n"
                       "\n"
                       "uniform int        channels;\n"
                       "uniform int        mirrorX;\n"
                       "uniform int        mirrorY;\n"
                       "uniform bool       colorEnabled;\n"
                       "uniform vec3       colorAdd;\n"
                       "uniform mat4       colorMatrix;\n"
                       "uniform bool       colorInvert;\n"
                       "uniform bool       levelsEnabled;\n"
                       "uniform Levels     levels;\n"
                       "uniform bool       exrDisplayEnabled;\n"
                       "uniform EXRDisplay exrDisplay;\n"
                       "uniform float      softClip;\n"
                       "uniform int        videoLevels;\n"
                       "uniform bool       normalizeEnabled;\n"
                       "uniform Normalize  normalizeDisplay;\n"
                       "uniform bool       invalidValues;\n"
                       "\n"
                       "vec4 colorFunc(vec4 value, vec3 add, mat4 m)\n"
                       "{\n"
                       "    vec4 tmp;\n"
                       "    tmp[0] = value[0] + add[0];\n"
                       "    tmp[1] = value[1] + add[1];\n"
                       "    tmp[2] = value[2] + add[2];\n"
                       "    tmp[3] = 1.0;\n"
                       "    tmp *= m;\n"
                       "    tmp[3] = value[3];\n"
                       "    return tmp;\n"
                       "}\n"
                       "\n"
                       "vec4 levelsFunc(vec4 value, Levels data)\n"
                       "{\n"
                       "    vec4 tmp;\n"
                       "    tmp[0] = (value[0] - data.inLow) / data.inHigh;\n"
                       "    tmp[1] = (value[1] - data.inLow) / data.inHigh;\n"
                       "    tmp[2] = (value[2] - data.inLow) / data.inHigh;\n"
                       "    if (tmp[0] >= 0.0)\n"
                       "        tmp[0] = pow(tmp[0], data.gamma);\n"
                       "    if (tmp[1] >= 0.0)\n"
                       "        tmp[1] = pow(tmp[1], data.gamma);\n"
                       "    if (tmp[2] >= 0.0)\n"
                       "        tmp[2] = pow(tmp[2], data.gamma);\n"
                       "    value[0] = tmp[0] * data.outHigh + data.outLow;\n"
                       "    value[1] = tmp[1] * data.outHigh + data.outLow;\n"
                       "    value[2] = tmp[2] * data.outHigh + data.outLow;\n"
                       "    return value;\n"
                       "}\n"
                       "\n"
                       "vec4 softClipFunc(vec4 value, float softClip)\n"
                       "{\n"
                       "    float tmp = 1.0 - softClip;\n"
                       "    if (value[0] > tmp)\n"
                       "        value[0] = tmp + (1.0 - exp(-(value[0] - tmp) "
                       "/ softClip)) * softClip;\n"
                       "    if (value[1] > tmp)\n"
                       "        value[1] = tmp + (1.0 - exp(-(value[1] - tmp) "
                       "/ softClip)) * softClip;\n"
                       "    if (value[2] > tmp)\n"
                       "        value[2] = tmp + (1.0 - exp(-(value[2] - tmp) "
                       "/ softClip)) * softClip;\n"
                       "    return value;\n"
                       "}\n"
                       "\n"
                       "float knee(float value, float f)\n"
                       "{\n"
                       "    return log(value * f + 1.0) / f;\n"
                       "}\n"
                       "\n"
                       "vec4 exrDisplayFunc(vec4 value, EXRDisplay data)\n"
                       "{\n"
                       "    value[0] = max(0.0, value[0] - data.d) * data.v;\n"
                       "    value[1] = max(0.0, value[1] - data.d) * data.v;\n"
                       "    value[2] = max(0.0, value[2] - data.d) * data.v;\n"
                       "    if (value[0] > data.k)\n"
                       "        value[0] = data.k + knee(value[0] - data.k, "
                       "data.f);\n"
                       "    if (value[1] > data.k)\n"
                       "        value[1] = data.k + knee(value[1] - data.k, "
                       "data.f);\n"
                       "    if (value[2] > data.k)\n"
                       "        value[2] = data.k + knee(value[2] - data.k, "
                       "data.f);\n"
                       "    if (value[0] > 0.0) value[0] = pow(value[0], "
                       "data.g);\n"
                       "    if (value[1] > 0.0) value[1] = pow(value[1], "
                       "data.g);\n"
                       "    if (value[2] > 0.0) value[2] = pow(value[2], "
                       "data.g);\n"
                       "    float s = pow(2, -3.5 * data.g);\n"
                       "    value[0] *= s;\n"
                       "    value[1] *= s;\n"
                       "    value[2] *= s;\n"
                       "    return value;\n"
                       "}\n"
                       "\n"
                       "vec4 normalizeFunc(vec4 value, Normalize data)\n"
                       "{\n"
                       "    value[0] = (value[0] - data.minimum[0]) / "
                       "(data.maximum[0] - data.minimum[0]);\n"
                       "    value[1] = (value[1] - data.minimum[1]) / "
                       "(data.maximum[1] - data.minimum[1]);\n"
                       "    value[2] = (value[2] - data.minimum[2]) / "
                       "(data.maximum[2] - data.minimum[2]);\n"
                       "    value[3] = (value[3] - data.minimum[3]) / "
                       "(data.maximum[3] - data.minimum[3]);\n"
                       "    return value;\n"
                       "}\n"
                       "\n"
                       "{1}\n"
                       "\n"
                       "{2}\n"
                       "\n"
                       "{3}\n"
                       "\n"
                       "{4}\n"
                       "\n"
                       "void main()\n"
                       "{\n"
                       "    vec2 t = fTexture;\n"
                       "    if (1 == mirrorX)\n"
                       "    {\n"
                       "        t.x = 1.0 - t.x;\n"
                       "    }\n"
                       "    if (1 == mirrorY)\n"
                       "    {\n"
                       "        t.y = 1.0 - t.y;\n"
                       "    }\n"
                       "\n"
                       "    outColor = texture(textureSampler, t);\n"
                       "\n"
                       "    // Video levels.\n"
                       "    if (VideoLevels_LegalRange == videoLevels)\n"
                       "    {\n"
                       "        const float scale = (940.0 - 64.0) / 1023.0;\n"
                       "        const float offset = 64.0 / 1023.0;\n"
                       "        outColor.r = outColor.r * scale + offset;\n"
                       "        outColor.g = outColor.g * scale + offset;\n"
                       "        outColor.b = outColor.b * scale + offset;\n"
                       "    }\n"
                       "\n"
                       "    // Apply color tranform to linear space and LUT.\n"
                       "    {5}\n"
                       "    {6}\n"
                       "\n"
                       "    // Call libplacebo tonemapping\n"
                       "    {7}\n"
                       "\n"
                       "    // Apply color transformations.\n"
                       "    if (colorEnabled)\n"
                       "    {\n"
                       "        outColor = colorFunc(outColor, colorAdd, "
                       "colorMatrix);\n"
                       "    }\n"
                       "    if (colorInvert)\n"
                       "    {\n"
                       "        outColor.r = 1.0 - outColor.r;\n"
                       "        outColor.g = 1.0 - outColor.g;\n"
                       "        outColor.b = 1.0 - outColor.b;\n"
                       "    }\n"
                       "    if (exrDisplayEnabled)\n"
                       "    {\n"
                       "        outColor = exrDisplayFunc(outColor, "
                       "exrDisplay);\n"
                       "    }\n"
                       "    if (softClip > 0.0)\n"
                       "    {\n"
                       "        outColor = softClipFunc(outColor, softClip);\n"
                       "    }\n"
                       "\n"
                       "    // Apply OCIO Display/View.\n"
                       "    {8}\n"
                       "\n"
                       "    if (levelsEnabled)\n"
                       "    {\n"
                       "        outColor = levelsFunc(outColor, levels);\n"
                       "    }\n"
                       "    if (normalizeEnabled)\n"
                       "    {\n"
                       "        outColor = normalizeFunc(outColor, "
                       "normalizeDisplay);\n"
                       "    }\n"
                       "    if (invalidValues)\n"
                       "    {\n"
                       "        if (outColor.r < 0.0 || outColor.r > 1.0 ||\n"
                       "            outColor.g < 0.0 || outColor.g > 1.0 ||\n"
                       "            outColor.b < 0.0 || outColor.b > 1.0 ||\n"
                       "            outColor.a < 0.0 || outColor.a > 1.0)\n"
                       "        {\n"
                       "           outColor.r = 1.0f;\n"
                       "           outColor.g *= 0.5f;\n"
                       "           outColor.b *= 0.5f;\n"
                       "        }\n"
                       "    }\n"
                       "    // Swizzle for the channels display.\n"
                       "    if (Channels_Red == channels)\n"
                       "    {\n"
                       "        outColor.g = outColor.r;\n"
                       "        outColor.b = outColor.r;\n"
                       "    }\n"
                       "    else if (Channels_Green == channels)\n"
                       "    {\n"
                       "        outColor.r = outColor.g;\n"
                       "        outColor.b = outColor.g;\n"
                       "    }\n"
                       "    else if (Channels_Blue == channels)\n"
                       "    {\n"
                       "        outColor.r = outColor.b;\n"
                       "        outColor.g = outColor.b;\n"
                       "    }\n"
                       "    else if (Channels_Alpha == channels)\n"
                       "    {\n"
                       "        outColor.r = outColor.a;\n"
                       "        outColor.g = outColor.a;\n"
                       "        outColor.b = outColor.a;\n"
                       "    }\n"
                       "    else if (Channels_Lumma == channels)\n"
                       "    {\n"
                       "        float t = (outColor.r + outColor.g + "
                       "outColor.b) / 3.0;\n"
                       "        outColor.r = t;\n"
                       "        outColor.g = t;\n"
                       "        outColor.b = t;\n"
                       "    }\n"
                       "}\n")
                .arg(args[0])
                .arg(args[1])
                .arg(args[2])
                .arg(args[3])
                .arg(args[4])
                .arg(args[5])
                .arg(args[6])
                .arg(args[7])
                .arg(args[8]);
        }

        std::string differenceFragmentSource()
        {
            return "#version 410\n"
                   "\n"
                   "in vec2 fTexture;\n"
                   "out vec4 outColor;\n"
                   "\n"
                   "uniform sampler2D textureSampler;\n"
                   "uniform sampler2D textureSamplerB;\n"
                   "\n"
                   "void main()\n"
                   "{\n"
                   "    vec4 c = texture(textureSampler, fTexture);\n"
                   "    vec4 cB = texture(textureSamplerB, fTexture);\n"
                   "    outColor.r = abs(c.r - cB.r);\n"
                   "    outColor.g = abs(c.g - cB.g);\n"
                   "    outColor.b = abs(c.b - cB.b);\n"
                   "    outColor.a = max(c.a, cB.a);\n"
                   "}\n";
        }
    } // namespace timeline_gl
} // namespace tl

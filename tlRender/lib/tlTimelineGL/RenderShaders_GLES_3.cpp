// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/GL/RenderPrivate.h>

#include <ftk/Core/Format.h>

namespace tl
{
    namespace gl
    {
        std::string vertexSource()
        {
            return
                "#version 300 es\n"
                "precision mediump float;\n"
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
            return
                "#version 300 es\n"
                "precision mediump float;\n"
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

        std::string textureFragmentSource()
        {
            return
                "#version 300 es\n"
                "precision mediump float;\n"
                "out vec4 outColor;\n"
                "\n"
                "in vec2 fTexture;\n"
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
            const std::string videoLevels =
                "// enum tl::image::VideoLevels\n"
                "const int VideoLevels_FullRange  = 0;\n"
                "const int VideoLevels_LegalRange = 1;\n";
        }

        std::string toLinearFragmentSource(
            const std::string&,
            const std::string&)
        {
            // No color management on GLES 2.
            return std::string();
        }

        std::string displayFragmentSource(
            const std::string& toLinearDef,
            const std::string& toLinear,
            const std::string& colorConfigDef,
            const std::string& colorConfig,
            const std::string& lutDef,
            const std::string& lut,
            LUTOrder lutOrder)
        {
            // What runs before the color corrections and what runs after:
            // the transform to linear comes before so the corrections
            // operate on linear values, the display transform after, and
            // the LUT is on whichever side of the color management it was
            // asked for.
            std::string before;
            std::string after;
            switch (lutOrder)
            {
            case LUTOrder::PreConfig:
                before = lut + "\n    " + toLinear;
                after = colorConfig;
                break;
            case LUTOrder::PostConfig:
                before = toLinear;
                after = colorConfig + "\n    " + lut;
                break;
            default: break;
            }
            return ftk::Format(
                "#version 300 es\n"
                "precision mediump float;\n"
                "out vec4 outColor;\n"
                "\n"
                "in vec2 fTexture;\n"
                "\n"
                "// enum tl::ftk::ChannelDisplay\n"
                "const int Channels_Color = 0;\n"
                "const int Channels_Red   = 1;\n"
                "const int Channels_Green = 2;\n"
                "const int Channels_Blue  = 3;\n"
                "const int Channels_Alpha = 4;\n"
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
                "{0}\n"
                "\n"
                "{1}\n"
                "\n"
                "{2}\n"
                "\n"
                "{3}\n"
                "\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "uniform int      channels;\n"
                "uniform bool     negative;\n"
                "uniform int      mirrorX;\n"
                "uniform int      mirrorY;\n"
                "uniform bool     colorEnabled;\n"
                "uniform vec3     colorAdd;\n"
                "uniform mat4     colorMatrix;\n"
                "uniform bool     levelsEnabled;\n"
                "uniform Levels   levels;\n"
                "uniform bool     exposureEnabled;\n"
                "uniform float    exposure;\n"
                "uniform float    softClip;\n"
                "uniform int      videoLevels;\n"
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
                "        value[0] = tmp + (1.0 - exp(-(value[0] - tmp) / softClip)) * softClip;\n"
                "    if (value[1] > tmp)\n"
                "        value[1] = tmp + (1.0 - exp(-(value[1] - tmp) / softClip)) * softClip;\n"
                "    if (value[2] > tmp)\n"
                "        value[2] = tmp + (1.0 - exp(-(value[2] - tmp) / softClip)) * softClip;\n"
                "    return value;\n"
                "}\n"
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
                "    if (negative)\n"
                "    {\n"
                "        outColor.r = 1.0 - outColor.r;\n"
                "        outColor.g = 1.0 - outColor.g;\n"
                "        outColor.b = 1.0 - outColor.b;\n"
                "    }\n"
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
                "    // To linear, so the color transformations operate on\n"
                "    // linear values.\n"
                "    {4}\n"
                "\n"
                "    // Apply color transformations.\n"
                "    if (colorEnabled)\n"
                "    {\n"
                "        outColor = colorFunc(outColor, colorAdd, colorMatrix);\n"
                "    }\n"
                "    if (levelsEnabled)\n"
                "    {\n"
                "        outColor = levelsFunc(outColor, levels);\n"
                "    }\n"
                "    if (exposureEnabled)\n"
                "    {\n"
                "        outColor.rgb *= exposure;\n"
                "    }\n"
                "    if (softClip > 0.0)\n"
                "    {\n"
                "        outColor = softClipFunc(outColor, softClip);\n"
                "    }\n"
                "\n"
                "    // Apply color management.\n"
                "    {5}\n"
                "\n"
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
                "}\n").
                arg(videoLevels).
                arg(toLinearDef).
                arg(colorConfigDef).
                arg(lutDef).
                arg(before).
                arg(after);
        }

        std::string dissolveFragmentSource()
        {
            return
                "#version 300 es\n"
                "precision mediump float;\n"
                "out vec4 outColor;\n"
                "\n"
                "in vec2 fTexture;\n"
                "\n"
                "uniform float     dissolve;\n"
                "uniform sampler2D textureSampler;\n"
                "uniform sampler2D textureSampler2;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    vec4 c = texture(textureSampler, fTexture);\n"
                "    vec4 c2 = texture(textureSampler2, fTexture);\n"
                "    float idissolve = 1.0 - dissolve;\n"
                "    outColor.r = c.r * idissolve + c2.r * dissolve;\n"
                "    outColor.g = c.g * idissolve + c2.g * dissolve;\n"
                "    outColor.b = c.b * idissolve + c2.b * dissolve;\n"
                "    outColor.a = c.a * idissolve + c2.a * dissolve;\n"
                "}\n";
        }

        std::string clippingWarningFragmentSource()
        {
            return
                "#version 300 es\n"
                "precision mediump float;\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 outColor;\n"
                "\n"
                "uniform int       mode;\n"
                "uniform float     low;\n"
                "uniform float     high;\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 c = texture(textureSampler, fTexture);\n"
                // What is compared with the top and the bottom of the range:
                // any channel is the largest above and the smallest below,
                // all channels the other way around, and the luminance both.
                "    float top = max(c.r, max(c.g, c.b));\n"
                "    float bottom = min(c.r, min(c.g, c.b));\n"
                "    if (1 == mode)\n"
                "    {\n"
                "        float t = top;\n"
                "        top = bottom;\n"
                "        bottom = t;\n"
                "    }\n"
                "    else if (2 == mode)\n"
                "    {\n"
                "        top = dot(c.rgb, vec3(0.2126, 0.7152, 0.0722));\n"
                "        bottom = top;\n"
                "    }\n"
                "    if (top > high)\n"
                "    {\n"
                "        outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
                "    }\n"
                "    else if (bottom < low)\n"
                "    {\n"
                "        outColor = vec4(1.0, 0.0, 1.0, 1.0);\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        outColor = vec4(0.0);\n"
                "    }\n"
                "}\n";
        }

        std::string butterflyFragmentSource()
        {
            return
                "#version 300 es\n"
                "precision mediump float;\n"
                "out vec4 outColor;\n"
                "\n"
                "in vec2 fTexture;\n"
                "\n"
                "uniform sampler2D textureSampler;\n"
                "uniform sampler2D textureSamplerB;\n"
                "\n"
                "void main()\n"
                "{\n"
                // The same half of both, the second one mirrored, so that
                // the middle of the picture is on both sides of the seam.
                "    if (fTexture.x < .5)\n"
                "    {\n"
                "        outColor = texture(textureSampler, fTexture);\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        outColor = texture(\n"
                "            textureSamplerB,\n"
                "            vec2(1.0 - fTexture.x, fTexture.y));\n"
                "    }\n"
                "}\n";
        }

        std::string differenceFragmentSource()
        {
            return
                "#version 300 es\n"
                "precision mediump float;\n"
                "out vec4 outColor;\n"
                "\n"
                "in vec2 fTexture;\n"
                "\n"
                "uniform sampler2D textureSampler;\n"
                "uniform sampler2D textureSamplerB;\n"
                "uniform float     gain;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    vec4 c = texture(textureSampler, fTexture);\n"
                "    vec4 cB = texture(textureSamplerB, fTexture);\n"
                "    outColor.r = abs(c.r - cB.r) * gain;\n"
                "    outColor.g = abs(c.g - cB.g) * gain;\n"
                "    outColor.b = abs(c.b - cB.b) * gain;\n"
                "    outColor.a = max(c.a, cB.a);\n"
                "}\n";
        }
    }
}


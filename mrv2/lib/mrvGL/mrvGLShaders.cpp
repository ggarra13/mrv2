// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/StringFormat.h>

#include "mrvGLShaders.h"

namespace mrv
{
    namespace
    {
        const std::string swizzleSource =
            "// enum tl::render::Channels\n"
            "const uint Channels_Color = 0;\n"
            "const uint Channels_Red   = 1;\n"
            "const uint Channels_Green = 2;\n"
            "const uint Channels_Blue  = 3;\n"
            "const uint Channels_Alpha = 4;\n"
            "\n"
            "uniform int   channels;\n"
            "\n"
            "vec4 swizzleFunc(vec4 c, int channels)\n"
            "{\n"
            "    // Swizzle for the channels display.\n"
            "    if (Channels_Red == channels)\n"
            "    {\n"
            "        c.g = c.r;\n"
            "        c.b = c.r;\n"
            "    }\n"
            "    else if (Channels_Green == channels)\n"
            "    {\n"
            "        c.r = c.g;\n"
            "        c.b = c.g;\n"
            "    }\n"
            "    else if (Channels_Blue == channels)\n"
            "    {\n"
            "        c.r = c.b;\n"
            "        c.g = c.b;\n"
            "    }\n"
            "    else if (Channels_Alpha == channels)\n"
            "    {\n"
            "        c.r = c.a;\n"
            "        c.g = c.a;\n"
            "        c.b = c.a;\n"
            "    }\n"
            " return c;\n"
            "}\n";
    }

    std::string textureFragmentSource()
    {
        return "#version 410\n"
               "\n"
               "in vec2 fTexture;\n"
               "out vec4 fColor;\n"
               "\n"
               "uniform sampler2D textureSampler;\n"
               "\n"
               "void main()\n"
               "{\n"
               "    fColor = texture(textureSampler, fTexture);\n"
               "}\n";
    }

    std::string annotationFragmentSource()
    {
        return tl::string::Format(
                   "#version 410\n"
                   "\n"
                   "in vec2 fTexture;\n"
                   "out vec4 fColor;\n"
                   "\n"
                   "{0}\n"
                   "\n"
                   "uniform sampler2D textureSampler;\n"
                   "uniform float alphamult;\n"
                   "\n"
                   "void main()\n"
                   "{\n"
                   "    fColor = texture(textureSampler, fTexture);\n"
                   "    fColor.a *= alphamult;\n"
                   "    fColor = swizzleFunc(fColor, channels);\n"
                   "}\n")
            .arg(swizzleSource);
    }

    std::string softFragmentSource()
    {
        return "#version 410\n"
               "\n"
               "in vec2 fTexture;\n"
               "out vec4 fColor;\n"
               "\n"
               "uniform vec4  color;\n"
               "\n"
               "void main()\n"
               "{\n"
               "    vec2       v = fTexture - vec2(0.5, 0.5);\n"
               "    float ratio  = 1- sqrt(v.x * v.x + v.y * v.y);\n"
               "    float radius = 0.75;\n"
               "    float feather = 0.25;\n"
               "    float mult = smoothstep(radius - feather, radius + "
               "feather, ratio);\n"
               "    fColor = color;\n"
               "    fColor.a *= mult;\n"
               "}\n";
    }

    std::string hardFragmentSource()
    {
        return "#version 410\n"
               "\n"
               "in vec2 fTexture;\n"
               "out vec4 fColor;\n"
               "\n"
               "uniform vec4  color;\n"
               "\n"
               "void main()\n"
               "{\n"
               "    fColor = color;\n"
               "}\n";
    }

} // namespace mrv

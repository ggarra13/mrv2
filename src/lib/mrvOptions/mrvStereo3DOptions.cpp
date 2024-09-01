// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include "mrvOptions/mrvStereo3DOptions.h"

namespace mrv
{
    using namespace tl;

    TLRENDER_ENUM_IMPL(Stereo3DInput, "None", "Image");
    TLRENDER_ENUM_SERIALIZE_IMPL(Stereo3DInput);

    TLRENDER_ENUM_IMPL(
        Stereo3DOutput, "Anaglyph", "Scanlines", "Columns", "Checkerboard",
        "OpenGL");
    TLRENDER_ENUM_SERIALIZE_IMPL(Stereo3DOutput);

    void to_json(nlohmann::json& j, const Stereo3DOptions& value)
    {
        j["input"] = value.input;
        j["output"] = value.output;
        j["eyeSeparation"] = value.eyeSeparation;
        j["swapEyes"] = value.swapEyes;
    }

    void from_json(const nlohmann::json& j, Stereo3DOptions& value)
    {
        if (j["input"].type() == nlohmann::json::value_t::string)
        {
            j.at("input").get_to(value.input);
        }
        else
        {
            int v;
            j.at("input").get_to(v);
            value.input = static_cast<Stereo3DInput>(v);
        }
        if (j["output"].type() == nlohmann::json::value_t::string)
        {
            j.at("output").get_to(value.output);
        }
        else
        {
            int v;
            j.at("output").get_to(v);
            value.output = static_cast<Stereo3DOutput>(v);
        }
        j.at("eyeSeparation").get_to(value.eyeSeparation);
        j.at("swapEyes").get_to(value.swapEyes);
    }
} // namespace mrv

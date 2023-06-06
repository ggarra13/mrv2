// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include "mrvCore/mrvStereo3DOptions.h"

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
        j.at("input").get_to(value.input);
        j.at("output").get_to(value.output);
        j.at("eyeSeparation").get_to(value.eyeSeparation);
        j.at("swapEyes").get_to(value.swapEyes);
    }
} // namespace mrv

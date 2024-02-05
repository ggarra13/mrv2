// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <tlCore/Util.h>

namespace mrv
{

    enum class Stereo3DInput {
        None,
        Image,

        Count,
        First = None
    };
    TLRENDER_ENUM(Stereo3DInput);
    TLRENDER_ENUM_SERIALIZE(Stereo3DInput);

    enum class Stereo3DOutput {
        Anaglyph,
        Scanlines,
        Columns,
        Checkerboard,
        OpenGL,

        Count,
        First = Anaglyph
    };
    TLRENDER_ENUM(Stereo3DOutput);
    TLRENDER_ENUM_SERIALIZE(Stereo3DOutput);

    /**
     * Struct holding the definitions for Environment Map Options.
     *
     */
    struct Stereo3DOptions
    {

        Stereo3DInput input = Stereo3DInput::None;
        Stereo3DOutput output = Stereo3DOutput::Anaglyph;
        float eyeSeparation = 0.F;
        bool swapEyes = false;

        bool operator==(const Stereo3DOptions& b) const;
        bool operator!=(const Stereo3DOptions& b) const;
    };

    void to_json(nlohmann::json& j, const Stereo3DOptions& value);

    void from_json(const nlohmann::json& j, Stereo3DOptions& value);

} // namespace mrv

#include "mrvOptions/mrvStereo3DOptionsInline.h"

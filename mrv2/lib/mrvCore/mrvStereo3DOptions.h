// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

namespace mrv
{
    /**
     * Struct holding the definitions for Environment Map Options.
     *
     */
    struct Stereo3DOptions
    {
        enum class Input { None, Image, Vertical, Horizontal };

        enum class Output {
            None,
            Anaglyph,
            Cubic,
        };

        Input input = Input::None;
        Output output = Output::None;
        float eyeSeparation = 0.01F;

        bool operator==(const Stereo3DOptions& b) const;
        bool operator!=(const Stereo3DOptions& b) const;
    };

    void to_json(nlohmann::json& j, const Stereo3DOptions& value);

    void from_json(const nlohmann::json& j, Stereo3DOptions& value);

} // namespace mrv

#include "mrvCore/mrvStereo3DOptionsInline.h"

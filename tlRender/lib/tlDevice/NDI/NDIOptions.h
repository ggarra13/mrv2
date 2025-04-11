// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2023-2024 Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <tlCore/Time.h>

#include <string>

namespace tl
{
    namespace ndi
    {
        struct Options
        {
            std::string sourceName;
            bool noAudio = false;
            bool bestFormat = true;

            // These are used internally by NDIRead
            size_t requestTimeout = 5;
            size_t videoBufferSize = 4;
            otime::RationalTime audioBufferSize = otime::RationalTime(2.0, 1.0);
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Options&);

        void from_json(const nlohmann::json&, Options&);

        ///@}
    } // namespace ndi
} // namespace tl

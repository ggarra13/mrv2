// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace tl
{
    namespace timeline
    {
        //! OpenColorIO options.
        struct OCIOOptions
        {
            bool enabled = false;
            std::string fileName;
            std::string input;
            std::string display;
            std::string view;
            std::string look;

            bool operator==(const OCIOOptions&) const;
            bool operator!=(const OCIOOptions&) const;
        };

        void to_json(nlohmann::json&, const OCIOOptions&);

        void from_json(const nlohmann::json&, OCIOOptions&);
    } // namespace timeline
} // namespace tl

#include <tlTimeline/OCIOOptionsInline.h>

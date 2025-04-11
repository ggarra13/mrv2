// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace tl
{
    namespace timeline
    {
        //! Transitions.
        enum class Transition {
            None,
            Dissolve,

            Count,
            First = None
        };
        TLRENDER_ENUM(Transition);
        TLRENDER_ENUM_SERIALIZE(Transition);

        //! Convert to a transition.
        Transition toTransition(const std::string&);
    } // namespace timeline
} // namespace tl

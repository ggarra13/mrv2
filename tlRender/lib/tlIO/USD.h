// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Plugin.h>

namespace tl
{
    //! USD Base.
    namespace usd
    {
        //! USD draw modes.
        enum class DrawMode {
            Points,
            Wireframe,
            WireframeOnSurface,
            ShadedFlat,
            ShadedSmooth,
            GeomOnly,
            GeomFlat,
            GeomSmooth,

            Count,
            First = Points
        };
        TLRENDER_ENUM(DrawMode);
        TLRENDER_ENUM_SERIALIZE(DrawMode);
    } // namespace usd
} // namespace tl

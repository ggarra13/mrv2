// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/USDPrivate.h>

#include <tlCore/Error.h>

namespace tl
{
    namespace usd
    {
        TLRENDER_ENUM_IMPL(
            DrawMode,
            "Points",
            "Wireframe",
            "WireframeOnSurface",
            "ShadedFlat",
            "ShadedSmooth",
            "GeomOnly",
            "GeomFlat",
            "GeomSmooth");
        TLRENDER_ENUM_SERIALIZE_IMPL(DrawMode);
    } // namespace usd
} // namespace tl

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <stddef.h>
#include <algorithm>

#include <tlCore/Math.h>
#include <tlCore/Mesh.h>

namespace mrv
{
    using namespace tl;

    //! Create an environment cube triangle mesh.
    geom::TriangleMesh3 createEnvCube(const float size = 1.0F, const bool flipped = false);
} // namespace mrv

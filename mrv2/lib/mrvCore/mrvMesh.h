// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <stddef.h>
#include <tlCore/Math.h>
#include <tlCore/Mesh.h>

#include <algorithm>

namespace mrv {
using namespace tl;

//! Create a an environment cube triangle mesh.
geom::TriangleMesh3 createEnvCube(float size = 1.0F);
} // namespace mrv

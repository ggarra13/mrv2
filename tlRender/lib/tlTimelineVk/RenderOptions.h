// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025-Present Gonzalo Garramuño.
// All rights reserved.

#pragma once

//
// when 1, it uses hard-coded constants instead of
// dynamic push_constants in libplacebo.
//
#define USE_CONSTANTS 0

//
// Use 1 for faster startups.
//
#define USE_PRECOMPILED_SHADERS 1

//
// Use 1 for using dynamic stencils.
//
#define USE_DYNAMIC_STENCILS 0          

//
// Use 1 for using dynamic RGBA write masks.
// MoltenVK does not currently support dynamic write masks.
//
#define USE_DYNAMIC_RGBA_WRITE_MASKS 0

#include <tlTimelineVk/RenderShadersBinary.h>

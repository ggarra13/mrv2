// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025-Present Gonzalo Garramuño.
// All rights reserved.

#pragma once

//
// when 1, it uses hard-coded constants instead of
// dynamic push_constants in libplacebo.
//
#define USE_CONSTANTS 0

// When 1, these will output when a pipeline and layout is created.
#define DEBUG_PIPELINE_USE 0
#define DEBUG_PIPELINE_LAYOUT_USE 0

// Use 1 for using the dummy shader for debugging.
#define USE_DUMMY_SHADER 0

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

//
// Use 1 to output display shader code.
//
#define DEBUG_DISPLAY_SHADER 0

//
// Use 1 to output display descriptor sets for "display" shader.
//
#define DEBUG_DISPLAY_DESCRIPTOR_SETS 0

//
// Use 1 to use Vulkan shader code.
//
#define USE_OCIO_VULKAN 0


#include <tlTimelineVk/RenderShadersBinary.h>

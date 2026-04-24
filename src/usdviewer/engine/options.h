// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2025-Present Gonzalo Garramuño.
// All rights reserved.

#pragma once

// When 1, these will output when a pipeline and layout is created.
#define DEBUG_PIPELINE_USE 0
#define DEBUG_PIPELINE_LAYOUT_USE 0

// Use 1 for using the dummy shader for debugging.
#define USE_DUMMY_SHADER 1
#define USE_ST_SHADER 1

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
#define DEBUG_TEXTURES  0
#define DEBUG_MATERIALS 1
#define DEBUG_BAKE_JOINTS 0


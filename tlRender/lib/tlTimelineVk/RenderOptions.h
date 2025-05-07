#pragma once

#define USE_CONSTANTS 0  // when 1, it uses hard-coded constants instead of
                         // dynamic push_constants
#define USE_PRECOMPILED_SHADERS 0 // Use 1 for faster startups.

#if USE_PRECOMPILED_SHADERS
#   include <tlTimelineVk/RenderShadersBinary.h>
#endif

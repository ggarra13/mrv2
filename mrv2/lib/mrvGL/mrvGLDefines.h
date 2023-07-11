// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

// Set it draw selection rectangle with GL_LINE_LOOP instead of a mesh
#define USE_ONE_PIXEL_LINES 1

// Set it to draw text shapes with opengl2 instead of
// tlRender's OpenGL 3 freetype routines
#define USE_OPENGL2 1

// Set it to debug OpenGL issues with the CHECK_GL macro.
// #define USE_GL_CHECKS 1

// On debug builds, we always use GL checks.
#ifndef NDEBUG
#    define USE_GL_CHECKS 1
#endif

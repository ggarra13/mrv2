// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

// Set it draw selection rectangle with GL_LINE_STRIP
#define USE_ONE_PIXEL_LINES 1

// Set it to draw text shapes with opengl2 instead of
// tlRender's OpenGL 3 freetype routines
#define USE_OPENGL2 1

#define USE_GL_DRAW_COLOR_MESH 1

// macOS seems to have a problem with drawColorMesh resulting in meshes
// not being colored.
#ifdef __APPLE__
#    undef USE_GL_DRAW_COLOR_MESH
#endif

// Set it to debug OpenGL issues with the CHECK_GL macro.
// #define USE_GL_CHECKS 1

// On debug builds, we always use GL checks.
#ifndef NDEBUG
#    undef USE_GL_CHECKS
#    define USE_GL_CHECKS 1
#endif

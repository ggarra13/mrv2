#pragma once


// Set it draw selection rectangle with GL_LINE_LOOP instead of a mesh
#define USE_ONE_PIXEL_LINES 1

// Set it to draw text shapes with opengl2 instead of
// tlRender's OpenGL 3 freetype routines
#define USE_OPENGL2

// If we are drawing with opengl, force to always draw with it
#ifdef USE_OPENGL2
#  define ALWAYS_DRAW_WITH_GL2
#endif

#ifdef __APPLE__
#  define NO_GL_WIDGETS
#endif




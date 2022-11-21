#pragma once


// Set it draw selection rectangle with GL_LINE_LOOP instead of a mesh
#define USE_ONE_PIXEL_LINES 1

// Set it to draw text shapes with opengl2 instead of
// tlRender's OpenGL 3 freetype routines
#define USE_OPENGL2

// There seems to be a bug with the opengl3 implementation of the marix
//#define ALWAYS_DRAW_WITH_GL2

// If we are drawing with opengl, force to always draw with it
#ifdef USE_OPENGL2
#  define ALWAYS_DRAW_WITH_GL2
#endif

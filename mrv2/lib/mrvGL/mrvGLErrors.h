// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <glad/gl.h>

#include "mrvGL/mrvGLDefines.h"

#ifdef USE_GL_CHECKS
#    define CHECK_GL glPrintError(__FILE__, __LINE__);
#else
#    define CHECK_GL
#endif

#ifdef TLRENDER_API_GL_4_1_Debug
void glDebugOutput(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam);
#endif

inline void glPrintError(const char* file, const unsigned line)
{
    GLenum error = glGetError();
    if (error == GL_NO_ERROR)
        return;

    std::cerr << "GL Error: ";
    switch (error)
    {
    case GL_INVALID_ENUM:
        std::cerr << "INVALID_ENUM";
        break;
    case GL_INVALID_VALUE:
        std::cerr << "INVALID_VALUE";
        break;
    case GL_INVALID_OPERATION:
        std::cerr << "INVALID_OPERATION";
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        std::cerr << "INVALID_FRAMEBUFFER_OPERATION";
        break;
    case GL_OUT_OF_MEMORY:
        std::cerr << "OUT_OF_MEMORY";
        break;
    default:
        std::cerr << "UNKNOWN";
        break;
    }

    std::cerr << " (" << error << ") at " << file << " (" << line << ")"
              << std::endl;
}

// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvGL/mrvGLErrors.h"

void glDebugOutput(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam)
{
#ifdef TLRENDER_API_GL_4_1_Debug
    //! \todo Send output to the log instead of cerr?
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cerr << "GL HIGH: " << message << std::endl;
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cerr << "GL MEDIUM: " << message << std::endl;
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cerr << "GL LOW: " << message << std::endl;
        break;
    // case GL_DEBUG_SEVERITY_NOTIFICATION:
    //     std::cerr << "GL NOTIFICATION: " << message << std::endl;
    //     break;
    default:
        break;
    }
#endif
}

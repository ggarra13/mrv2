// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <memory>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlTimeline/GLRender.h>
#include <tlGL/Shader.h>

#include "mrvGL/mrvGLDefines.h"
#include "mrvGL/mrvGLErrors.h"
#include "mrvGL/mrvGLLines.h"
#include "mrvGL/mrvGLViewport.h"
#include "mrvGL/mrvGLOutline.h"

namespace mrv
{
    struct Viewport::GLPrivate
    {
        std::weak_ptr<system::Context> context;

        // GL variables
        //! OpenGL Offscreen buffers
        std::shared_ptr<tl::gl::OffscreenBuffer> buffer = nullptr;
        std::shared_ptr<tl::gl::OffscreenBuffer> stereoBuffer = nullptr;
        std::shared_ptr<tl::gl::OffscreenBuffer> annotation = nullptr;
        std::shared_ptr<timeline::GLRender> render = nullptr;
        std::shared_ptr<gl::Shader> shader = nullptr;
        std::shared_ptr<gl::Shader> annotationShader = nullptr;
        std::shared_ptr<gl::Shader> stereoShader = nullptr;
        int index = 0;
        int nextIndex = 1;
        GLuint pboIds[2];
        std::shared_ptr<gl::VBO> vbo;
        std::shared_ptr<gl::VAO> vao;

#ifdef USE_ONE_PIXEL_LINES
        std::shared_ptr<tl::gl::Outline> outline;
#endif
        std::shared_ptr<tl::gl::Lines> lines;

#ifdef TLRENDER_API_GL_4_1_Debug
        bool init_debug = false;
#endif
    };

//! Define a variable, "gl", that references the private implementation.
#define MRV2_GL() auto& gl = *_gl

} // namespace mrv

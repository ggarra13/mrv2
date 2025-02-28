// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <memory>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlTimelineGL/Render.h>
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
        image::PixelType colorBufferType = image::PixelType::RGBA_F32;

        std::shared_ptr<timeline_gl::Render> render;
        std::shared_ptr<tl::gl::OffscreenBuffer> buffer;
        std::shared_ptr<tl::gl::OffscreenBuffer> stereoBuffer;
        std::shared_ptr<tl::gl::OffscreenBuffer> annotation;
        std::shared_ptr<tl::gl::OffscreenBuffer> overlay;
        std::shared_ptr<gl::Shader> shader;
        std::shared_ptr<gl::Shader> annotationShader;
        
        int currentPBOIndex = 0;
        int nextPBOIndex = 1;
        GLuint pboIDs[2] = { 0, 0 };
        GLsync pboFences[2] = { 0, 0 };
        GLuint overlayPBO = 0;
        GLsync overlayFence;
        std::shared_ptr<gl::VBO> vbo;
        std::shared_ptr<gl::VAO> vao;

#ifdef USE_ONE_PIXEL_LINES
        std::shared_ptr<opengl::Outline> outline;
#endif
        std::shared_ptr<opengl::Lines> lines;

        bool init_debug = false;
    };

//! Define a variable, "gl", that references the private implementation.
#define MRV2_GL() auto& gl = *_gl

} // namespace mrv

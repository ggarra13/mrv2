// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <memory>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/Util.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    //! OpenGL support
    namespace gl
    {
        //! The graphics APIs. One build has both, and which one a program
        //! uses is decided when it runs: OpenGL where it can be had, and
        //! OpenGL ES where it cannot, or the other way round for a build
        //! that prefers ES.
        enum class API
        {
            GL_4_1,
            GLES_3,

            Count,
            First = GL_4_1
        };
        TLRENDER_ENUM(API);

        //! Get the APIs to try, the preferred one first. The build says
        //! which is preferred (ftk_API), and the FTK_GL_API environment
        //! variable names the only one to try. macOS has OpenGL alone and
        //! the web OpenGL ES alone.
        std::vector<API> getAPIs();

        //! Get the API in use. Until the first context is made that is the
        //! preferred one; gl::System decides when it starts the video, and
        //! every context after is of the same API.
        API getAPI();

        //! Set the API in use. Only before a context has been made.
        void setAPI(API);

        //! Get whether the API in use is OpenGL ES.
        bool isGLES();

        //! Initialize the library.
        void init(const std::shared_ptr<system::Context>&);

        //! Initialize GLAD.
        void initGLAD();
    } // namespace gl
} // namespace tl

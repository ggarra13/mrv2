// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGL/Init.h>

#include <tlGL/GL.h>
#if defined(TLRENDER_GLFW)
#    include <tlGL/GLFWSystem.h>
#endif // TLRENDER_GLFW

#include <tlCore/Context.h>

#if defined(TLRENDER_GLFW)
#    define GLFW_INCLUDE_NONE
#    include <GLFW/glfw3.h>
#endif // TLRENDER_GLFW

namespace tl
{
    namespace gl
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
#if defined(TLRENDER_GLFW)
            if (!context->getSystem<GLFWSystem>())
            {
                context->addSystem(GLFWSystem::create(context));
            }
#endif // TLRENDER_GLFW
        }

        void initGLAD()
        {
#if defined(TLRENDER_API_GL_4_1)
            gladLoaderLoadGL();
#elif defined(TLRENDER_API_GLES_2)
            gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress);
#endif // TLRENDER_API_GL_4_1
        }
    } // namespace gl
} // namespace tl

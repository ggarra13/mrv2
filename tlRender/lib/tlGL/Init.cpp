// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGL/Init.h>

#include <tlGL/GL.h>
#if defined(TLRENDER_GLFW)
#    include <tlGL/GLFWSystem.h>
#endif // TLRENDER_GLFW

#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>

#if defined(TLRENDER_GLFW)
#    define GLFW_INCLUDE_NONE
#    include <GLFW/glfw3.h>
#endif // TLRENDER_GLFW

#include <algorithm>
#include <array>
#include <cstdlib>
#include <stdexcept>
#include <vector>

namespace tl
{
    namespace gl
    {
        TLRENDER_ENUM_IMPL(
            API,
            "GL_4_1",
            "GLES_3");

        namespace
        {
#if !defined(__EMSCRIPTEN__) && !defined(__APPLE__)
            API getPreferredAPI()
            {
#if defined(TLRENDER_API_GLES_3)
                return API::GLES_3;
#else // TLRENDER_API_GLES_3
                return API::GL_4_1;
#endif // TLRENDER_API_GLES_3
            }
#endif // __EMSCRIPTEN__

            API& getAPIRef()
            {
                static API api = getAPIs().front();
                return api;
            }
        }

        std::vector<API> getAPIs()
        {
            std::vector<API> out;
#if defined(__EMSCRIPTEN__)
            out.push_back(API::GLES_3);
#elif defined(__APPLE__)
            out.push_back(API::GL_4_1);
#else // __EMSCRIPTEN__
            if (const char* env = std::getenv("MRV2_GL_API"))
            {
                API api = API::GL_4_1;
                if (from_string(env, api))
                {
                    out.push_back(api);
                }
            }
            if (out.empty())
            {
                const API preferred = getPreferredAPI();
                out.push_back(preferred);
                out.push_back(API::GL_4_1 == preferred ? API::GLES_3 : API::GL_4_1);
            }
#endif // __EMSCRIPTEN__
            return out;
        }

        API getAPI()
        {
            return getAPIRef();
        }

        void setAPI(API value)
        {
            getAPIRef() = value;
        }

        bool isGLES()
        {
            return API::GLES_3 == getAPIRef();
        }

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
#elif defined(TLRENDER_API_GLES_3)
            gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress);
#endif // TLRENDER_API_GL_4_1
        }
    } // namespace gl
} // namespace tl

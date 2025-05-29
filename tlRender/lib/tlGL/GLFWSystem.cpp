// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGL/GLFWSystem.h>

#include <tlGL/GL.h>

#include <tlCore/Context.h>
#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>

namespace tl
{
    namespace gl
    {
        namespace
        {
            void glfwErrorCallback(int, const char* description)
            {
                std::cerr << "GLFW ERROR: " << description << std::endl;
            }
        } // namespace

        bool isWayland()
        {
            bool out = false;
#ifdef __linux__
            char* platform = getenv("FLTK_BACKEND");
            if (!platform)
                platform = getenv("XDG_SESSION_TYPE");
            if (platform && strcmp(platform, "wayland") == 0)
            {
                out = true;
            }
#endif // __linux__
            return out;
        }

        struct GLFWSystem::Private
        {
            bool glfwInit = false;
        };

        void GLFWSystem::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::gl::GLFWSystem", context);
            TLRENDER_P();

            // Initialize GLFW.
            glfwSetErrorCallback(glfwErrorCallback);
            int glfwMajor = 0;
            int glfwMinor = 0;
            int glfwRevision = 0;
            glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
            _log(string::Format("GLFW version: {0}.{1}.{2}")
                     .arg(glfwMajor)
                     .arg(glfwMinor)
                     .arg(glfwRevision));

#ifdef __linux__
            int platform_hint = GLFW_PLATFORM_X11;
            if (isWayland())
            {
                platform_hint = GLFW_PLATFORM_WAYLAND;
            }

            if (glfwPlatformSupported(platform_hint) == GLFW_TRUE)
                glfwInitHint(GLFW_PLATFORM, platform_hint);
#endif
            
            if (!glfwInit())
            {
                //! \todo Only log the error for now so that non-OpenGL
                //! tests can run.
                // throw std::runtime_error("Cannot initialize GLFW");
                auto logSystem = context->getSystem<log::System>();
                logSystem->print(
                    "tl::gl::GLFWSystem", "Cannot initialize GLFW",
                    log::Type::Error);
            }
            p.glfwInit = true;
        }

        GLFWSystem::GLFWSystem() :
            _p(new Private)
        {
        }

        GLFWSystem::~GLFWSystem()
        {
            TLRENDER_P();
            if (p.glfwInit)
            {
                glfwTerminate();
            }
        }

        std::shared_ptr<GLFWSystem>
        GLFWSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<GLFWSystem>(new GLFWSystem);
            out->_init(context);
            return out;
        }
    } // namespace gl
} // namespace tl

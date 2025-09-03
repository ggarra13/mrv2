// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#if defined(MRV2_BACKEND_VK)
#  define VULKAN_BACKEND 1
#  define VULKAN_NAME "Vulkan"
#  define BACKEND mrv::vulkan
#  define BACKEND_NAMESPACE vulkan
#  define SUPER_CLASS VkWindow
#elif defined(MRV2_BACKEND_GL)
#  define OPENGL_BACKEND 1
#  define OPENGL_NAME "OpenGL"
#  define BACKEND mrv::opengl
#  define BACKEND_NAMESPACE opengl
#  define SUPER_CLASS GLWindow
#endif


#if !defined(VULKAN_BACKEND) && !defined(OPENGL_BACKEND)
#   error "Please define one of VULKAN_BACKEND or OPENGL_BACKEND"
#endif

#if defined(VULKAN_BACKEND) && defined(OPENGL_BACKEND)
#   error "Please define either VULKAN_BACKEND or OPENGL_BACKEND, not both."
#endif

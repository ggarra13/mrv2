// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#if defined(MRV2_BACKEND_VK)
#  undef VULKAN_BACKEND
#  define VULKAN_BACKEND 1
#  define VULKAN_NAME "Vulkan"
#  define BACKEND mrv::vulkan
#  define BACKEND_NAMESPACE vulkan
#  define BACKEND_SUPER_CLASS VkWindow
#elif defined(MRV2_BACKEND_GL)
#  undef OPENGL_BACKEND
#  define OPENGL_BACKEND 1
#  define OPENGL_NAME "OpenGL"
#  define BACKEND mrv::opengl
#  define BACKEND_NAMESPACE opengl
#  define BACKEND_SUPER_CLASS GLWindow
#else
#  undef WEBGPU_BACKEND
#  define WEBGPU_BACKEND 1
#  define WEBGPU_NAME "WebGPU"
#  define BACKEND mrv::webgpu
#  define BACKEND_NAMESPACE webgpu
#  define BACKEND_SUPER_CLASS WGPUWindow
#endif


#if !defined(VULKAN_BACKEND) && !defined(OPENGL_BACKEND) && !defined(WEBGPU_BACKEND)
#   error "Please define one of MRV2_BACKEND_VK, MRV2_BACKEND_GL or MRV2_BACKEND_WGPU"
#endif

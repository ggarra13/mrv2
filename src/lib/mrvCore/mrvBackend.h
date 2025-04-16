#pragma once

#if defined(MRV2_BACKEND_GL)
#define OPENGL_BACKEND 1
#elif defined(MRV2_BACKEND_VK)
#define VULKAN_BACKEND 1
#elif defined(MRV2_BACKEND_BOTH)
#define BOTH_BACKEND 1
#else
#error "MRV2_BACKEND not defined properly"
#endif


#if !defined(VULKAN_BACKEND) && !defined(OPENGL_BACKEND)
#   error "Please define one of VULKAN_BACKEND or OPENGL_BACKEND"
#endif

#if defined(VULKAN_BACKEND) && defined(OPENGL_BACKEND)
#   error "Please define either VULKAN_BACKEND or OPENGL_BACKEND, not both."
#endif

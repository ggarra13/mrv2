#pragma once



#define VULKAN_BACKEND
// #define OPENGL_BACKEND 1


#if !defined(VULKAN_BACKEND) && !defined(OPENGL_BACKEND)
#   error "Please define one of VULKAN_BACKEND or OPENGL_BACKEND"
#endif

#if defined(VULKAN_BACKEND) && defined(OPENGL_BACKEND)
#   error "Please define either VULKAN_BACKEND or OPENGL_BACKEND, not both."
#endif

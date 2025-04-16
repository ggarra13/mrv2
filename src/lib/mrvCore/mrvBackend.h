#pragma once


#if 1 //def TLRENDER_GL
#  define OPENGL_BACKEND 1
#else
#  define VULKAN_BACKEND 1
#endif


#if !defined(VULKAN_BACKEND) && !defined(OPENGL_BACKEND)
#   error "Please define one of VULKAN_BACKEND or OPENGL_BACKEND"
#endif

#if defined(VULKAN_BACKEND) && defined(OPENGL_BACKEND)
#   error "Please define either VULKAN_BACKEND or OPENGL_BACKEND, not both."
#endif

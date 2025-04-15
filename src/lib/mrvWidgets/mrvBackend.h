#pragma once


#define VULKAN_BACKEND
// #define OPENGL_BACKEND 1

#include "mrvCore/mrvTimeObject.h"



#if !defined(VULKAN_BACKEND) && !defined(OPENGL_BACKEND)
#   error "Please define one of VULKAN_BACKEND or OPENGL_BACKEND"
#endif

#if defined(VULKAN_BACKEND) && defined(OPENGL_BACKEND)
#   error "Please define either VULKAN_BACKEND or OPENGL_BACKEND, not both."
#endif

#ifdef VULKAN_BACKEND
#    include "mrvVk/mrvVkViewport.h"
#    include "mrvVk/mrvTimelineWidget.h"

#    define MyViewport       mrv::vulkan::Viewport
#    define MyTimelineWidget mrv::vulkan::TimelineWidget
#endif

#ifdef OPENGL_BACKEND
#     include "mrvGL/mrvGLViewport.h"
#     include "mrvGL/mrvTimelineWidget.h"

#     define MyViewport       mrv::opengl::Viewport
#     define MyTimelineWidget mrv::opengl::TimelineWidget
#endif

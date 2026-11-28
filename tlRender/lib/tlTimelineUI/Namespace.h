
#pragma once

#ifdef VULKAN_BACKEND
#    define TIMELINEUI timelineui
#    include <FL/Fl_Vk_Context.H>
#elif defined(OPENGL_BACKEND)
#    define TIMELINEUI timelineui
#elif defined(WEBGPU_BACKEND)
#    include <FL/Fl_WebGPU_Context.H>
#    define TIMELINEUI timelineui
#else
#    error "Unknown backend"
#endif

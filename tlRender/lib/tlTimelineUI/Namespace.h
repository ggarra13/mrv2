
#pragma once

#ifdef VULKAN_BACKEND
#    define TIMELINEUI timelineui_vk
#    include <FL/Fl_Vk_Context.H>
#elif defined(OPENGL_BACKEND)
#    define TIMELINEUI timelineui
#else
#    include <FL/Fl_WebGPU_Context.H>
#    define TIMELINEUI timelineui
#endif

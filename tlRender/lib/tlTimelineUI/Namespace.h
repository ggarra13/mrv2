
#pragma once

#ifdef VULKAN_BACKEND
#    define TIMELINEUI timelineui_vk
#    include <FL/Fl_Vk_Context.H>
#endif

#ifdef OPENGL_BACKEND
#    define TIMELINEUI timelineui
#endif

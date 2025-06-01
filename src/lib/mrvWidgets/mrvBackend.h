// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvCore/mrvBackend.h"
#include "mrvCore/mrvTimeObject.h"



#ifdef VULKAN_BACKEND
#    include "mrvVk/mrvVkViewport.h"
#    include "mrvVk/mrvTimelineWidget.h"

#    define MyViewport       mrv::vulkan::Viewport
#    define MyTimelineWidget mrv::vulkan::TimelineWidget
#endif

#ifdef OPENGL_BACKEND
#    include "mrvGL/mrvGLViewport.h"
#    include "mrvGL/mrvTimelineWidget.h"

#    define MyViewport       mrv::opengl::Viewport
#    define MyTimelineWidget mrv::opengl::TimelineWidget
#endif

#ifdef BOTH_BACKEND
#    include "mrvVk/mrvVkViewport.h"
#    include "mrvGL/mrvTimelineWidget.h"
#
#    define MyViewport       mrv::vulkan::Viewport
#    define MyTimelineWidget mrv::opengl::TimelineWidget
#endif

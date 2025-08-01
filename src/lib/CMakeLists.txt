# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

add_subdirectory(mrvCore)        # tlRender Imath::Imath FLTK cpptrace
add_subdirectory(mrvIcons)
add_subdirectory(mrvOptions)     # mrvCore + USD libraries
add_subdirectory(mrvFlmm)        # FLTK
add_subdirectory(mrvWidgets)     # mrvFlmm mrvCore FLTK FLTK_gl MINIzip
add_subdirectory(mrvFLU)         # mrvWidgets, FLTK, Intl
add_subdirectory(mrvUI)          # mrvWidgets, Xrandr, wayland-client
if(TLRENDER_VK AND MRV2_BACKEND STREQUAL "VK" OR MRV2_BACKEND STREQUAL "BOTH")
    # tlTimelineVk, tlVk, tlTimeline, tlTimelineUIVk, tlUI
    # mrvUI, mrvEdit, mrvWidgets, mrvCore, FLTK
    add_subdirectory(mrvVk)
endif()
if(TLRENDER_GL AND MRV2_BACKEND STREQUAL "GL" OR MRV2_BACKEND STREQUAL "BOTH")
    # tlGL, tlTimeline, tlTimelineUI, tlUI, glfw
    # mrvUI, mrvEdit, mrvWidgets, mrvCore, FLTK_gl
    add_subdirectory(mrvGL)
endif()
if(MRV2_PDF)
    add_subdirectory(mrvPDF)     # mrvWidgets
endif()
add_subdirectory(mrvEdit)    # mrvWidgets, mrvNetwork
add_subdirectory(mrvFl)      # mrvEdit, mrvUI, mrvGL, mrvVk, mrvPDF
add_subdirectory(mrvNetwork) # mrvFl, Poco::Net
add_subdirectory(mrvPanels)  # mrvFl, mrvEdit
add_subdirectory(mrvBaseApp) # tlBaseApp, tlCore, RtAudio, freetype, Imath::Imath
add_subdirectory(mrvApp)  # mrvBaseApp, mrvPanels, mrvFLU, mrvOptions
if(MRV2_PYBIND11)
    add_subdirectory(mrvPy)  # mrvApp
endif()
if(TLRENDER_VK AND TLRENDER_NDI AND MRV2_HDR)
    add_subdirectory(mrvHDRWidgets)  # FLTK FLTK_vk Intl
    add_subdirectory(mrvHDR)         # FLTK FLTK_vk NDI Intl
endif()



set( PO_SOURCES ${PO_SOURCES} PARENT_SCOPE)
set( PO_ABS_SOURCES ${PO_ABS_SOURCES} PARENT_SCOPE)


set( PO_HDR_SOURCES ${PO_HDR_SOURCES} PARENT_SCOPE)
set( PO_HDR_ABS_SOURCES ${PO_HDR_ABS_SOURCES} PARENT_SCOPE)

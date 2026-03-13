# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvDesktop.h
    mrvNDICallbacks.h
    mrvNDIView.h
    mrvMonitor.h
)

set(SOURCES
    mrvDesktop.cpp
    mrvNDICallbacks.cpp
    mrvNDIView.cpp
    mrvMonitor.cpp
)
if(APPLE)
    list(APPEND SOURCES mrvMonitor_macOS.mm)
endif()

find_package(libplacebo REQUIRED)
find_package(NDI REQUIRED)
find_package(tlRender REQUIRED)

set(LIBRARIES 
    libplacebo
    mrvHDRWidgets
    NDI
    tlRender::tlVk
    tlRender::tlCore
    ${FLTK_vk_LIBRARY} ${FLTK_LIBRARIES} 
    ${Intl_LIBRARIES}
)

if(TLRENDER_X11)
    find_library(XRANDR_LIBRARY
	NAMES
	Xrandr)
    list(APPEND LIBRARIES_PRIVATE ${XRANDR_LIBRARY})
endif()

if(TLRENDER_WAYLAND)
    find_library(WAYLAND_CLIENT_LIBRARY
	NAMES
	wayland-client)
    list(APPEND LIBRARIES_PRIVATE ${WAYLAND_CLIENT_LIBRARY})
endif()

if (UNIX AND NOT APPLE)
    find_library(DISPLAY_INFO_LIBRARY
	NAMES
	display-info)
    list(APPEND LIBRARIES_PRIVATE ${DISPLAY_INFO_LIBRARY})

    find_library(DRM_LIBRARY
	NAMES
	drm)
    list(APPEND LIBRARIES_PRIVATE ${DRM_LIBRARY})
endif()

if (WIN32)
    list(APPEND LIBRARIES_PRIVATE dxgi)
endif()


add_library(mrvHDR ${SOURCES} ${HEADERS})
hdr_files_to_absolute_paths()

target_link_libraries(mrvHDR PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvHDR PROPERTIES FOLDER lib)
if (UNIX AND NOT APPLE)
    target_include_directories(mrvHDR PRIVATE /usr/include/libdrm)
endif()


install(TARGETS mrvHDR
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvHDR
    COMPONENT libraries )

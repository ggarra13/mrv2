# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

# Note: mrvMonitor_x11.cpp, mrvMonitor_wayland.cpp, mrvMonitor_macOS.cpp/.h,
#       mrvMonitor_win32.cpp, mrvMonitor_linux.cpp
#       are opened by a #ifdef in mrvMonitor.cpp
set(HEADERS
    mrvAsk.h
    mrvDesktop.h
    mrvMenus.h
    mrvMonitor.h
    mrvUtil.h
)

set(SOURCES
    mrvAsk.cpp
    mrvDesktop.cpp
    mrvMenus.cpp
    mrvMonitor.cpp
    mrvUtil.cpp
)

if(APPLE)
    list(APPEND SOURCES
	mrvMonitor_macOS.mm)
endif()

add_library(mrvUI ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set(LIBRARIES mrvWidgets)
set(LIBRARIES_PRIVATE )

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
    find_library(DISPLAY_INFO
	NAMES
	display-info)
    list(APPEND LIBRARIES_PRIVATE ${DISPLAY_INFO})
endif()

if (WIN32)
    list(APPEND LIBRARIES_PRIVATE dxgi)
endif()

target_link_libraries(mrvUI PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvUI PROPERTIES FOLDER lib)

install(TARGETS mrvUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvUI
    COMPONENT libraries)

# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvAsk.h
    mrvDesktop.h
    mrvMenus.h
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
    list(APPEND SOURCES mrvMonitor_macOS.mm)
endif()

add_library(mrvUI ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set(LIBRARIES mrvWidgets)
set(LIBRARIES_PRIVATE )

if(TLRENDER_X11)
    list(APPEND LIBRARIES_PRIVATE Xrandr)
endif()

if(TLRENDER_WAYLAND)
    list(APPEND LIBRARIES_PRIVATE wayland-client)
endif()

target_link_libraries(mrvUI PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvUI PROPERTIES FOLDER lib)

install(TARGETS mrvUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)

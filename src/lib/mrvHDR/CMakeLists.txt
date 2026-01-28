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
    ${FLTK_vk_LIBRARY} ${FLTK_LIBRARIES} 
    ${Intl_LIBRARIES}
)

if (UNIX AND NOT APPLE)
    find_library(DISPLAY_INFO
	NAMES
	display-info)
    list(APPEND LIBRARIES_PRIVATE ${DISPLAY_INFO})
endif()

add_library(mrvHDR ${SOURCES} ${HEADERS})
hdr_files_to_absolute_paths()

target_link_libraries(mrvHDR PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvHDR PROPERTIES FOLDER lib)


install(TARGETS mrvHDR
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvHDR
    COMPONENT libraries )

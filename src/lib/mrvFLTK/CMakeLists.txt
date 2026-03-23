# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvCallbacks.h
    mrvColorSchemes.h
    mrvFileRequester.h
    mrvSave.h
    mrvWait.h
)

set(SOURCES
    mrvCallbacks.cpp
    mrvColorSchemes.cpp
    mrvFileRequester.cpp
    mrvWait.cpp
)

set(LIBRARIES mrvEdit mrvFLU ${FLTK_LIBRARIES})

if(TLRENDER_GL AND MRV2_BACKEND STREQUAL "GL")
    list(APPEND LIBRARIES mrvGL tlRender::tlTimelineUI )
endif()

if(TLRENDER_VK AND MRV2_BACKEND STREQUAL "VK")
    list(APPEND LIBRARIES mrvVk tlRender::tlTimelineUIVk Vulkan::Vulkan)
endif()

if(MRV2_PYBIND11)
    list(APPEND LIBRARIES pybind11::embed )
    if(UNIX AND NOT APPLE)
	# Needed for python3.10 on Rocky and Ubuntu 20.04
	list( APPEND LIBRARIES "util" )
    endif()
endif()

add_library(mrvFLTK ${SOURCES} ${HEADERS})

files_to_absolute_paths()

if (WIN32)
   target_compile_definitions(mrvFLTK PUBLIC -DFL_DLL)
 endif()
target_link_libraries(mrvFLTK PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvFLTK PROPERTIES FOLDER lib)


install(TARGETS mrvFLTK
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvFLTK
    COMPONENT libraries )

# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvTimelineViewport.h
)

set(SOURCES
    mrvTimelineViewport.cpp
    mrvTimelineViewportEvents.cpp
)
    
add_library(mrvViewport ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set(LIBRARIES mrvWidgets)
if(TLRENDER_VK AND MRV2_BACKEND STREQUAL "VK")
    list(APPEND LIBRARIES ${FLTK_vk_LIBRARY})
endif()

target_link_libraries(mrvViewport PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(mrvViewport PROPERTIES FOLDER lib)

install(TARGETS mrvViewport
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)

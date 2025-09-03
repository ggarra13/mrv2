# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    )

set(SOURCES
    mrvTimelineViewport.cpp
    mrvTimelineViewportEvents.cpp
)
    
add_library(mrvViewport ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvViewport PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(mrvViewport PROPERTIES FOLDER lib)

install(TARGETS mrvViewport
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)

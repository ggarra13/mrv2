# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvEdit.h
)

set(SOURCES
    mrvEdit.cpp
)


add_library(mrvTimeline ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvTimeline PUBLIC ${LIBRARIES} )
set_target_properties(mrvTimeline PROPERTIES FOLDER lib)
set_target_properties(mrvTimeline PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvTimeline
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvTimeline
    COMPONENT libraries)

# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvCreateEDLFromFiles.h
    mrvEditCallbacks.h
    mrvEditMode.h
    mrvEditUtil.h
)

set(SOURCES
    mrvCreateEDLFromFiles.cpp
    mrvEditCallbacks.cpp
    mrvEditMode.cpp
    mrvEditUtil.cpp
)


add_library(mrvEdit ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set(LIBRARIES mrvWidgets)
if(MRV2_NETWORK)
    list(APPEND LIBRARIES mrvNetwork)
endif()
list(APPEND LIBRARIES OTIO)

target_link_libraries(mrvEdit PUBLIC ${LIBRARIES} )
set_target_properties(mrvEdit PROPERTIES FOLDER lib)

install(TARGETS mrvEdit
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)

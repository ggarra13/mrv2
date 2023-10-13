# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvAsk.h
    mrvMenus.h
    mrvUtil.h
)

set(SOURCES
    mrvAsk.cpp
    mrvMenus.cpp
    mrvUtil.cpp
)


add_library(mrvUI ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set(LIBRARIES mrvWidgets)

target_link_libraries(mrvUI PUBLIC ${LIBRARIES} )
set_target_properties(mrvUI PROPERTIES FOLDER lib)

install(TARGETS mrvUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)

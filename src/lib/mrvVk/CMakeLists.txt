# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvVkDefines.h
    # mrvVkErrors.h
    # mrvVkJson.h
    mrvVkLines.h
    mrvVkOutline.h
    # mrvVkShaders.h
    mrvVkShape.h
    # mrvVkTextEdit.h
    # mrvVkUtil.h
    mrvVkViewport.h
    mrvVkViewportPrivate.h
    mrvVkWindow.h
    mrvTimelineViewport.h
    mrvTimelineViewportPrivate.h
    mrvTimelineWidget.h
    )

set(SOURCES
    # mrvVk2TextShape.cpp
    # mrvVkErrors.cpp
    # mrvVkJson.cpp
    mrvVkLines.cpp
    mrvVkOutline.cpp
    # mrvVkShaders.cpp
    mrvVkShape.cpp
    # mrvVkTextEdit.cpp
    # mrvVkUtil.cpp
    mrvVkViewport.cpp
    mrvVkViewportDraw.cpp
    mrvVkViewportPrims.cpp
    mrvVkWindow.cpp
    mrvTimelineViewportEvents.cpp
    mrvTimelineViewport.cpp
    mrvTimelineWidget.cpp
)

if(APPLE)
    list(APPEND SOURCES mrvVkWindow.mm)
endif()

set(LIBRARIES tlVk tlTimeline tlTimelineUI tlUI)
set(LIBRARIES_PRIVATE )
list( APPEND LIBRARIES mrvUI mrvWidgets mrvDraw mrvCore ${FLTK_vk_LIBRARY} )

add_library(mrvVk ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvVk PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(mrvVk PROPERTIES FOLDER lib)

install(TARGETS mrvVk
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)

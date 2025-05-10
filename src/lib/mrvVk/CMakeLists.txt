# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvVkDefines.h
    # mrvVkErrors.h
    mrvVkJson.h
    mrvVkLines.h
    mrvVkShaders.h       # done
    mrvVkShape.h
    mrvVkTextEdit.h      # done
    mrvVkUtil.h
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
    mrvVkJson.cpp
    mrvVkLines.cpp
    mrvVkShaders.cpp        # done
    mrvVkShape.cpp
    mrvVkTextEdit.cpp       # done
    mrvVkUtil.cpp
    mrvVkViewport.cpp
    mrvVkViewportDraw.cpp
    mrvVkViewportPrims.cpp  # done
    mrvVkWindow.cpp
    mrvTimelineViewportEvents.cpp
    mrvTimelineViewport.cpp
    mrvTimelineWidget.cpp
)

if(APPLE)
    list(APPEND SOURCES mrvVkWindow.mm)
endif()

set(LIBRARIES mrvUI mrvWidgets mrvCore ${FLTK_vk_LIBRARY} )
set(LIBRARIES_PRIVATE )
list(APPEND LIBRARIES tlTimelineVk tlVk tlTimeline tlDraw tlTimelineUI tlUI)

add_library(mrvVk ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvVk PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(mrvVk PROPERTIES FOLDER lib)

install(TARGETS mrvVk
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)

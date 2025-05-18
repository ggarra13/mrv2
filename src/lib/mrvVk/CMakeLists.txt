# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvVkDefines.h
    mrvVkJson.h        # done
    mrvVkLines.h        # done
    mrvVkShaders.h       # done
    mrvVkShape.h         # done
    mrvVkTextEdit.h      # done
    mrvVkUtil.h        # done
    mrvVkViewport.h
    mrvVkViewportPrivate.h
    mrvVkWindow.h        # done
    mrvTimelineViewport.h        # done
    mrvTimelineViewportPrivate.h        # done
    mrvTimelineWidget.h
    )

set(SOURCES
    # mrvVk2TextShape.cpp
    mrvVkJson.cpp        # done
    mrvVkLines.cpp        # done
    mrvVkShaders.cpp        # done
    mrvVkShape.cpp        # done
    mrvVkTextEdit.cpp       # done
    mrvVkUtil.cpp        # done
    mrvVkViewport.cpp
    mrvVkViewportDraw.cpp
    mrvVkViewportPrims.cpp  # done
    mrvVkWindow.cpp        # done
    mrvTimelineViewportEvents.cpp        # done
    mrvTimelineViewport.cpp        # done
    mrvTimelineWidget.cpp
)

if(APPLE)
    list(APPEND SOURCES mrvVkWindow.mm)
endif()

set(LIBRARIES mrvUI mrvWidgets mrvCore ${FLTK_vk_LIBRARY} )
set(LIBRARIES_PRIVATE )
list(APPEND LIBRARIES tlRender::tlDraw tlRender::tlVk tlRender::tlTimeline tlRender::tlUI tlRender::tlTimelineUIVk tlRender::tlTimelineVk)

add_library(mrvVk ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvVk PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(mrvVk PROPERTIES FOLDER lib)

install(TARGETS mrvVk
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)

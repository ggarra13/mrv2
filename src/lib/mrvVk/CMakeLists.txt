# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvVkDefines.h
    mrvVkJson.h        
    mrvVkLines.h        
    mrvVkShadersBinary.h       
    mrvVkShaders.h       
    mrvVkShape.h         
    mrvVkTextEdit.h      
    mrvVkUtil.h        
    mrvVkViewport.h
    mrvVkViewportPrivate.h
    mrvVkWindow.h        
    mrvTimelineViewportPrivate.h        
    mrvTimelineWidget.h               
)

set(SOURCES
    mrvVkJson.cpp        
    mrvVkLines.cpp        
    mrvVkShaders.cpp        
    mrvVkShape.cpp        
    mrvVkTextEdit.cpp
    mrvVkTextShape.cpp
    mrvVkUtil.cpp        
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

set(LIBRARIES mrvUI mrvWidgets mrvCore ${FLTK_vk_LIBRARY} )
list(APPEND LIBRARIES tlRender::tlTimelineUIVk tlRender::tlDraw tlRender::tlTimelineVk tlRender::tlVk tlRender::tlUI tlRender::tlTimeline  Vulkan::Vulkan)
set(LIBRARIES_PRIVATE )

add_library(mrvVk ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvVk PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(mrvVk PROPERTIES FOLDER lib)

add_dependencies(mrvVk mrvVoice)

install(TARGETS mrvVk
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvVk
    COMPONENT libraries)

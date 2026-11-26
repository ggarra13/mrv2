# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvWGPUDefines.h
#    mrvWGPUErrors.h
mrvWGPUJson.h
mrvWGPULines.h
#    mrvWGPUShaders.h
#    mrvWGPUShape.h
mrvWGPUTextEdit.h
#    mrvWGPUUtil.h
mrvWGPUViewport.h
mrvWGPUViewportPrivate.h
mrvWGPUWindow.h
mrvTimelineViewportPrivate.h
mrvTimelineWidget.h
)

set(SOURCES
#    mrvWGPU2TextShape.cpp
#    mrvWGPUErrors.cpp
mrvWGPUJson.cpp
mrvWGPULines.cpp
#    mrvWGPUShaders.cpp
mrvWGPUShape.cpp
mrvWGPUTextShape.cpp
mrvWGPUTextEdit.cpp
#    mrvWGPUUtil.cpp

mrvWGPUViewport.cpp # 

#    mrvWGPUViewportDraw.cpp
mrvWGPUViewportPrims.cpp

mrvWGPUWindow.cpp #

#    mrvSaveImage.cpp
#    mrvSaveMovie.cpp


mrvTimelineViewportEvents.cpp #
mrvTimelineViewport.cpp       #
mrvTimelineWidget.cpp         #
)

set(LIBRARIES mrvUI mrvWidgets mrvCore ${FLTK_gl_LIBRARY} )
list(APPEND LIBRARIES
    tlTimelineWGPU
    #tlRender::tlTimelineUI
    tlRender::tlTimeline
    tlWGPU
    tlRender::tlDraw)
set(LIBRARIES_PRIVATE )

add_library(mrvWGPU ${SOURCES} ${HEADERS})

target_link_libraries(mrvWGPU PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(mrvWGPU PROPERTIES FOLDER lib)
target_compile_definitions(mrvWGPU PUBLIC OPENWGPU_BACKEND)

if(TLENDER_FFMPEG)
    add_dependencies(mrvWGPU mrvVoice)
endif()

install(TARGETS mrvWGPU
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvWGPU
    COMPONENT libraries)

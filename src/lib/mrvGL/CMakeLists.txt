# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvGLDefines.h
    mrvGLErrors.h
    mrvGLJson.h
    mrvGLLines.h
    mrvGLShaders.h
    mrvGLShape.h
    mrvGLTextEdit.h
    mrvGLUtil.h
    mrvGLViewport.h
    mrvGLViewportPrivate.h
    mrvGLWindow.h
    mrvTimelineViewportPrivate.h
    mrvTimelineWidget.h
    )

set(SOURCES
    mrvGL2TextShape.cpp
    mrvGLErrors.cpp
    mrvGLJson.cpp
    mrvGLLines.cpp
    mrvGLShaders.cpp
    mrvGLShape.cpp
    mrvGLTextEdit.cpp
    mrvGLUtil.cpp
    mrvGLViewport.cpp
    mrvGLViewportDraw.cpp
    mrvGLViewportPrims.cpp
    mrvGLWindow.cpp
    mrvTimelineViewportEvents.cpp
    mrvTimelineViewport.cpp
    mrvTimelineWidget.cpp
)

if(APPLE)
    list(APPEND SOURCES mrvGLWindow.mm)
endif()
    
add_definitions( -DGL_SILENCE_DEPRECATION )

set(LIBRARIES mrvUI mrvWidgets mrvCore ${FLTK_gl_LIBRARY} )
set(LIBRARIES_PRIVATE )
list(APPEND LIBRARIES tlRender::tlTimelineGL tlRender::tlTimelineUI tlRender::tlTimeline tlRender::tlGL tlRender::tlDraw)

add_library(mrvGL ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvGL PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(mrvGL PROPERTIES FOLDER lib)
target_compile_definitions(mrvGL PUBLIC OPENGL_BACKEND)

if(TLENDER_FFMPEG)
    add_dependencies(mrvGL mrvVoice)
endif()

install(TARGETS mrvGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)

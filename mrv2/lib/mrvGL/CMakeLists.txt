# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvGLDefines.h
    mrvGLErrors.h
    mrvGLJson.h
    mrvGLLines.h
    mrvGLOffscreenContext.h
    mrvGLOutline.h
    mrvGLShaders.h
    mrvGLShape.h
    mrvGLTextEdit.h
    mrvGLUtil.h
    mrvGLViewport.h
    mrvGLViewportPrivate.h
    mrvThumbnailCreator.h
    mrvThumbnailWindow.h
    mrvTimelineViewport.h
    mrvTimelineViewportPrivate.h
    mrvTimelineWidget.h
    )

set(SOURCES
    mrvGL2TextShape.cpp
    mrvGLErrors.cpp
    mrvGLJson.cpp
    mrvGLLines.cpp
    mrvGLOffscreenContext.cpp
    mrvGLOutline.cpp
    mrvGLShaders.cpp
    mrvGLShape.cpp
    mrvGLTextEdit.cpp
    mrvGLUtil.cpp
    mrvGLViewport.cpp
    mrvGLViewportDraw.cpp
    mrvGLViewportPrims.cpp
    mrvThumbnailCreator.cpp
    mrvTimelineViewportEvents.cpp
    mrvTimelineViewport.cpp
    mrvTimelineWidget.cpp
    )

add_definitions( -DGL_SILENCE_DEPRECATION )

set(LIBRARIES tlGL tlTimeline tlTimelineUI tlUI glfw)
set(LIBRARIES_PRIVATE )
list( APPEND LIBRARIES mrvEdit mrvWidgets mrvDraw mrvCore ${FLTK_gl_LIBRARY} )

add_library(mrvGL ${SOURCES} ${HEADERS})
files_to_absolute_paths()

if(WIN32 AND FLTK_BUILD_SHARED_LIBS)
    target_compile_definitions(mrvGL PUBLIC FL_DLL)
endif()
target_link_libraries(mrvGL PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(mrvGL PROPERTIES FOLDER lib)

install(TARGETS mrvGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)

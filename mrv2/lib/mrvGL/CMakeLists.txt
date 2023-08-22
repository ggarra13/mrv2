# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvGLDefines.h
    mrvGLErrors.h
    mrvGLLines.h
    mrvGLOffscreenContext.h
    mrvGLOutline.h
    mrvGLShaders.h
    mrvGLShape.h
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
    mrvGLOffscreenContext.cpp
    mrvGLLines.cpp
    mrvGLOutline.cpp
    mrvGLShaders.cpp
    mrvGLShape.cpp
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

set(LIBRARIES tlGL tlTimeline tlTimelineUI tlUI GLFW)
set(LIBRARIES_PRIVATE )
list( APPEND LIBRARIES mrvEdit mrvWidgets mrvDraw mrvCore fltk_gl )

add_library(mrvGL ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvGL PUBLIC ${LIBRARIES} ${LIBRARIES_PRIVATE})
set_target_properties(mrvGL PROPERTIES FOLDER lib)
set_target_properties(mrvGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvGL
    COMPONENT libraries)

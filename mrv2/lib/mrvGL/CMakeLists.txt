# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvGLDefines.h
    mrvGLErrors.h
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
    )

set(SOURCES
    mrvGLEnvironmentMap.cpp
    mrvGL2TextShape.cpp
    mrvGLOffscreenContext.cpp
    mrvGLOutline.cpp
    mrvGLShaders.cpp
    mrvGLShape.cpp
    mrvGLViewport.cpp
    mrvGLViewportDraw.cpp
    mrvThumbnailCreator.cpp
    mrvTimelineViewportEvents.cpp
    mrvTimelineViewport.cpp
    mrvGLUtil.cpp
    )

add_definitions( -DGL_SILENCE_DEPRECATION )

set(LIBRARIES tlGL tlTimeline)
set(LIBRARIES_PRIVATE )
list( APPEND LIBRARIES mrvWidgets mrvDraw mrvCore ${OpenEXR_Iex_LIBRARY} fltk_gl )

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

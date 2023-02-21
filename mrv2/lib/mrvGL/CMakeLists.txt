# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvGLDefines.h
    mrvGLErrors.h
    mrvGLOffscreenContext.h
    mrvGLOutline.h
    mrvGLShape.h
    mrvGLUtil.h
    mrvGLViewport.h
    mrvThumbnailCreator.h
    mrvThumbnailWindow.h
    mrvTimelineViewport.h
    mrvTimelineViewportPrivate.h
    )

set(SOURCES
    mrvGL2TextShape.cpp
    mrvGLOffscreenContext.cpp
    mrvGLOutline.cpp
    mrvGLShape.cpp
    mrvThumbnailCreator.cpp
    mrvTimelineViewportEvents.cpp
    mrvTimelineViewport.cpp
    mrvGLViewport.cpp
    )

add_definitions( -DGL_SILENCE_DEPRECATION )

set(LIBRARIES tlGL tlTimeline)
set(LIBRARIES_PRIVATE tlGlad)
list( APPEND LIBRARIES mrvWidgets ${OpenEXR_Iex_LIBRARY} fltk_gl )

add_library(mrvGL ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvGL PUBLIC ${LIBRARIES} ${LIBRARIES_PRIVATE})
set_target_properties(mrvGL PROPERTIES FOLDER lib)
set_target_properties(mrvGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvGL
    COMPONENT libraries)

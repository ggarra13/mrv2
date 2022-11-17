set(HEADERS
  mrvGLErrors.h
  mrvGLOutline.h
  mrvGLShape.h
  mrvGLUtil.h
  mrvGLViewport.h
  mrvTimelineViewport.h
  mrvTimelineViewportPrivate.h
  )
  set(SOURCES
    mrvGLOutline.cpp
    mrvGLShape.cpp
    mrvTimelineViewportEvents.cpp
    mrvTimelineViewport.cpp
    mrvGLViewport.cpp
  )

list( APPEND LIBRARIES tlGL tlGlad mrvFl mrvDraw fltk_gl )

add_library(mrvGL ${SOURCES} ${HEADERS})
target_link_libraries(mrvGL PUBLIC ${LIBRARIES})
set_target_properties(mrvGL PROPERTIES FOLDER lib)
set_target_properties(mrvGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvGL)

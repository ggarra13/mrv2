set(HEADERS
  mrvGLErrors.h
  mrvGLShape.h
  mrvGLUtil.h
  mrvGLViewport.h
  mrvTimelineViewportPrivate.h
  mrvTimelineViewport.h
  )
  set(SOURCES
    mrvGLShape.cpp
    mrvGLViewport.cpp
    mrvTimelineViewportEvents.cpp
    mrvTimelineViewport.cpp
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

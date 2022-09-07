set(HEADERS
  mrvGLViewport.h
  mrvTimelineViewportInline.h
  mrvTimelineViewport.h
  )
set(SOURCES
  mrvGLViewport.cpp
  mrvTimelineViewport.cpp
  )

list( APPEND LIBRARIES tlGL tlGlad mrvFl fltk_gl )

add_library(mrvFlGL ${SOURCES} ${HEADERS})
target_link_libraries(mrvFlGL PUBLIC ${LIBRARIES})
set_target_properties(mrvFlGL PROPERTIES FOLDER lib)
set_target_properties(mrvFlGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvFlGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvFl)

set(HEADERS
  mrvGLViewport.h
#  mrvThumbnailCreator.h
  mrvTimelineViewportInline.h
  mrvTimelineViewport.h
  )
set(SOURCES
  mrvGLViewport.cpp
#  mrvThumbnailCreator.cpp
  mrvTimelineViewport.cpp
  )

list( APPEND LIBRARIES tlGL tlGlad mrvFl fltk_gl )

add_library(mrvGL ${SOURCES} ${HEADERS})
target_link_libraries(mrvGL PUBLIC ${LIBRARIES})
set_target_properties(mrvGL PROPERTIES FOLDER lib)
set_target_properties(mrvGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvFl)

set(HEADERS
  mrvGLViewport.h
  mrvTimelineViewportInline.h
  mrvTimelineViewport.h
  )
set(SOURCE
  mrvGLViewport.cpp
  mrvTimelineViewport.cpp
  )

set(LIBRARIES tlTimeline tlGlad mrvFl fltk_gl fltk )
set(LIBRARIES_PRIVATE )

add_library(mrvFlGL ${SOURCE} ${HEADERS})
target_link_libraries(mrvFlGL PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(mrvFlGL PROPERTIES FOLDER lib)
set_target_properties(mrvFlGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvFlGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvFl)

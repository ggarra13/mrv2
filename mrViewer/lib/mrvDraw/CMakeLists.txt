set(HEADERS
  Annotation.h
  LineSegment.h
  Point.h
  Polyline2D.h
  Shape.h
  )

  set(SOURCES
    Annotation.cpp
  )



set(LIBRARIES mrvCore )

add_library(mrvDraw ${SOURCES} ${HEADERS})

target_link_libraries(mrvDraw PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvDraw PROPERTIES FOLDER lib)
set_target_properties(mrvDraw PROPERTIES PUBLIC_HEADER "${HEADERS}")



install(TARGETS mrvDraw
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvDraw)

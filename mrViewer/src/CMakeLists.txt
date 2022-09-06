set(HEADERS
  App.h
  Util.h)
set(SOURCES
  App.cpp
  Util.cpp
  main.cpp)

set(LIBRARIES tlApp mrvFlGL ${LIBINTL_LIBRARIES} ${FLTK_LIBRARIES} )

add_executable(mrViewer ${SOURCES} ${HEADERS})
target_link_libraries(mrViewer PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( mrViewer BEFORE PRIVATE ${CMAKE_INSTALL_PREFIX}/lib )

install(TARGETS mrViewer
  RUNTIME DESTINATION bin)

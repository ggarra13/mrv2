set(HEADERS
  App.h
  Util.h)
set(SOURCES
  App.cpp
  Util.cpp
  main.cpp)

set(LIBRARIES tlApp mrvFlGL )
list( APPEND LIBRARIES ${LIBINTL_LIBRARIES} )

if( APPLE )
   set( OSX_FRAMEWORKS "-framework Cocoa -framework IOKit" )
   list( APPEND LIBRARIES ${OSX_FRAMEWORKS} )
endif()

add_executable(mrViewer ${SOURCES} ${HEADERS})
target_link_libraries(mrViewer PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( mrViewer BEFORE PRIVATE ${CMAKE_INSTALL_PREFIX}/lib )

install(TARGETS mrViewer
  RUNTIME DESTINATION bin)

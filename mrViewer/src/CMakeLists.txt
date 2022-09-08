set(HEADERS )
set(SOURCES main.cpp )

list( APPEND LIBRARIES mrvPlayApp mrvGL ${LIBINTL_LIBRARIES} )


if( APPLE )
   set( OSX_FRAMEWORKS "-framework Cocoa -framework IOKit" )
   list( APPEND LIBRARIES ${OSX_FRAMEWORKS} )
endif()

add_executable(mrViewer ${SOURCES} ${HEADERS})
target_link_libraries(mrViewer PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( mrViewer BEFORE PRIVATE ${CMAKE_INSTALL_PREFIX}/lib )

install(TARGETS mrViewer
  RUNTIME DESTINATION bin)

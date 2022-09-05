if ( FLTK_FOUND )

  include( cmake/functions.cmake )
  include( cmake/find_packages.cmake )

  include_directories(
    ${FLTK_INCLUDE_DIRS}
    .
    ${CMAKE_BINARY_DIR}/examples/mrv2/mrvFl
    )

  set( CMAKE_CXX_STANDARD 14 )
  set( CMAKE_CXX_EXTENSIONS OFF )

  add_definitions( -DUSE_GETTEXT )

  add_subdirectory( mrvCore )
  add_subdirectory( mrvFl )
  add_subdirectory( mrvFlGL )
  #add_subdirectory( mrvFlApp )

  set(HEADERS
    App.h
    Util.h)
  set(SOURCES
      App.cpp
      Util.cpp
      main.cpp)

  set(LIBRARIES_PRIVATE tlApp tlGL mrvFlGL mrvFl mrvCore Boost::filesystem )

  add_executable(mrv2 ${SOURCES} ${HEADERS})
  target_link_libraries(mrv2 PRIVATE ${LIBRARIES_PRIVATE} )
  set_target_properties(mrv2 PROPERTIES FOLDER examples)


endif()

cmake_minimum_required(VERSION 3.19)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)

project(mrv2)


# For macOS, finds Frameworks last.  Uses Find*.cmake first.
set(CMAKE_FIND_FRAMEWORK LAST)


list(PREPEND $ENV{PATH} ${CMAKE_PREFIX_PATH}/bin)


list(PREPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../tlRender/cmake/Modules)


find_package( FLTK     REQUIRED CONFIG )
find_package( Boost    REQUIRED COMPONENTS filesystem )

find_program(FLTK_FLUID_EXECUTABLE
  NAME fluid
  HINTS ${CMAKE_INSTALL_PREFIX}/bin)


set( include_dirs
  ${CMAKE_PREFIX_PATH}/include/tlRender
  ${CMAKE_PREFIX_PATH}/include/OpenEXR
  ${CMAKE_PREFIX_PATH}/include/Imath
  ${CMAKE_PREFIX_PATH}/include/opentimelineio/deps
  ${CMAKE_PREFIX_PATH}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/lib/mrvFl
  ${CMAKE_CURRENT_SOURCE_DIR}/../tlRender/lib
  )
include_directories( ${include_dirs} )


add_definitions( -DUSE_GETTEXT )

find_package( FSeq     REQUIRED )
find_package( OCIO     REQUIRED )
find_package( OTIO     REQUIRED )
find_package( FreeType REQUIRED )
find_package( RtAudio  REQUIRED )
find_package( FFmpeg )
find_package( JPEG )
find_package( PNG  )
find_package( TIFF )
find_package( OpenEXR )

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/lib )


set(HEADERS
  App.h
  Util.h)
set(SOURCES
  App.cpp
  Util.cpp
  main.cpp)

set(LIBRARIES tlApp mrvFlGL )

add_executable(mrv2 ${SOURCES} ${HEADERS})
target_link_libraries(mrv2 PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( mrv2 BEFORE PRIVATE ${CMAKE_INSTALL_PREFIX}/lib )

install(TARGETS mrv2
  RUNTIME DESTINATION bin)

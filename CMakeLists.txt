cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)

project( mrv2 )
#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".so")

include( cmake/options.cmake )
include( cmake/functions.cmake )
include( cmake/find_packages.cmake )

set( include_dirs
  ${CMAKE_PREFIX_PATH}/include
  ${CMAKE_PREFIX_PATH}/include/OpenEXR
  ${CMAKE_PREFIX_PATH}/include/Imath
  ${CMAKE_PREFIX_PATH}/include/opentimelineio/deps
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  ${CMAKE_CURRENT_BINARY_DIR}/mrvFl
  ${FLTK_INCLUDE_DIRS}
  )


include_directories(
  ${include_dirs}
  )

set( lib_dirs
  ${CMAKE_PREFIX_PATH}/lib
  tlRender/build/tlRender/src/tlRender-build/deps/tlGlad/
  )

link_directories( ${lib_dirs} )

list(PREPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules ${PROJECT_SOURCE_DIR}/tlRender/cmake/Modules)

add_definitions( -DUSE_GETTEXT )

add_subdirectory( tlRender/etc/SuperBuild )
add_subdirectory( lib )
add_subdirectory( mrv2 )

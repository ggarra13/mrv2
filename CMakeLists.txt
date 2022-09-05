cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)

project( mrv2 )

include( cmake/options.cmake )
include( cmake/functions.cmake )
include( cmake/Modules/BuildFLTK.cmake )


set( include_dirs
  ${CMAKE_PREFIX_PATH}/include
  ${CMAKE_PREFIX_PATH}/include/OpenEXR
  ${CMAKE_PREFIX_PATH}/include/Imath
  ${CMAKE_PREFIX_PATH}/include/opentimelineio/deps
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/mrv2/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/deps
  ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/etc
  ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/tests
  ${CMAKE_CURRENT_BINARY_DIR}/mrv2/lib/mrvFl
  ${FLTK_INCLUDE_DIRS}
  )


include_directories(
  ${include_dirs}
  )

add_subdirectory( tlRender/etc/SuperBuild )


include( cmake/Buildmrv.cmake )

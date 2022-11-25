cmake_minimum_required(VERSION 3.19)

project( mrViewer2 )

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)
if( POLICY CMP0135 )
  cmake_policy(SET CMP0135 NEW)
endif()

include( cmake/version.cmake )
include( cmake/options.cmake )
include( cmake/functions.cmake )

set( include_dirs
  ${CMAKE_PREFIX_PATH}/include
  ${CMAKE_PREFIX_PATH}/include/OpenEXR
  ${CMAKE_PREFIX_PATH}/include/Imath
  ${CMAKE_PREFIX_PATH}/include/opentimelineio/deps
  ${CMAKE_CURRENT_SOURCE_DIR}/mrViewer/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/deps
  ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/etc
  ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/tests
  ${CMAKE_CURRENT_BINARY_DIR}/mrViewer/lib/mrvFl  # for FLUID .h files
)

include_directories(
  ${include_dirs}
  )

set( lib_dirs
  ${CMAKE_PREFIX_PATH}/lib
  )

link_directories( ${lib_dirs} )

#
# Fetch the tlRender library as a submodule with git.
#

option(GIT_SUBMODULE "Check tlRender submodule during build if missing" ON)
if(GIT_SUBMODULE)
  find_package(Git QUIET REQUIRED)
  if(NOT EXISTS "${PROJECT_SOURCE_DIR}/tlRender/.git")
    message( FATAL_ERROR "Please run:\ngit submodule add https://github.com/ggarra13/tlRender.git\nthen run runme.sh" )
  endif()

  message(STATUS "Update/init tlRender's submodules (FSeq, etc)")
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    RESULT_VARIABLE GIT_SUBMOD_RESULT)
  if(NOT GIT_SUBMOD_RESULT EQUAL "0")
    message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
  endif()
endif()

# Omit superfluous "Up-to-date" messages.
if(NOT DEFINED CMAKE_INSTALL_MESSAGE)
  set(CMAKE_INSTALL_MESSAGE "LAZY")
endif()



add_subdirectory( tlRender/etc/SuperBuild )

if( BUILD_FLTK )
  include( cmake/Modules/BuildFLTK.cmake )
endif()

if( BUILD_BOOST )
  include( cmake/Modules/BuildBoost.cmake )
endif()

if( BUILD_MRVIEWER )
  include( cmake/Modules/BuildmrViewer.cmake )
endif()
include( cmake/packaging.cmake )

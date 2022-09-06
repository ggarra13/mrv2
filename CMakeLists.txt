cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)

project( mrViewer2 )

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
  ${CMAKE_CURRENT_BINARY_DIR}/mrViewer/lib/mrvFl
)

set( MACOSX_RPATH TRUE )

set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

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
  find_package(Git QUIET)
  if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/tlRender/.git")
    # Update submodules as needed
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
  endif()
endif()




add_subdirectory( tlRender/etc/SuperBuild )


include( cmake/Modules/BuildFLTK.cmake )
include( cmake/Modules/BuildBoost.cmake )
include( cmake/Modules/BuildmrViewer.cmake )

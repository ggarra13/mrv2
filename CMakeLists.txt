cmake_minimum_required(VERSION 3.23)



project( mrv2 )

if(APPLE)
    if( CMAKE_OSX_DEPLOYMENT_TARGET )
        message( STATUS "-------------------------------------------------" )
        message( STATUS "Building for macOS ${CMAKE_OSX_DEPLOYMENT_TARGET}" )
        message( STATUS "-------------------------------------------------" )
        set( CMAKE_CXX_FLAGS "-Wno-register")
    endif()
endif()


cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)
if( POLICY CMP0135 )
    cmake_policy(SET CMP0135 NEW)
endif()

include( cmake/version.cmake )
include( cmake/options.cmake )
include( cmake/functions.cmake )

set( mrv_FLTK_FLUID_WIDGETS ${CMAKE_CURRENT_BINARY_DIR}/mrv2/lib/mrvWidgets )

set( include_dirs
    ${CMAKE_PREFIX_PATH}/include
    ${CMAKE_PREFIX_PATH}/include/OpenEXR
    ${CMAKE_PREFIX_PATH}/include/Imath
    ${CMAKE_PREFIX_PATH}/include/opentimelineio/deps
    ${CMAKE_CURRENT_SOURCE_DIR}/mrv2/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/deps
    ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/etc
    ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/tests
    ${mrv_FLTK_FLUID_WIDGETS}
)

include_directories(
    ${include_dirs}
)


#
# Fetch the tlRender library as a submodule with git.
#

if(GIT_SUBMODULE)
    find_package(Git REQUIRED)
    
    message(STATUS "Update/init tlRender's submodules (FSeq, etc)")
    execute_process(
	COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
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


#
# brew .dylibs are compiled only for the current version of the OS.
# We rebuild zlib and gettext to be compatible with version 10.15.
# tlRender builds zlib but statically, so on macOS the OS one gets picked
# instead.
#
set( ZLIB_SHARED )
if( BUILD_ZLIB )
    include( cmake/Modules/BuildZLIB.cmake )
endif()

set( Gettext )
if( BUILD_GETTEXT )
    include( cmake/Modules/BuildGettext.cmake )
endif()

if( BUILD_FLTK )
    include( cmake/Modules/BuildFLTK.cmake )
endif()

if(NOT MRV2_PYBIND11 AND BUILD_PYTHON)
    set(BUILD_PYTHON OFF)
endif()

set( PYTHON_DEP )
if( BUILD_PYTHON )
    include( cmake/Modules/BuildPython.cmake )
endif()

set( PYBIND11 )
if( MRV2_PYBIND11 )
    include( cmake/Modules/Buildpybind11.cmake )
endif()

add_subdirectory( tlRender/etc/SuperBuild )

if(BUILD_PYTHON AND TLRENDER_USD)
    add_dependencies( USD ${PYTHON_DEP} )
endif()


set( POCO_DEP )
if( MRV2_NETWORK )
    include( cmake/Modules/BuildPoco.cmake )
endif()

set( LibHaru_DEP )
if( MRV2_PDF )
    include( cmake/Modules/BuildLibHaru.cmake )
endif()

if( BUILD_MRV2 )
    include( cmake/Modules/Buildmrv2.cmake )
endif()


cmake_minimum_required(VERSION 3.19)



project( mrv2 )

if(APPLE)
    if( CMAKE_OSX_DEPLOYMENT_TARGET )
        message( STATUS "-------------------------------------------------" )
        message( STATUS "Building for macOS ${CMAKE_OSX_DEPLOYMENT_TARGET}" )
        message( STATUS "-------------------------------------------------" )
        set( CMAKE_C_FLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} ${CMAKE_C_FLAGS})
        set( CMAKE_CXX_FLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} ${CMAKE_CXX_FLAGS})
        set( CMAKE_OBJC_FLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} ${CMAKE_OBJC_FLAGS})
        set( CMAKE_OBJCXX_FLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} ${CMAKE_OBJCXX_FLAGS})
        set( CMAKE_SHARED_LINKER_FLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} ${CMAKE_SHARED_LINKER_FLAGS})
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

set( lib_dirs
    ${CMAKE_PREFIX_PATH}/lib
)

link_directories( ${lib_dirs} )

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
#
set( Gettext )
set( ZLIB_SHARED )
if ( APPLE )
    if( CMAKE_OSX_DEPLOYMENT_TARGET VERSION_EQUAL 10.15 )
        if( BUILD_ZLIB )
            include( cmake/Modules/BuildZLIB.cmake )
        endif()

        if( BUILD_GETTEXT )
            include( cmake/Modules/BuildGettext.cmake )
        endif()
    endif()
endif()

add_subdirectory( tlRender/etc/SuperBuild )

set( BOOST )
if( BUILD_BOOST )
    include( cmake/Modules/BuildBoost.cmake )
endif()

if( BUILD_FLTK )
    include( cmake/Modules/BuildFLTK.cmake )
endif()

if( MRV2_PYBIND11 )
    if (APPLE)
        set(PYTHON_EXECUTABLE /usr/local/bin/python3.8)
    endif()
    include( cmake/Modules/Buildpybind11.cmake )
endif()

if( BUILD_MRV2 )
    include( cmake/Modules/Buildmrv2.cmake )
endif()


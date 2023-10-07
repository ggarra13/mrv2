# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

# message( STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}" )
# message( STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}" )
# message( STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" )

#
# According to the CMAKE docs CMAKE_INSTALL_PREFIX should point to the
# install directory, but on Linux, macOS and Windows each path is different.
#
if ( UNIX AND NOT APPLE )
    set( ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../../../../../../../../" )
elseif(APPLE)
    set( ROOT_DIR "${CMAKE_INSTALL_PREFIX}/../../../../../../../../../../../../" )
else()
    set( ROOT_DIR "${CMAKE_INSTALL_PREFIX}/../../../../../../../../../" )
endif()

file(REAL_PATH ${ROOT_DIR} ROOT_DIR )

message( STATUS "cmake/prepackage.cmake has ROOT_DIR=${ROOT_DIR}" )

include( "${ROOT_DIR}/cmake/functions.cmake" )


#
# @bug: UNIX CMAKE_INSTALL_PREFIX is broken!!!
#
if( UNIX AND NOT APPLE )
    set( CPACK_PREPACKAGE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}" )
else()
    set( CPACK_PREPACKAGE "${CMAKE_INSTALL_PREFIX}" )
endif()

message( STATUS "CPACK_PREPACKAGE=${CPACK_PREPACKAGE}" )

#
# Remove .a, .lib and .dll files from packaging lib/ directory
#
file( GLOB STATIC_LIBS "${CPACK_PREPACKAGE}/lib/*.a"
		       "${CPACK_PREPACKAGE}/lib/*.lib"
		       "${CPACK_PREPACKAGE}/lib/*.dll" )

if ( NOT "${STATIC_LIBS}" STREQUAL "" )
    file( REMOVE ${STATIC_LIBS} )
endif()


#
# Remove include files from packaging directory
#
file( REMOVE_RECURSE "${CPACK_PREPACKAGE}/include" )

#
# Install system .SO dependencies
#
if( UNIX)
    set( EXES "${CPACK_PREPACKAGE}/bin/mrv2" )
    if ( APPLE )
	get_macos_runtime_dependencies( ${EXES} DEPENDENCIES )
    else()
	get_runtime_dependencies( ${EXES} DEPENDENCIES )
	set(linux_lib64_dir "${CPACK_PREPACKAGE}/lib64")
	if (EXISTS "${linux_lib64_dir}" )
	    message( NOTICE "${linux_lib64_dir} exists...")
	    file(GLOB lib64_dsos "${linux_lib64_dir}/*.so*")
	    file( COPY ${linux_lib64_dsos}
		DESTINATION "${CPACK_PREPACKAGE}/lib/" )
	else()
	    message( NOTICE "${linux_lib64_dir} does not exist...")
	endif()
    endif()
    file( COPY ${DEPENDENCIES} DESTINATION "${CPACK_PREPACKAGE}/lib/" )
elseif(WIN32)
    #
    # Remove usd directory from lib/ directory on Windows
    #
    file( REMOVE_RECURSE "${CPACK_PREPACKAGE}/lib/usd" )
endif()

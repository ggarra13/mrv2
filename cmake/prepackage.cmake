# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

message( STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}" )
message( STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}" )
message( STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" )

if ( UNIX AND NOT APPLE )
    set( ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../../../../../../../../" )
else()
    set( ROOT_DIR "${CMAKE_INSTALL_PREFIX}/../../../../../../../../../../../../" )
endif()

message( STATUS "ROOT_DIR=${ROOT_DIR}" )
include( "${ROOT_DIR}/cmake/functions.cmake" )


#
# Remove .a and .lib files from packaging lib/ directory
#

#
# Note UNIX CMAKE_INSTALL_PREFIX is broken!!!
#
if( UNIX AND NOT APPLE )
    set( CPACK_PREPACKAGE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}" )
else()
    set( CPACK_PREPACKAGE "${CMAKE_INSTALL_PREFIX}" )
endif()

message( STATUS "CPACK_PREPACKAGE=${CPACK_PREPACKAGE}" )

file( GLOB STATIC_LIBS "${CPACK_PREPACKAGE}/lib/*.a"
		       "${CPACK_PREPACKAGE}/lib/*.lib" )

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
    else()
	get_runtime_dependencies( ${EXES} DEPENDENCIES )
        file( COPY ${DEPENDENCIES} DESTINATION "${CPACK_PREPACKAGE}/lib/" )
    endif()
endif()

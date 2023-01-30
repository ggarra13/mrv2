# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( "${CMAKE_CURRENT_BINARY_DIR}/../../../../../../../../../cmake/functions.cmake" )


#
# Remove .a and .lib files from packaging lib/ directory
#

set( CPACK_PREPACKAGE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}" )
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
if( UNIX AND NOT APPLE )
    set( EXES "${CPACK_PREPACKAGE}/bin/mrv2" )

    get_runtime_dependencies( ${EXES} DEPENDENCIES )
endif()
file( COPY ${DEPENDENCIES} DESTINATION "${CPACK_PREPACKAGE}/lib/" )

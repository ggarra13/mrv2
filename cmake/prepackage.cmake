# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Remove .a files from packaging lib/ directory
#

file( GLOB STATIC_LIBS "${CMAKE_INSTALL_PREFIX}/lib/*.a" "${CMAKE_INSTALL_PREFIX}/lib/*.lib" )

if ( NOT "${STATIC_LIBS}" STREQUAL "" )
    file( REMOVE ${STATIC_LIBS} )
endif()

#
# Remove include files from packaging directory
#
file( REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}/include" )

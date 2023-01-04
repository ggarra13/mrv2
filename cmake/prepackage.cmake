# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

message( STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" )
message( STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}" )
message( STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}" )
message( STATUS $ENV{DESTDIR} )

file( GLOB STATIC_LIBS "${CMAKE_INSTALL_PREFIX}/lib/*.a" )
file( REMOVE ${STATIC_LIBS} )
file( REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}/include" )

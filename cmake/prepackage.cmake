# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Remove .a files from packaging lib/ directory
#

file( GLOB STATIC_LIBS "${CMAKE_INSTALL_PREFIX}/lib/*.a" )
file( REMOVE ${STATIC_LIBS} )

#
# Remove include files from packaging directory
#
file( REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}/include" )

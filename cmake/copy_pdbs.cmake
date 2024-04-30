# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

message( STATUS "----------------------------------------copy_pdbs------------------------------------" )
message( STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}" )
message( STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}" )
message( STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" )


#
# \@bug:
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
message( STATUS "ROOT_DIR=${ROOT_DIR}" )


include( "${ROOT_DIR}/cmake/functions.cmake" )

#
# Copy PDBs.  Unfortunately, CMAKE_BUILD_TYPE is undefined when this is called.
#
set(MRV2_DEST_DIR "${CMAKE_INSTALL_PREFIX}/applications")
if (NOT EXISTS ${MRV2_DEST_DIR})
    copy_pdbs( ${ROOT_DIR} ${CMAKE_INSTALL_PREFIX} )
else()
    copy_pdbs( ${ROOT_DIR} ${MRV2_DEST_DIR} )
endif()



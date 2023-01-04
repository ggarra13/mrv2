# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ../../../../../cmake/version.cmake )
set( CPACK_PACKAGE_VERSION_MAJOR "${mrViewer2_VERSION_MAJOR}" )
set( CPACK_PACKAGE_VERSION_MINOR "${mrViewer2_VERSION_MINOR}" )
set( CPACK_PACKAGE_VERSION_PATCH "${mrViewer2_VERSION_PATCH}" )
set( CPACK_PACKAGE_VERSION "${mrViewer2_VERSION_MAJOR}.${mrViewer2_VERSION_MINOR}.${mrViewer2_VERSION_PATCH}")
set( SHORTVERSION ${CPACK_PACKAGE_VERSION} )
set( mrv2_dir "mrViewer2-${SHORTVERSION}-Darwin" )
set( cpack_dir ${CMAKE_CURRENT_BINARY_DIR}/_CPack_Packages/Darwin/Bundle/${mrv2_dir}/mrViewer2.app/Contents/Resources/ )



message( STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" )
message( STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}" )
message( STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}" )
message( STATUS "cpack_dir=${cpack_dir}" )

file( GLOB STATIC_LIBS "${cpack_dir}/lib/*.a" )
file( REMOVE "${STATIC_LIBS}" )
file( REMOVE_RECURSE "${cpack_dir}/include" )

# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

if ( APPLE )
  set( build_zlib TRUE )
  set( build_gettext TRUE )
else()
  set( build_zlib FALSE )
  set( build_gettext FALSE )
endif()

set(BUILD_ZLIB ${build_zlib} CACHE BOOL "Enable ZLIB building")
set(BUILD_GETTEXT ${build_gettext} CACHE BOOL "Enable GETTEXT building")
set(BUILD_FLTK TRUE CACHE BOOL "Enable FLTK building")
set(BUILD_BOOST TRUE CACHE BOOL "Enable Boost building")
set(BUILD_MRVIEWER TRUE CACHE BOOL "Enable mrViewer2 (mrv2) building")

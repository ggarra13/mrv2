# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set( build_zlib FALSE )
set( build_gettext FALSE )
if ( APPLE )
    #
    # brew .dylibs are compiled only for the current version of the OS.
    # We rebuild zlib and gettext to be compatible with version 10.15.
    #
    if( CMAKE_OSX_DEPLOYMENT_TARGET VERSION_EQUAL 10.15 )
	set( build_zlib TRUE )
	set( build_gettext TRUE )
    endif()
endif()

set(BUILD_ZLIB ${build_zlib} CACHE BOOL "Enable ZLIB building")
set(BUILD_GETTEXT ${build_gettext} CACHE BOOL "Enable GETTEXT building")
set(BUILD_FLTK TRUE CACHE BOOL "Enable FLTK building (needed)")
set(BUILD_PYTHON TRUE CACHE BOOL "Enable building Python" )
set(MRV2_PYBIND11 TRUE CACHE BOOL "Enable pybind11 building" )
set(MRV2_NETWORK TRUE CACHE BOOL "Enable Networking in mrv2" )
set(MRV2_PDF TRUE CACHE BOOL "Enable PDF creation in mrv2" )
set(MRV2_DIST_RELEASE FALSE CACHE BOOL "Will remove some disk space but it is not good for recompilation.")
set(BUILD_MRV2 TRUE CACHE BOOL "Enable mrv2 building")
option(GIT_SUBMODULE "Check tlRender submodule during build if missing" ON)


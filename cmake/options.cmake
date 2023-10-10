# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set(BUILD_ZLIB FALSE CACHE BOOL "Enable ZLIB building")
set(BUILD_GETTEXT FALSE CACHE BOOL "Enable GETTEXT building")
set(BUILD_FLTK TRUE CACHE BOOL "Enable FLTK building (needed)")
set(BUILD_MRV2 TRUE CACHE BOOL "Enable mrv2 building")

set(BUILD_PYTHON TRUE CACHE BOOL "Enable building Python" )
set(MRV2_PYBIND11 TRUE CACHE BOOL "Enable pybind11 building" )
set(MRV2_PYFLTK   TRUE CACHE BOOL "Enable pyFLTK binding" )
set(MRV2_NETWORK TRUE CACHE BOOL "Enable Networking in mrv2" )
set(MRV2_PDF TRUE CACHE BOOL "Enable PDF creation in mrv2" )

option(GIT_SUBMODULE "Check tlRender submodule during build if missing" ON)


# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

set(vcpkg_REPOSITORY "https://github.com/Microsoft/vcpkg.git")
set(vcpkg_TAG 2025.08.27)

set(vcpkg_CONFIGURE bootstrap-vcpkg.bat -disableMetrics)
if (UNIX)
    set(vcpkg_CONFIGURE bootstrap-vcpkg.sh -disableMetrics)
endif()

#
# Sourcecode is in github:/microsoft/vcpkg-tool
#
ExternalProject_Add(
    vcpkg
    GIT_REPOSITORY ${vcpkg_REPOSITORY}
    GIT_TAG ${vcpkg_TAG}
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps/vcpkg
    CONFIGURE_COMMAND bootstrap-vcpkg.bat -disableMetrics
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
)

set( VCPKG_ROOT ${CMAKE_CURRENT_BINARY_DIR}/deps/vcpkg/src/vcpkg)
if (WIN32)
    set( VCPKG ${VCPKG_ROOT}/vcpkg.exe)
else()
    set( VCPKG ${VCPKG_ROOT}/vcpkg.exe)
endif()
set( vcpkg_DEP vcpkg )

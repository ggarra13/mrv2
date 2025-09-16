# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

set(vcpkg_REPOSITORY "https://github.com/Microsoft/vcpkg.git")
set(vcpkg_TAG 2025.08.27)

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

set( VCPKG ${CMAKE_CURRENT_BINARY_DIR}/deps/vcpkg/src/vcpkg/vcpkg.exe)
set( vcpkg_DEP vcpkg )

# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

set(libbacktrace_TAG master)

ExternalProject_Add(
    libbacktrace
    GIT_REPOSITORY "https://github.com/ianlancetaylor/libbacktrace.git"
    GIT_TAG ${libbacktrace_TAG}
    CONFIGURE_COMMAND ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1
)

set( libbacktrace_DEP libbacktrace )

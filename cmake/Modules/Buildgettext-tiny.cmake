# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

set( gettext-tiny_REPOSITORY "https://github.com/sabotage-linux/gettext-tiny.git")
set( gettext-tiny_TAG v0.3.2 )

set( gettext-tiny_INSTALL ${CMAKE_COMMAND} -E copy_if_different libintl.a ${CMAKE_INSTALL_PREFIX}/lib
    COMMAND ${CMAKE_COMMAND} -E copy_if_different include/libintl.h ${CMAKE_INSTALL_PREFIX}/include )

ExternalProject_Add(
    gettext-tiny
    GIT_REPOSITORY ${gettext-tiny_REPOSITORY}
    GIT_TAG ${gettext-tiny_TAG}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make
    INSTALL_COMMAND ${gettext-tiny_INSTALL}
    BUILD_IN_SOURCE 1
)

set( Gettext gettext-tiny )

# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

# Stable TAG
set( FLTK_TAG 64ac7b47b81549bce8911422209c81b77a9232ba )
#set( FLTK_TAG master )


set( patch_cmd )

if (APPLE OR WIN32)
    set( wayland OFF )
    set( pango   OFF )
else()
    set( wayland  ON ) # we'll leave it on, but it is way too
		        # buggy with, at least, Nvidia cards.
    set( pango    ON )
endif()

ExternalProject_Add(
    FLTK
    GIT_REPOSITORY "https://github.com/fltk/fltk.git"
    GIT_TAG ${FLTK_TAG}
    GIT_PROGRESS 1
    PATCH_COMMAND ${patch_cmd}
    CMAKE_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHIECTURES}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DFLTK_BUILD_EXAMPLES=OFF
    -DFLTK_BUILD_TEST=OFF
    -DOPTION_BUILD_SHARED_LIBS=0
    -DOPTION_USE_SYSTEM_ZLIB=0
    -DOPTION_USE_SYSTEM_LIBJPEG=0
    -DOPTION_USE_SYSTEM_LIBPNG=0
    -DOPTION_USE_PANGO=${pango}
    -DOPTION_USE_WAYLAND=${wayland}
    )

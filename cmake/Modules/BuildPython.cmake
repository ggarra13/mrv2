# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

set( PYTHON_VERSION 3.10 )
set( PYTHON_PATCH   6 )
set( PYTHON_URL https://www.python.org/ftp/python/${PYTHON_VERSION}.${PYTHON_PATCH}/Python-${PYTHON_VERSION}.${PYTHON_PATCH}.tar.xz )

if(UNIX)
    set( PYTHON_CONFIGURE ./configure --enable-optimizations --prefix=${CMAKE_INSTALL_PREFIX} )
    set( PYTHOM_BUILD    make -j 4)
    set( PYTHON_INSTALL  make install )
else()
    set( PYTHON_CONFIGURE  )
    set( PYTHOM_BUILD PCBuild/build.bat -e -q )
    set( PYTHON_INSTALL python${PYTHON_VERSION} -m pip install . )
endif()

ExternalProject_Add(
    Python
    URL ${PYTHON_URL}
    CONFIGURE_COMMAND ${PYTHON_CONFIGURE}
    BUILD_COMMAND     ${PYTHON_BUILD}
    INSTALL_COMMAND   ${PYTHON_INSTALL}
    BUILD_IN_SOURCE 1
)

set( PYTHON Python )

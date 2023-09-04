# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

set( PYTHON_VERSION 3.10 )
set( PYTHON_TINY   9 )
set( PYTHON_URL https://www.python.org/ftp/python/${PYTHON_VERSION}.${PYTHON_TINY}/Python-${PYTHON_VERSION}.${PYTHON_TINY}.tar.xz )

set( PYTHON_PATCH )
if(APPLE)
    set( PYTHON_PATCH cmake -E copy ${PROJECT_SOURCE_DIR}/etc/configure-macos-python.sh ${CMAKE_CURRENT_BINARY_DIR}/ )
    set( PYTHON_CONFIGURE ${CMAKE_CURRENT_BINARY_DIR}/configure-macos-python.sh --prefix=${CMAKE_INSTALL_PREFIX}
        "CFLAGS=${CMAKE_C_FLAGS}"
        "CPPFLAGS=${CMAKE_C_FLAGS}"
        "CXXFLAGS=${CMAKE_CXX_FLAGS}"
    )
    set( PYTHON_BUILD    make -j 4)
    set( PYTHON_INSTALL  make altinstall )
elseif(UNIX)
    set( PYTHON_CONFIGURE ./configure --enable-optimizations
        --prefix=${CMAKE_INSTALL_PREFIX}
        "CFLAGS=${CMAKE_C_FLAGS}"
        "CPPFLAGS=${CMAKE_C_FLAGS}"
        "CXXFLAGS=${CMAKE_CXX_FLAGS}"
    )
    set( PYTHON_BUILD    make -j 4)
    set( PYTHON_INSTALL  make altinstall )
else()
    set( platform x64 )
    if( "$ENV{ARCH}" STREQUAL "i386" )
    	set( platform Win32 )
    endif()
    set( PYTHON_CONFIGURE )
    set( PYTHON_BUILD  PCbuild/build.bat -e -q -p ${platform} )
    set( PYTHON_INSTALL python.bat PC/layout --precompile --preset-default  --copy "${CMAKE_INSTALL_PREFIX}/bin/" )
endif()

ExternalProject_Add(
    Python
    URL ${PYTHON_URL}
    PATCH_COMMAND     ${PYTHON_PATCH}
    CONFIGURE_COMMAND "${PYTHON_CONFIGURE}"
    BUILD_COMMAND     ${PYTHON_BUILD}
    INSTALL_COMMAND   ${PYTHON_INSTALL}
    BUILD_IN_SOURCE 1
)


set( PYTHON_DEP Python )

#
# This is needed for our own tlRender's USD building
#
if(UNIX)
    set( PYTHON_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/python${PYTHON_VERSION} )
else()
    set( PYTHON_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/python.exe )
endif()


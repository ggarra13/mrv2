# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )
	
set( Python_VERSION 3.10 )
set( Python_TINY   9 )
set( Python_URL https://www.python.org/ftp/python/${Python_VERSION}.${Python_TINY}/Python-${Python_VERSION}.${Python_TINY}.tar.xz )

set( Python_PATCH )
if(APPLE)
    set( Python_PATCH cmake -E copy ${PROJECT_SOURCE_DIR}/etc/configure-macos-python.sh ${CMAKE_CURRENT_BINARY_DIR}/ )
    
    set( Python_CXX_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    set( Python_C_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} ${CMAKE_C_FLAGS}")
    
    set( Python_CONFIGURE ${CMAKE_CURRENT_BINARY_DIR}/configure-macos-python.sh --prefix=${CMAKE_INSTALL_PREFIX}
        "CFLAGS=${Python_C_FLAGS}"
        "CPPFLAGS=${Python_C_FLAGS}"
        "CXXFLAGS=${Python_CXX_FLAGS}"
    )
    set( Python_BUILD    )
    set( Python_INSTALL  make altinstall )
elseif(UNIX)
    set( Python_CONFIGURE ./configure
	--enable-optimizations
	--enable-shared
        --prefix=${CMAKE_INSTALL_PREFIX}
        "CFLAGS=${CMAKE_C_FLAGS}"
        "CPPFLAGS=${CMAKE_C_FLAGS}"
        "CXXFLAGS=${CMAKE_CXX_FLAGS}"
    )
    set( Python_BUILD    )
    set( Python_INSTALL  make altinstall )
else()
    set( platform x64 )
    if( "$ENV{ARCH}" STREQUAL "i386" )
    	set( platform Win32 )
    endif()
    set( Python_CONFIGURE )
    set( Python_BUILD  PCbuild/build.bat -e -q -p ${platform} )
    set( Python_INSTALL python.bat PC/layout --precompile --preset-default  --copy "${CMAKE_INSTALL_PREFIX}/bin/" )
endif()

ExternalProject_Add(
    Python
    URL ${Python_URL}
    PATCH_COMMAND     "${Python_PATCH}"
    CONFIGURE_COMMAND ${Python_CONFIGURE}
    BUILD_COMMAND     ${Python_BUILD}
    INSTALL_COMMAND   "${Python_INSTALL}"
    BUILD_IN_SOURCE 1
)


set( PYTHON_DEP Python )

#
# This is needed for our own tlRender's USD building
#
if(UNIX)
    set( Python_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/python${Python_VERSION} )
else()
    set( Python_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/python.exe )
endif()

set(PYTHON_EXECUTABLE ${Python_EXECUTABLE} )

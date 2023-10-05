# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )


if(APPLE)
    set(GETTEXT_CXX_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} ${CMAKE_CXX_FLAGS}")
    set(GETTEXT_C_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} ${CMAKE_C_FLAGS}")
endif()

set(GETTEXT_ARGS
	--enable-relocatable
	--disable-java
	--disable-static
	--enable-shared
	--disable-curses

	--disable-c++
	--disable-curses
	--disable-nls
	--disable-acl
	--disable-dependency-tracking
	--disable-libaseprintf
	--disable-openmp
	--disable-xattr
	--without-libtextstyle-prefix
	--without-emacs
	--without-bzip2
	--without-xz
	--prefix=${CMAKE_INSTALL_PREFIX})
    
ExternalProject_Add(
    GETTEXT
    URL "https://ftp.gnu.org/gnu/gettext/gettext-0.22.3.tar.gz"
    CONFIGURE_COMMAND ./configure ${GETTEXT_ARGS}
    BUILD_COMMAND     make libintl.dylib
    "CFLAGS=${GETTEXT_C_FLAGS}"
    "CPPFLAGS=${GETTEXT_C_FLAGS}"
    "CXXFLAGS=${GETTEXT_CXX_FLAGS}"
    BUILD_IN_SOURCE 1
)

set( Gettext GETTEXT )

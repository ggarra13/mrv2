# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )


if(APPLE)
    set(GETTEXT_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(GETTEXT_C_FLAGS "${CMAKE_C_FLAGS}")
    if (CMAKE_OSX_DEPLOYMENT_TARGET)
	list(APPEND GETTEXT_CXX_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
	list(APPEND GETTEXT_C_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
endif()

set(GETTEXT_ARGS
    --disable-silent-rules
    --with-included-glib
    --with-included-libcroco
    --with-included-libunistring
    --with-included-libxml
    --with-included-gettext
    
    --disable-java
    --disable-csharp

    --without-git
    --without-cvs
    --without-xz
    
    --prefix=${CMAKE_INSTALL_PREFIX})

ExternalProject_Add(
    GETTEXT
    URL "https://ftp.gnu.org/gnu/gettext/gettext-0.21.1.tar.gz"
    CONFIGURE_COMMAND ./configure ${GETTEXT_ARGS}
    "CFLAGS=${GETTEXT_C_FLAGS}"
    "CXXFLAGS=${GETTEXT_CXX_FLAGS}"
    BUILD_COMMAND make -j 16
    BUILD_IN_SOURCE 1
)


# ExternalProject_Add(
#      GETTEXT
#      URL "https://ftp.gnu.org/gnu/gettext/gettext-0.21.1.tar.gz"
#      CONFIGURE_COMMAND ""
#      BUILD_COMMAND ""
#      INSTALL_COMMAND "true"
#      BUILD_IN_SOURCE 1
# )



set( Gettext GETTEXT )

# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

set(FLTK_GIT_TAG v1.5.0)  # v1.4.9 was latest mrv2 v1.5.1 release

#set(FLTK_GIT_TAG vk)  # Cutting edge!
#set(FLTK_GIT_TAG vk_merge) # Testing branch

if(MRV2_VK)
    message(STATUS "Using ggarra13's private FLTK branch")
    set(USER_NAME $ENV{USER})
    if (USER_NAME STREQUAL "gga" OR USER_NAME STREQUAL "User-PC" OR
	    USER_NAME STREQUAL "ggarra13")
	# If it is me, we check the repository with git so we can easily edit it
	# later.
	set(FLTK_GIT_REPOSITORY "git@github.com:ggarra13/fltk.git")
    else()
        set(FLTK_GIT_REPOSITORY "https://github.com/ggarra13/fltk.git")
    endif()
else()
    message(WARNING "Using FLTK's original FLTK branch")
    set(FLTK_GIT_REPOSITORY "https://github.com/fltk/fltk.git")
    set(FLTK_GIT_TAG master)
endif()

# If we are building pyFLTK compile shared too.  FLTK compiles statically for fluid.
set( FLTK_BUILD_SHARED_LIBS ON )  

set( FLTK_BUILD_TYPE ${CMAKE_BUILD_TYPE} )

set( FLTK_C_COMPILER ${NATIVE_COMPILER})
set( FLTK_C_FLAGS ${CMAKE_C_FLAGS})
set( FLTK_CXX_FLAGS ${CMAKE_CXX_FLAGS})

if(APPLE)
    set(FLTK_C_COMPILER /usr/bin/cc)
    set(FLTK_CXX_COMPILER /usr/bin/c++)
elseif(WIN32)
    set(FLTK_C_COMPILER ${NATIVE_C_COMPILER})
    set(FLTK_CXX_COMPILER ${NATIVE_CXX_COMPILER})
elseif(UNIX)
    set(FLTK_C_COMPILER ${NATIVE_C_COMPILER})
    set(FLTK_CXX_COMPILER ${NATIVE_CXX_COMPILER})
    list(APPEND FLTK_C_FLAGS -fPIC)
    list(APPEND FLTK_CXX_FLAGS -fPIC)
endif()

set(GITHUB_REPO "$ENV{GITHUB_REPO}")
if (NOT GITHUB_REPO STREQUAL "mrv2")
    message(FATAL_ERROR "Invalid repository")
endif()

# These two are always built by tlRender
set(FLTK_USE_SYSTEM_ZLIB TRUE)
set(FLTK_USE_SYSTEM_LIBPNG TRUE)


# We set this to use FLTK's built-in libdecor
set(FLTK_USE_SYSTEM_LIBDECOR FALSE)

# Set this to FALSE to use libdecor's uglier looking windows' borders
# instead of GTK's nicer window borders.  Note that using GTK's borders will
# result in a warning due to FLTK and GLFW calling the same function.
set(FLTK_USE_LIBDECOR_GTK FALSE)

# This one may be turned off

# Set FLTK default dependencies
set(FLTK_DEPENDENCIES PNG ${VMA_DEP} ${SHADERC_DEP}
		      	  ${WAYLAND_PROTOCOLS_DEP} ZLIB)

set(FLTK_USE_SYSTEM_LIBJPEG FALSE)
if(TLRENDER_JPEG)
    set(FLTK_USE_SYSTEM_LIBJPEG TRUE)
    set(FLTK_DEPENDENCIES libjpeg-turbo ${FLTK_DEPENDENCIES})
endif()

message(STATUS "FLTK DEPENDENCIES=${FLTK_DEPENDENCIES}")

#
# Determine whether to build fltk-gl
#
# We always build GL for pyFLTK that uses it.
set(FLTK_BUILD_GL ON)
if (FLTK_BUILD_GL)
    message(STATUS "FLTK BUILD OpenGL ${FLTK_BUILD_GL}")
endif()

#
# Determine whether to build fltk-vk
#
set(FLTK_BUILD_VK ${TLRENDER_VK})
if (FLTK_BUILD_VK)
    message(STATUS "FLTK BUILD Vulkan ${FLTK_BUILD_VK}")
endif()

set(FLTK_PATCH
)

if (APPLE OR WIN32)
    # We modify and use TLRENDER_* variables to propagate them to tlRender
    set(TLRENDER_X11 OFF)
    set(TLRENDER_WAYLAND OFF )
    set(FLTK_USE_PANGO   OFF )
else()
    # Linux
    # We modify and use TLRENDER_* variables to propagate them to tlRender
    set(TLRENDER_X11 ON)
    set(TLRENDER_WAYLAND ON)
    set(FLTK_USE_PANGO   ON)
endif()

    
ExternalProject_Add(
    FLTK
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps/FLTK

    GIT_REPOSITORY ${FLTK_GIT_REPOSITORY}
    GIT_TAG ${FLTK_GIT_TAG}


    GIT_SHALLOW       OFF  # for ExternalProject_Add.
                           # For FetchContent_Declere it is GIT_SHALLOW FALSE
    
    DEPENDS ${FLTK_DEPENDENCIES}
    
    CMAKE_ARGS
    -DCMAKE_C_COMPILER=${FLTK_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${FLTK_CXX_COMPILER}
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHIECTURES}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    -DCMAKE_VERBOSE_MAKEFILE=ON
    -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH_TMP}
    -DCMAKE_BUILD_TYPE=${FLTK_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_TMP}
    -DCMAKE_C_FLAGS=${FLTK_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${FLTK_CXX_FLAGS}
    -DCMAKE_CXX_STANDARD=17  # 17 needed for VMA
    -DCMAKE_INSTALL_MESSAGE=${CMAKE_INSTALL_MESSAGE}
    -DFLTK_BUILD_VK=${FLTK_BUILD_VK}
    -DFLTK_BUILD_GL=${FLTK_BUILD_GL}
    -DFLTK_BUILD_EXAMPLES=OFF
    -DFLTK_BUILD_FLUID=ON
    -DFLTK_BUILD_FLTK_OPTIONS=OFF
    -DFLTK_BUILD_HTML_DOCS=OFF
    -DFLTK_BUILD_TEST=OFF
    -DFLTK_BUILD_SHARED_LIBS=${FLTK_BUILD_SHARED_LIBS}
    -DFLTK_BUILD_FORMS=OFF
    -DFLTK_BACKEND_WAYLAND=${TLRENDER_WAYLAND}
    -DFLTK_BACKEND_X11=${TLRENDER_X11}
    -DFLTK_USE_SYSTEM_LIBDECOR=${FLTK_USE_SYSTEM_LIBDECOR}
    -DFLTK_USE_LIBDECOR_GTK=${FLTK_USE_LIBDECOR_GTK}
    -DFLTK_USE_SYSTEM_ZLIB=${FLTK_USE_SYSTEM_ZLIB}
    -DFLTK_USE_SYSTEM_LIBJPEG=${FLTK_USE_SYSTEM_LIBJPEG}
    -DFLTK_USE_SYSTEM_LIBPNG=${FLTK_USE_SYSTEM_LIBPNG}
    -DFLTK_USE_PANGO=${FLTK_USE_PANGO}
)

set(FLTK_DEP FLTK)

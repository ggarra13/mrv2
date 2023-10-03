# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

# Stable TAG
set( LibHaru_TAG v2.4.3 )
#set( LibHaru_TAG master )

set( patch_cmd )


ExternalProject_Add(
    LibHaru
    GIT_REPOSITORY "https://github.com/libharu/libharu.git"
    GIT_TAG ${LibHaru_TAG}
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
    -DCMAKE_INSTALL_MESSAGE=${CMAKE_INSTALL_MESSAGE}
    -DBUILD_SHARED_LIBS=OFF
    -DLIBHPDF_EXAMPLES=OFF
    DEPENDS tlRender
)

set(LibHaru_DEP LibHaru )

# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

# Stable TAG
set(libdatachannel_REPOSITORY "https://github.com/paullouisageneau/libdatachannel.git" )
#set(libdatachannel_GIT_TAG v0.24.2 )
set(libdatachannel_GIT_TAG 0400bc1e0452f62be14025eabde2bbf83bb1cad6)

set(libdatachannel_DEPENDENCIES )
list(APPEND libdatachannel_DEPENDENCIES ${OpenSSL_DEP} nlohmann_json)
message(STATUS "libdatachannel DEPENDENCIES=${libdatachannel_DEPENDENCIES}")

set(libdatachannel_UPDATE_CMD
    COMMAND git submodule update --init)

ExternalProject_Add(
    libdatachannel
    GIT_REPOSITORY ${libdatachannel_REPOSITORY}
    GIT_TAG ${libdatachannel_GIT_TAG}
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps/libdatachannel
    
    UPDATE_COMMAND ${libdatachannel_UPDATE_CMD}

    DEPENDS ${libdatachannel_DEPENDENCIES}

    LIST_SEPARATOR |
    
    CMAKE_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHIECTURES}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH_TMP}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_TMP}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_INSTALL_MESSAGE=${CMAKE_INSTALL_MESSAGE}
    -DENABLE_OPENSSL=ON
    -DUSE_SYSTEM_JSON=ON
    -DNO_EXAMPLES=TRUE
)

set( libdatachannel_DEP libdatachannel )

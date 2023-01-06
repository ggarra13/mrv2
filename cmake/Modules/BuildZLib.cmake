# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

ExternalProject_Add(
  ZLIB_SHARED
  URL "https://zlib.net/fossils/zlib-1.2.11.tar.gz"
  CMAKE_ARGS
  -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
  -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  ${MRV_EXTERNAL_ARGS}
  -Dlibdeflate=OFF
  DEPENDS ZLIB
  )

set( ZLIB_SHARED ZLIB_SHARED )

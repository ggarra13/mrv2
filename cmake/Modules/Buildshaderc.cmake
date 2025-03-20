# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

set(shaderc_REPOSITORY "https://github.com/jeremy-rifkin/shaderc.git")

ExternalProject_Add(
    shaderc
    GIT_REPOSITORY ${shaderc_REPOSITORY}
    GIT_SUBMODULES_RECURSE TRUE
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps/shaderc
    CMAKE_ARGS
    -DSHADERC_ENABLE_SHARED_CRT=ON
    -DSHADERC_SKIP_TESTS=ON
)

set( shaderc_DEP shaderc )

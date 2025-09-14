# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include(ExternalProject)

set(shaderc_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
)

ExternalProject_Add(
    shaderc
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/shaderc
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
    LIST_SEPARATOR |
    CMAKE_ARGS ${shaderc_ARGS}
)

set(SHADERC_DEP shaderc)

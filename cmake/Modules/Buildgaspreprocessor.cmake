# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

set(gaspreprocessor_REPOSITORY "https://github.com/FFmpeg/gas-preprocessor.git")
set(gaspreprocessor_TAG master)

ExternalProject_Add(
    gaspreprocessor
    GIT_REPOSITORY ${gaspreprocessor_REPOSITORY}
    GIT_TAG ${gaspreprocessor_TAG}
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps/gaspreprocessor
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
)

set(gaspreprocessor_DEP gasprocessor)

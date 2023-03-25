# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



set( ROOT_DIR ${PROJECT_SOURCE_DIR}/.. )

add_custom_target(
    docs
    COMMAND doxygen
    WORKING_DIRECTORY "${ROOT_DIR}"
    DEPENDS mrv2
)


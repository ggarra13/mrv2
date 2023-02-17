#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
# This script formats all source code to follow the .clang-format specification.
# Currently, we follow the llvm source code formatting.
#
echo "Formatting code...."
find mrv2 -regex '.*\.\(cpp\|hpp\|cu\|c\|h\)' -exec clang-format -style=file -i {} \;

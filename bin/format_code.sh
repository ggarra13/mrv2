#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.




#
#  This script will format all code to the .clang-format specification.
#  It should be run before submitting a PR request.
#
# You must run it from the root of the mrv2 project.
#

echo "Formatting code with clang..."

find mrv2 \( -name \*.h -o -name \*.cpp \) -exec clang-format --verbose --style=file -i {} \;

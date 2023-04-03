#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script installs with pip the needed modules for documentation
#

if [[ $KERNEL == *Msys* ]]; then
    export PYTHON=python.exe
else
    export PYTHON=python3
fi

$BUILD_DIR/install/bin/$PYTHON -m pip install sphinx sphinx_rtd_theme

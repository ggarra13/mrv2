#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script installs with pip the needed modules for documentation
#

export PYTHON=`ls $BUILD_DIR/install/bin/python* | grep -o 'python.*' | head -1 `
run_cmd $BUILD_DIR/install/bin/$PYTHON -m pip install --upgrade pip
run_cmd $BUILD_DIR/install/bin/$PYTHON -m pip install sphinx sphinx_rtd_theme requests urllib3

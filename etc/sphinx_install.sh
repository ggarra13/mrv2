#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script installs with pip the needed modules for documentation
#

locate_python

export PYTHONPATH=$PYTHON_SITEDIR:$PYTHON_PATH
if [[ ! -d ${SITE_DIR}/sphinx ]]; then
    run_cmd $PYTHON -m pip install --upgrade pip
    run_cmd $PYTHON -m pip install --upgrade sphinx sphinx_rtd_theme requests urllib3
fi

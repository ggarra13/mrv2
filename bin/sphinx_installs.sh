#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script installs the Sphinx debendencies for documenting mrv2.
#
pip3=`which pip3`
if [[ $pip3 == "*not found*" ]]; then
    pip3=pip
fi

${pip3} install sphinx sphinx-autodoc-typehints

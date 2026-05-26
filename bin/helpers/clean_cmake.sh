#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# Cleans up USD directories for an upgrade of OpenUSD.
#


#
# Find out our build dir
#
. etc/build_dir.sh

echo "This script will clear cmake at $BUILD_DIR"
ask_to_continue


export INSTALL=$BUILD_DIR/install

echo "Cleaning bin directory"
rm -rf $INSTALL/bin/cmake*

echo "Cleaning share/cmake-* directory"
rm -rf $INSTALL/share/cmake-*


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

echo "This script will clear Python, pybind11 and pyFLTK at $BUILD_DIR"
ask_to_continue


export INSTALL=$BUILD_DIR/install

echo "Cleaning bin directory"
rm -rf $INSTALL/bin/python*
rm -rf $INSTALL/bin/tcl*

echo "Cleaning lib directory"
rm -rf $INSTALL/lib/python*

if [[ $KERNEL == *Windows* ]]; then
    echo "Cleaning Windows' bin/DLLs directory"
    rm -rf $INSTALL/bin/DLLs

    echo "Cleaning Windows' bin/Libs directory"
    rm -rf $INSTALL/bin/Lib
    
    echo "Cleaning Windows' bin/Scripts directory"
    rm -rf $INSTALL/bin/Scripts
fi

echo "Cleaning deps/Python"
rm -rf $BUILD_DIR/deps/Python

echo "Cleaning deps/pyFLTK"
rm -rf $BUILD_DIR/deps/pyFLTK

echo "Cleaning deps/pybind11"
rm -rf $BUILD_DIR/deps/pybind11

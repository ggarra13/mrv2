#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

echo "Getting latest release of cmake"

. ./etc/build_dir.sh

echo "Will install it in $PWD/$BUILD_DIR/install.."

echo "Downloading cmake..."
wget -c -q https://github.com/Kitware/CMake/releases/download/v3.26.3/cmake-3.26.3-linux-x86_64.tar.gz

echo "Decompressing archive..."
tar -xf cmake-3.26.3-linux-x86_64.tar.gz

echo "Installing..."
if [[ ! -d $PWD/$BUILD_DIR/install/ ]]; then
    mkdir -p $PWD/$BUILD_DIR/install/
fi
mv -f cmake-3.26.3-linux-x86_64/* $PWD/$BUILD_DIR/install/

echo "Cleaning up..."
rm -rf cmake-3.26.3-linux-x86_64*

echo "Checking executable is there:"
ls  $BUILD_DIR/install/bin
echo "Done with installing cmake"

#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. ./etc/build_dir.sh

echo "Will install it in $PWD/$BUILD_DIR/install.."
if [[ ! -d $PWD/$BUILD_DIR/install/ ]]; then
    mkdir -p $PWD/$BUILD_DIR/install/
fi

cmake_version=3.28.1  # 3.26.3 worked on Docker

if [[ ! -e cmake-${cmake_version}-linux-x86_64.tar.gz ]]; then
    echo "Cleaning up old cmake version..."
    rm -rf cmake-${cmake_version}-linux-x86_64*
    
    echo "Downloading cmake v${cmake_version}..."
    curl -# -L -o cmake-${cmake_version}-linux-x86_64.tar.gz https://github.com/Kitware/CMake/releases/download/v${cmake_version}/cmake-${cmake_version}-linux-x86_64.tar.gz
    
    echo "Decompressing archive..."
    tar -xf cmake-${cmake_version}-linux-x86_64.tar.gz
    
    echo "Installing..."
    
    if [[ ! -d $PWD/$BUILD_DIR/install/bin ]]; then
	mv -f cmake-${cmake_version}-linux-x86_64/* $PWD/$BUILD_DIR/install/
    else
	rsync -avz cmake-${cmake_version}-linux-x86_64/ $PWD/$BUILD_DIR/install/
    fi
    
    echo "Cleaning up..."
    rm -rf cmake-${cmake_version}-linux-x86_64*

    echo "Checking executable is there:"
    ls  $BUILD_DIR/install/bin
    echo "Done with installing cmake"
fi

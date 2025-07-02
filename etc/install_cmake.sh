#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

CMAKE_RELEASE=3.31.8 
CMAKE_PLATFORM=macos-universal
CMAKE_EXT=tar.gz

echo "Getting latest release of cmake"
. ./etc/build_dir.sh

if [[ ! -d $PWD/$BUILD_DIR/install ]]; then
    mkdir -p $PWD/$BUILD_DIR/install
fi

if [[ -e $PWD/$BUILD_DIR/install/bin/cmake ]]; then
    return
fi

echo "Will install it in $PWD/$BUILD_DIR/install.."
if [[ $KERNEL == *Linux* ]]; then
    CMAKE_PLATFORM=linux-x86_64
    CMAKE_EXT=tar.gz
elif [[ $KERNEL == *Msys* ]]; then
    CMAKE_PLATFORM=windows-x86_64
    CMAKE_EXT=zip
fi

echo "Downloading cmake..."
wget -c -q https://github.com/Kitware/CMake/releases/download/v${CMAKE_RELEASE}/cmake-${CMAKE_RELEASE}-${CMAKE_PLATFORM}.${CMAKE_EXT}


echo "Decompressing archive..."
if [[ $KERNEL != *Msys* ]]; then
    echo "untarring cmake"
    tar -xf cmake-${CMAKE_RELEASE}-${CMAKE_PLATFORM}.${CMAKE_EXT}
else
    echo "unzip cmake"
    unzip -o cmake-${CMAKE_RELEASE}-${CMAKE_PLATFORM}.${CMAKE_EXT}
fi


if [[ $KERNEL != *Darwin* ]]; then
    dir=cmake-${CMAKE_RELEASE}-${CMAKE_PLATFORM}
    mv -f ${dir}/* $PWD/$BUILD_DIR/install/
elif [[ $KERNEL == *Darwin* ]]; then
    dir=cmake-${CMAKE_RELEASE}-${CMAKE_PLATFORM}/CMake.app/Contents/
    mv -f ${dir}/* $PWD/$BUILD_DIR/install/
fi


echo "Cleaning up..."
rm -rf cmake-${CMAKE_RELEASE}-${CMAKE_PLATFORM}*


echo "Checking executable is there:"
ls  $BUILD_DIR/install/bin
echo "Done with installing cmake"

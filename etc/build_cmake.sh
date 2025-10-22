#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


if [[ !$RUNME ]]; then
    . ./etc/build_dir.sh
fi

root=$PWD

if [[ ! -d $root/$BUILD_DIR/install ]]; then
    mkdir -p $root/$BUILD_DIR/install
fi

if [[ -e $root/$BUILD_DIR/install/bin/cmake ]]; then
    return
fi

mkdir -p $root/$BUILD_DIR/deps

cd $root/$BUILD_DIR/deps
git clone https://gitlab.kitware.com/cmake/cmake.git

#
# Working checkout, instead of master
#
cd cmake
git checkout 55c6b79da6e16b1683c8b2339ad4a60133e93816

mkdir -p _build
cd _build
cmake .. -G Ninja -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=$root/$BUILD_DIR/install
ninja
ninja install
cd ../..

#
# Clean up cmake repository and build
#
cd $root/$BUILD_DIR/deps
rm -rf cmake

echo "Checking executable is there:"
ls  $root/$BUILD_DIR/install/bin
echo "Done with building cmake"

cd $root

#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Script used to install compiling tools on windows through Msys.
#

if [[ ! $RUNME ]]; then
    . etc/build_dir.sh
else
    . etc/functions.sh
fi

echo "Installing swig, diffutils thru Msys..."
pacman -Syu --noconfirm

#
# Install 
#
programs="swig diffutils"
pacman -Sy swig diffutils --noconfirm

if [[ $ARCH == *amd64* ]]; then
    echo "Installing nasm and perl thru Msys2 x86_64..."
    programs="$programs nasm perl"
fi

pacman -Sy $programs --noconfirm

mkdir -p $BUILD_DIR/install/bin
mkdir -p $BUILD_DIR/install/lib
mkdir -p $BUILD_DIR/install/include

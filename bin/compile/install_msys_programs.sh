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

echo "Installing swig, diffutils, nasm and gettext thru Msys..."
pacman -Syu --noconfirm

#
# Install 
#
pacman -Sy swig diffutils --noconfirm

if [[ $ARCH == *amd64* ]]; then
    pacman -Sy mingw-w64-ucrt-x86_64-gettext --noconfirm
    pacman -Sy nasm --noconfirm
else
    pacman -Sy mingw-w64-clang-aarch64-binutils
    echo "------------------------ AS version:"
    as --version
fi

mkdir -p $BUILD_DIR/install/bin
mkdir -p $BUILD_DIR/install/lib
mkdir -p $BUILD_DIR/install/include

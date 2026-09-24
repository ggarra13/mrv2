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
if [[ "$MSYS2_INSTALL" == "ON" ]]; then
    programs="diffutils"
    if command -v swig >/dev/null 2>&1; then
	echo "swig already installed"
    else
	programs="$programs swig"
    fi

    if [[ $ARCH == *amd64* ]]; then
	if command -v nasm >/dev/null 2>&1; then
	    echo "nasm already installed"
	else
	    programs="$programs nasm"
	fi
	
	if command -v perl >/dev/null 2>&1; then
	    echo "perl already installed"
	else
	    programs="$programs perl"
	fi
    fi

    echo "Installing $programs"
    pacman -S --needed --noconfirm $programs
fi

mkdir -p $BUILD_DIR/install/bin
mkdir -p $BUILD_DIR/install/lib
mkdir -p $BUILD_DIR/install/include

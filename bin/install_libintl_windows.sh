#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/bin/bash

#
# Script used to install libiconv and libintl on windows through Msys.
#

. etc/build_dir.sh

pacman -Sy --noconfirm

echo "Running install_libintl_windows.sh"

#
# Install gettext
#
if [[ ! -e /mingw64/bin/gettext.exe ]]; then
    pacman -Sy mingw-w64-x86_64-gettext --noconfirm
fi

mkdir -p $BUILD_DIR/install/bin
mkdir -p $BUILD_DIR/install/lib
mkdir -p $BUILD_DIR/install/include

#
# Install libiconv
#
if [[ ! -e $BUILD_DIR/install/lib/libiconv.lib ]]; then
    if [[ ! -e /mingw64/lib/libiconv.dll.a ]]; then
	pacman -Sy mingw-w64-x86_64-libiconv --noconfirm
    fi
    cp /mingw64/bin/libiconv*.dll $BUILD_DIR/install/bin/
    cp /mingw64/lib/libiconv.dll.a $BUILD_DIR/install/lib/libiconv.lib
    cp /mingw64/include/iconv.h $BUILD_DIR/install/include/
fi

#
# Install libintl
#
if [[ ! -e $BUILD_DIR/install/lib/libintl.lib ]]; then
    if [[ ! -e /mingw64/lib/libintl.dll.a ]]; then
	pacman -Sy mingw-w64-x86_64-libintl --noconfirm
    fi
    cp /mingw64/bin/libintl*.dll $BUILD_DIR/install/bin/
    cp /mingw64/lib/libintl.dll.a $BUILD_DIR/install/lib/libintl.lib
    cp /mingw64/include/libintl.h $BUILD_DIR/install/include/
fi

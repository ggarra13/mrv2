#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/bin/bash

#
# Script used to install libiconv and libintl on windows through Msys.
#

if [[ ! $RUNME ]]; then
    . etc/build_dir.sh
else
    . etc/functions.sh
fi


run_pacman=
if [[ ! -e /mingw64/bin/gettext.exe ||
	  ! -e /mingw64/lib/libiconv.dll.a ||
	  ! -e /mingw64/lib/libintl.dll.a ]]; then
    echo "Installing libssl, libiconv, libintl, subversion, swig and gettext thru Msys..."
    run_pacman=1
    pacman -Syu --noconfirm
fi


#
# Install git (breaks BuildpyFLTK)
#
# if [[ ! -e /mingw64/bin/git.exe ]]; then
#     pacman -Sy git --noconfirm
# fi

#
# Install subversion
#
pacman -Sy libsqlite  --noconfirm
pacman -Sy subversion --noconfirm

#
# Install swig
#
pacman -Sy swig --noconfirm

#
# Install gettext
#
pacman -Sy mingw-w64-x86_64-gettext --noconfirm

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

    run_cmd cp /mingw64/bin/libiconv*.dll $BUILD_DIR/install/bin/
    run_cmd cp /mingw64/lib/libiconv.dll.a $BUILD_DIR/install/lib/libiconv.lib

    run_cmd cp /mingw64/include/iconv.h $BUILD_DIR/install/include/
fi

#
# Install libintl
#
if [[ ! -e $BUILD_DIR/install/lib/libintl.lib ]]; then
    if [[ ! -e /mingw64/lib/libintl.dll.a ]]; then
	pacman -Sy mingw-w64-x86_64-libintl --noconfirm
    fi

    run_cmd cp /mingw64/bin/libintl*.dll $BUILD_DIR/install/bin/
    run_cmd cp /mingw64/lib/libintl.dll.a $BUILD_DIR/install/lib/libintl.lib

    run_cmd cp /mingw64/include/libintl.h $BUILD_DIR/install/include/
fi

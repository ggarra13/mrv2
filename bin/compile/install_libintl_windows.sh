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

if [[ "${BUILD_GETTEXT}" != "OFF" || "${BUILD_GETTEXT}" != "0" ]]; then

    if [[ ! -e /ucrt64/bin/gettext.exe ||
	      ! -e /ucrt64/lib/libiconv.dll.a ||
	      ! -e /ucrt64/lib/libintl.dll.a ]]; then
	echo "Installing libssl, libsqlite, libiconv, libintl thru Msys..."
	pacman -Syu --noconfirm
    fi

    #
    # Install 
    #
    pacman -Sy libsqlite --noconfirm

    mkdir -p $BUILD_DIR/install/bin
    mkdir -p $BUILD_DIR/install/lib
    mkdir -p $BUILD_DIR/install/include

    #
    # Install libiconv
    #
    if [[ ! -e $BUILD_DIR/install/lib/libiconv.lib ]]; then
	if [[ ! -e /ucrt64/lib/libiconv.dll.a ]]; then
	    pacman -Sy mingw-w64-ucrt-x86_64-libiconv --noconfirm
	fi

	run_cmd cp /ucrt64/bin/libiconv*.dll $BUILD_DIR/install/bin/
	run_cmd cp /ucrt64/lib/libiconv.dll.a $BUILD_DIR/install/lib/libiconv.lib

	run_cmd cp /ucrt64/include/iconv.h $BUILD_DIR/install/include/
    fi

    #
    # Install libintl
    #
    if [[ ! -e $BUILD_DIR/install/lib/libintl.lib ]]; then
	if [[ ! -e /ucrt64/lib/libintl.dll.a ]]; then
	    pacman -Sy mingw-w64-ucrt-x86_64-libintl --noconfirm
	fi

	run_cmd cp /ucrt64/bin/libintl*.dll $BUILD_DIR/install/bin/
	run_cmd cp /ucrt64/lib/libintl.dll.a $BUILD_DIR/install/lib/libintl.lib

	run_cmd cp /ucrt64/include/libintl.h $BUILD_DIR/install/include/
    fi
fi

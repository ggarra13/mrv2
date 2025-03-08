#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script compiles a GPL version of libx264.
# And, if TLRENDER_NET is on, downloads openssl and libcrypto from MSys2
# and copies them so they can be used by FFmpeg on Windows MSys2 build.
#
if [[ ! -e etc/build_dir.sh ]]; then
    echo "You must run this script from the root of mrv2 directory like:"
    echo
    script=`basename $0`
    echo "> bin/$script"
    exit 1
fi

if [[ ! $RUNME ]]; then
    . etc/build_dir.sh
else
    . etc/functions.sh
    
    get_msvc_version
fi

if [[ $KERNEL != *Msys* ]]; then
    echo
    echo "This script is for Windows MSys2-64 only."
    echo
    exit 1
fi

#
# Latest TAGS of all libraries
#
X264_TAG=stable


#
# Repositories
#
LIBX264_REPO=https://code.videolan.org/videolan/x264.git

#
# Some auxiliary variables
#
MRV2_ROOT=$PWD
ROOT_DIR=$PWD/$BUILD_DIR/tlRender/etc/SuperBuild/FFmpeg
INSTALL_DIR=$PWD/$BUILD_DIR/install
    

if [ -z "$TLRENDER_NET" ]; then
    export TLRENDER_NET=ON
fi


#
# Get Msys dependencies
#
pacman -Sy make diffutils nasm --noconfirm


#
# Build with h264 encoding.
#
TLRENDER_X264=ON
if [[ $FFMPEG_GPL == LGPL ]]; then
    TLRENDER_X264=OFF
fi

if [[ $TLRENDER_FFMPEG == OFF || $TLRENDER_FFMPEG == 0 ]]; then
    export TLRENDER_VPX=OFF
    export TLRENDER_AV1=OFF
    export TLRENDER_NET=OFF
    export TLRENDER_X264=OFF
else
    echo
    echo "Installing packages needed to build:"
    echo
    if [[ $TLRENDER_X264 == ON || $TLRENDER_X264 == 1 ]]; then
	echo "libx264"
    fi
    echo
fi



#############
## BUILDING #
#############

#
# Build x264
#
ENABLE_LIBX264=""
if [[ $TLRENDER_X264 == ON || $TLRENDER_X264 == 1 ]]; then
    
    mkdir -p $ROOT_DIR

    cd    $ROOT_DIR
    
    mkdir -p sources
    mkdir -p build


    cd $ROOT_DIR/sources

    if [[ ! -d x264 ]]; then
	git clone ${LIBX264_REPO} --branch ${X264_TAG}
    fi

    if [[ ! -e $INSTALL_DIR/lib/libx264.lib ]]; then
	echo
	echo "Compiling libx264 as GPL......"
	echo
	cd $ROOT_DIR/build
	mkdir -p x264
	cd x264
	CC=cl CXX=cl LD=link ./../../sources/x264/configure --prefix=$INSTALL_DIR --enable-shared
	make -j ${CPU_CORES}
	make install
	run_cmd mv $INSTALL_DIR/lib/libx264.dll.lib $INSTALL_DIR/lib/libx264.lib
    fi
    
    ENABLE_LIBX264="--enable-libx264 
                    --enable-decoder=libx264
                    --enable-encoder=libx264
                    --enable-gpl"
else
    # Remove unused libx264
    if [[ -e $INSTALL_DIR/lib/libx264.lib ]]; then
	run_cmd rm -f $INSTALL_DIR/bin/libx264*.dll
	run_cmd rm -f $INSTALL_DIR/lib/libx264.lib
    fi
fi


#
# Install openssl and libcrypto
#
if [[ $TLRENDER_NET == ON || $TLRENDER_NET == 1 ]]; then
    if [[ ! -e $BUILD_DIR/install/lib/ssl.lib ]]; then
	
	if [[ ! -e /mingw64/lib/libssl.dll.a ]]; then
	    pacman -Sy mingw-w64-x86_64-openssl --noconfirm
	fi

	run_cmd cp /mingw64/bin/libssl*.dll $INSTALL_DIR/bin/
	run_cmd cp /mingw64/lib/libssl.dll.a $INSTALL_DIR/lib/ssl.lib
	run_cmd cp /mingw64/bin/libcrypto*.dll $INSTALL_DIR/bin/
	run_cmd cp /mingw64/lib/libcrypto.dll.a $INSTALL_DIR/lib/crypto.lib
	run_cmd mkdir -p $INSTALL_DIR/lib/pkgconfig
	run_cmd cp /mingw64/lib/pkgconfig/libssl.pc $INSTALL_DIR/lib/pkgconfig/openssl.pc

	run_cmd cp -r /mingw64/include/openssl $INSTALL_DIR/include/
	run_cmd sed -i -e 's/SSL_library_init../SSL_library_init/' $INSTALL_DIR/include/openssl/ssl.h
	run_cmd sed -i -e "s#=/mingw64#=$INSTALL_DIR#" $INSTALL_DIR/lib/pkgconfig/openssl.pc
	run_cmd sed -i -e 's%Requires.private:.libcrypto%%' $INSTALL_DIR/lib/pkgconfig/openssl.pc
	
	run_cmd cp /mingw64/lib/pkgconfig/libcrypto.pc $INSTALL_DIR/lib/pkgconfig/
	run_cmd sed -i -e "s#=/mingw64#=$INSTALL_DIR#" $INSTALL_DIR/lib/pkgconfig/libcrypto.pc
    fi
fi

echo
echo "Removing packages used to build:"
echo
if [[ $TLRENDER_X264 == ON || $TLRENDER_X264 == 1 ]]; then
    echo "libx264"
fi

cd $MRV2_ROOT

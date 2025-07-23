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
# Some auxiliary variables
#
MRV2_ROOT=$PWD
INSTALL_DIR=$PWD/$BUILD_DIR/install
    

if [ -z "$TLRENDER_NET" ]; then
    export TLRENDER_NET=ON
fi


#############
## BUILDING #
#############


#
# Install openssl and libcrypto
#
if [[ $TLRENDER_NET == ON || $TLRENDER_NET == 1 ]]; then
    if [[ ! -e $BUILD_DIR/install/lib/ssl.lib ]]; then
	
	if [[ ! -e /ucrt64/lib/libssl.dll.a ]]; then
	    pacman -Sy mingw-w64-ucrt-x86_64-openssl --noconfirm
	fi
	

	run_cmd cp /ucrt64/bin/libssl*.dll $INSTALL_DIR/bin/
	run_cmd cp /ucrt64/lib/libssl.dll.a $INSTALL_DIR/lib/ssl.lib
	run_cmd cp /ucrt64/bin/libcrypto*.dll $INSTALL_DIR/bin/
	run_cmd cp /ucrt64/lib/libcrypto.dll.a $INSTALL_DIR/lib/crypto.lib
	run_cmd mkdir -p $INSTALL_DIR/lib/pkgconfig
	run_cmd cp /ucrt64/lib/pkgconfig/libssl.pc $INSTALL_DIR/lib/pkgconfig/openssl.pc

	
	run_cmd cp -r /ucrt64/include/openssl $INSTALL_DIR/include/
	run_cmd sed -i -e 's/SSL_library_init../SSL_library_init/' $INSTALL_DIR/include/openssl/ssl.h
	run_cmd sed -i -e "s#=/ucrt64#=$INSTALL_DIR#" $INSTALL_DIR/lib/pkgconfig/openssl.pc
	run_cmd sed -i -e 's%Requires.private:.libcrypto%%' $INSTALL_DIR/lib/pkgconfig/openssl.pc
	
	run_cmd cp /ucrt64/lib/pkgconfig/libcrypto.pc $INSTALL_DIR/lib/pkgconfig/
	run_cmd sed -i -e "s#=/ucrt64#=$INSTALL_DIR#" $INSTALL_DIR/lib/pkgconfig/libcrypto.pc
    fi
fi

cd $MRV2_ROOT

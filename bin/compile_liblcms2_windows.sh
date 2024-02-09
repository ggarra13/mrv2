#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script compiles a LCMS2 as a shared library (DLL).
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
fi

#
# Set the main tag to compile
#
LCMS_BRANCH=lcms2.16

if [[ $KERNEL != *Msys* ]]; then
    echo
    echo "This script is for Windows MSys2-64 only."
    echo
    exit 1
fi

MRV2_ROOT=$PWD
SUPERBUILD=$PWD/$BUILD_DIR/tlRender/etc/SuperBuild
INSTALLDIR=$PWD/$BUILD_DIR/install

if [[ ! -e $INSTALLDIR/lib/liblcms2.lib ]]; then

    #
    # Install development tools
    #
    pacman -Sy --noconfirm

    pacman -Sy binutils --noconfirm
    
    #
    # Clone the repository
    #
    mkdir -p $SUPERBUILD
    cd $SUPERBUILD
    if [[ ! -d LCMS2 ]]; then
	run_cmd git clone --depth 1 --branch $LCMS_BRANCH https://github.com/mm2/Little-CMS.git LCMS2 2> /dev/null
    fi
    
    #
    # Run configure
    #
    cd LCMS2
    export CC=cl
    export CXX=cl
    export LD=/usr/bin/ld
    ./configure --enable-shared --disable-static --prefix=$INSTALLDIR
    
    #
    # Compile and install the library
    #
    make -j ${CPU_CORES} install

    echo "Compiled result:"
    ls $INSTALLDIR/lib/liblcms2*
    
    run_cmd mv $INSTALLDIR/lib/liblcms2.a $INSTALLDIR/lib/liblcms2.lib
    
    cd $MRV2_ROOT
else
    echo "liblcms2 already installed."
fi

#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script compiles a GPL or LGPL version of ffmpeg. The GPL version has
# libx264 encoding and libvpx support.  The LGPL version does not have libx264.
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
fi

#
# Set the main tag to compile
#
export LCMS_BRANCH=lcms2.15

if [[ $KERNEL != *Msys* ]]; then
    echo
    echo "This script is for Windows MSys2-64 only."
    echo
    exit 1
fi

superbuild=$PWD/$BUILD_DIR/tlRender/etc/SuperBuild
installdir=$PWD/$BUILD_DIR/install
lcms2dir=$superbuild/LCMS2

mkdir -p $superbuild
mkdir -p $installdir

rm -rf $lcms2dir

#
# Install development tools
#
pacman -Sy --noconfirm
pacman -Sy base-devel --noconfirm

#
# Clone the repository
#
cd $superbuild
git clone --depth 1 --branch $LCMS_BRANCH https://github.com/mm2/Little-CMS.git LCMS2 2> /dev/null

#
# Run configure
#
cd $lcms2dir
./configure --enable-shared --disable-static --prefix=$installdir

#
# Compile and install the library
#
make -j ${CPU_CORES} install

mv $installdir/lib/liblcms2.a $installdir/lib/liblcms2.lib



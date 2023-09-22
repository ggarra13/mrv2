#!/bin/bash

. etc/build_dir.sh

#
# Set the main tag to compile
#
export LCMS_BRANCH=lcms2.15


superbuild=$PWD/$BUILD_DIR/tlRender/etc/SuperBuild
installdir=$PWD/$BUILD_DIR/install
lcms2dir=$superbuild/LCMS2

mkdir -p $superbuild
mkdir -p $installdir

rm -rf $lcms2dir

#
# Clone the repository
#
cd $superbuild
git clone --depth 1 --branch $LCMS_BRANCH https://github.com/mm2/Little-CMS.git LCMS2 2> /dev/null

#
# Set the path to point to gcc and ld
#
export PATH="/mingw64/bin:$PATH"

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

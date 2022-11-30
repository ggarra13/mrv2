#!/usr/bin/env bash

#
#
# Main build script for mrv2.  It builds all dependencies and will install the
# main executable on BUILD-OS-ARCH/BUILD_TYPE/install/bin.
#
# On Linux and macOS, it will also create a mrv2 or mrv2-dbg symbolic link
# in $HOME/bin if the directory exists.
#
#

. $PWD/aux/build_dir.sh

cd $BUILD_DIR

cmake ../.. -G Ninja -D CMAKE_VERBOSE_MAKEFILE=ON -D CMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -D CMAKE_INSTALL_PREFIX=$PWD/install -D CMAKE_PREFIX_PATH=$PWD/install -D TLRENDER_PROGRAMS=OFF -D TLRENDER_EXAMPLES=FALSE -D TLRENDER_TESTS=FALSE -D TLRENDER_QT6=OFF -D TLRENDER_QT5=OFF -D LIBINTL_ROOT=${LIBINTL_ROOT}

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE

cd -

. $PWD/aux/build_end.sh

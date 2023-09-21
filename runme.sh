#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
#
# Main build script for mrv2.  It builds all dependencies and will install the
# main executable on BUILD-OS-ARCH/BUILD_TYPE/install/bin.
#
# On Linux and macOS, it will also create a mrv2 or mrv2-dbg symbolic link
# in $HOME/bin if the directory exists.
#
#

. $PWD/etc/build_dir.sh

cd $BUILD_DIR

export TLRENDER_USD=ON

cmd="cmake -G '${CMAKE_GENERATOR}' -D CMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -D CMAKE_INSTALL_PREFIX=$PWD/install -D CMAKE_PREFIX_PATH=$PWD/install -D TLRENDER_USD=${TLRENDER_USD} -D TLRENDER_RAW=ON -D TLRENDER_NFD=OFF -D TLRENDER_PROGRAMS=OFF -D TLRENDER_EXAMPLES=FALSE -D TLRENDER_TESTS=FALSE -D TLRENDER_QT6=OFF -D TLRENDER_QT5=OFF ${CMAKE_FLAGS} ../.."

run_cmd $cmd

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE

cd -

cmd="./runmeq.sh ${CMAKE_BUILD_TYPE} -t install"
run_cmd $cmd

if [[ $CMAKE_TARGET == "package" ]]; then
    cmd="./runmeq.sh ${CMAKE_BUILD_TYPE} -t package"
    run_cmd $cmd
else
    . $PWD/etc/build_end.sh
fi


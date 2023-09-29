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

if [[ !$RUNME ]]; then
    . $PWD/etc/build_dir.sh
fi

sleep 10

cd $BUILD_DIR

export TLRENDER_USD=ON

cmd="cmake -G '${CMAKE_GENERATOR}' -D CMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -D CMAKE_INSTALL_PREFIX=$PWD/install -D CMAKE_PREFIX_PATH=$PWD/install -D TLRENDER_USD=${TLRENDER_USD} -D TLRENDER_RAW=ON -D TLRENDER_NFD=OFF -D TLRENDER_PROGRAMS=OFF -D TLRENDER_EXAMPLES=FALSE -D TLRENDER_TESTS=FALSE -D TLRENDER_QT6=OFF -D TLRENDER_QT5=OFF ${CMAKE_FLAGS} ../.."

run_cmd $cmd

run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE

cd -

if [[ "$CMAKE_TARGET" == "" ]]; then
    CMAKE_TARGET=install
fi

cmd="./runmeq.sh ${CMAKE_BUILD_TYPE} -t ${CMAKE_TARGET}"
run_cmd $cmd

. $PWD/etc/build_end.sh


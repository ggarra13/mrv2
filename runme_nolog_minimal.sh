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
# This script does *NOT* save a log to  BUILD-OS-ARCH/BUILD_TYPE/compile.log.
# Use runme.sh for that.
#
#

if [[ !$RUNME ]]; then
    . $PWD/etc/build_dir.sh
fi

sleep 10

cd $BUILD_DIR

#
# These are some of the expensive mrv2 options
#
export BUILD_PYTHON=ON
export MRV2_PYFLTK=ON
export MRV2_PYBIND11=ON
export MRV2_NETWORK=ON
export MRV2_PDF=ON

#
# These are some of the expensive TLRENDER options
#

# asan memory debugging
if [[ $CMAKE_BUILD_TYPE == "*Debug*" ]]; then
    export TLRENDER_ASAN=ON
else
    export TLRENDER_ASAN=OFF
fi
export TLRENDER_NET=OFF
export TLRENDER_RAW=OFF
export TLRENDER_USD=OFF
export TLRENDER_VPX=ON
export TLRENDER_WAYLAND=ON
export TLRENDER_YASM=ON

cmd="cmake -G '${CMAKE_GENERATOR}' \
	   -D CMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
	   -D CMAKE_INSTALL_PREFIX=$PWD/install \
	   -D CMAKE_PREFIX_PATH=$PWD/install \
	   -D BUILD_PYTHON=${BUILD_PYTHON} \
	   -D MRV2_PYFLTK=${MRV2_PYFLTK} \
	   -D MRV2_PYBIND11=${MRV2_PYBIND11} \
	   -D MRV2_PDF=${MRV2_PDF} \
	   -D TLRENDER_USD=${TLRENDER_USD} \
	   -D TLRENDER_VPX=${TLRENDER_VPX} \
	   -D TLRENDER_YASM=${TLRENDER_YASM} \
	   -D TLRENDER_RAW=${TLRENDER_RAW} \
	   -D TLRENDER_WAYLAND=${TLRENDER_WAYLAND} \
	   -D TLRENDER_NET=${TLRENDER_NET} \
	   -D TLRENDER_NFD=OFF \
	   -D TLRENDER_PROGRAMS=OFF \
	   -D TLRENDER_EXAMPLES=FALSE \
	   -D TLRENDER_TESTS=FALSE \
	   -D TLRENDER_QT6=OFF \
	   -D TLRENDER_QT5=OFF \
	   ${CMAKE_FLAGS} ../.."

run_cmd $cmd

run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE

cd -

if [[ "$CMAKE_TARGET" == "" ]]; then
    CMAKE_TARGET=install
fi

cmd="./runmeq.sh ${CMAKE_BUILD_TYPE} -t ${CMAKE_TARGET}"
run_cmd $cmd

if [[ "$CMAKE_TARGET" != "package" ]]; then
    . $PWD/etc/build_end.sh
fi


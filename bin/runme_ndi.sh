#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
#
# Minimal build script for mrv2.  It builds all dependencies and will install
# the main executable on BUILD_DIR (by default
#                                   BUILD-OS-ARCH/BUILD_TYPE/install/bin).
#
# On Linux and macOS, it will also create a mrv2 or mrv2-dbg symbolic link
# in $HOME/bin if the directory exists.
#
# It will also log the compilation on $BUILD_DIR/compile.log
#


#
# Store the parameters for passing them later
#
params="$@"

if [[ $params != *"-ndi"* ]]; then
    echo "Please pass the -ndi flag to this script"
    exit 1
fi

#
# Find out our build dir
#
. etc/build_dir.sh

mkdir -p $BUILD_DIR


#
# Clear the flags, as they will be set by runme_nolog.sh.
#
export FLAGS=""
export CMAKE_FLAGS=""


echo
echo "Saving compile log to $BUILD_DIR/compile.log ..."

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

export TLRENDER_ASAN=OFF # asan memory debugging (not yet working)
export TLRENDER_NET=ON
export TLRENDER_RAW=ON
export TLRENDER_USD=ON
export TLRENDER_VPX=ON
export TLRENDER_WAYLAND=ON
export TLRENDER_YASM=ON

if [[ $KERNEL == *Linux* ]]; then
    NDI_SDK="/home/gga/code/lib/NDI_SDK_v5_Linux/NDI_SDK_for_Linux/"
elif [[ $KERNEL == *Msys* ]]; then
    NDI_SDK="C:/Program\ Files/NDI/NDI\ 5\ SDK/"
else
    NDI_SDK="/Library/NDI\ SDK\ for\ Apple/"
fi

cmd="./runme_nolog.sh 
	   -D BUILD_PYTHON=${BUILD_PYTHON} \
	   -D MRV2_PYFLTK=${MRV2_PYFLTK} \
	   -D MRV2_PYBIND11=${MRV2_PYBIND11} \
	   -D MRV2_NETWORK=${MRV2_NETWORK} \
	   -D MRV2_PDF=${MRV2_PDF} \
           -D TLRENDER_FFMPEG=ON \
           -D TLRENDER_NDI=ON \
           -D TLRENDER_NDI_SDK='${NDI_SDK}' \
	   -D TLRENDER_NET=${TLRENDER_NET} \
	   -D TLRENDER_NFD=OFF \
	   -D TLRENDER_PROGRAMS=OFF \
	   -D TLRENDER_EXAMPLES=FALSE \
	   -D TLRENDER_TESTS=FALSE \
	   -D TLRENDER_YASM=${TLRENDER_YASM} \
	   -D TLRENDER_QT6=OFF \
	   -D TLRENDER_QT5=OFF \
	   -D TLRENDER_RAW=${TLRENDER_RAW} \
	   -D TLRENDER_USD=${TLRENDER_USD} \
	   -D TLRENDER_VPX=${TLRENDER_VPX} \
	   -D TLRENDER_WAYLAND=${TLRENDER_WAYLAND} \
            $params 2>&1 | tee $BUILD_DIR/compile.log"
run_cmd $cmd

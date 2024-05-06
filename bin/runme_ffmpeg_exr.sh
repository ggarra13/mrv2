#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
#
# Small build script for mrv2.  Run it from the root of the mrv2 dir, like:
#
# ./bin/runme_small.sh
#
# It builds all dependencies and will install
# the main executable on BUILD_DIR (by default
#                                   BUILD-OS-ARCH/BUILD_TYPE/install/bin).
#
# The small build does not have Python, USD, PDF, NDI or NETWORK support.
# It is intended for solo artists.
# 
#
# On Linux and macOS, it will also create a mrv2 or mrv2-dbg symbolic link
# in $HOME/bin if the directory exists.
#
# It will also log the compilation on $BUILD_DIR/compile.log
#


#
# Store the parameters for passing them later
#
params=$*

#
# Find out our build dir
#
. etc/build_dir.sh


#
# Clear the flags, as they will be set by runme_nolog.sh.
#
export FLAGS=""
export CMAKE_FLAGS=""

#
# Common flags
#
export BUILD_PYTHON=OFF

#
# These are some of the expensive mrv2 options
#
export BUILD_PYTHON=OFF
export MRV2_PYFLTK=OFF
export MRV2_PYBIND11=OFF
export MRV2_NETWORK=OFF
export MRV2_PDF=OFF

#
# These are some of the expensive TLRENDER options
#
export TLRENDER_AV1=OFF
export TLRENDER_FFMPEG=ON
export TLRENDER_FFMPEG_MINIMAL=ON
export TLRENDER_EXR=ON
export TLRENDER_JPEG=OFF
export TLRENDER_NDI=OFF
export TLRENDER_NET=OFF
export TLRENDER_RAW=OFF
export TLRENDER_SGI=OFF
export TLRENDER_STB=OFF
export TLRENDER_TIFF=OFF
export TLRENDER_USD=OFF
export TLRENDER_VPX=OFF
export TLRENDER_WAYLAND=ON
export TLRENDER_X11=ON
export TLRENDER_YASM=ON

echo
echo "Saving compile log to $BUILD_DIR/compile.log ..."
echo
cmd="./runme_nolog.sh --ask $params 2>&1 | tee $BUILD_DIR/compile.log"
run_cmd $cmd

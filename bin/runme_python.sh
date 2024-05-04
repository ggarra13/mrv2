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
export BUILD_PYTHON=ON

#
# These are some of the expensive mrv2 options
#
export MRV2_PYFLTK=ON
export MRV2_PYBIND11=ON
export MRV2_NETWORK=ON
export MRV2_PDF=ON

#
# These are some of the expensive TLRENDER options
#
export TLRENDER_AV1=ON
export TLRENDER_FFMPEG=ON
export TLRENDER_FFMPEG_MINIMAL=ON
export TLRENDER_EXR=ON
export TLRENDER_JPEG=ON
export TLRENDER_NDI=ON
export TLRENDER_NET=ON
export TLRENDER_RAW=ON
export TLRENDER_SGI=ON
export TLRENDER_STB=ON
export TLRENDER_TIFF=ON
export TLRENDER_USD=OFF
export TLRENDER_VPX=ON
export TLRENDER_WAYLAND=ON
export TLRENDER_X11=ON
export TLRENDER_YASM=ON

echo
echo "Saving compile log to $BUILD_DIR/compile.log ..."
echo
cmd="./runme_nolog.sh --ask $params 2>&1 | tee $BUILD_DIR/compile.log"
run_cmd $cmd

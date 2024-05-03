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

export build_root=BUILD-$KERNEL-$ARCH-python
export BUILD_DIR=${build_root}/${BUILD_TYPE_DIR}
mkdir -p $BUILD_DIR


#
# Clear the flags, as they will be set by runme_nolog.sh.
#
export FLAGS=""
export CMAKE_FLAGS=""
unset PYTHONEXE
unset PYTHON
unset PYTHONDIR
unset PYTHON_SITEDIR
unset PYTHON_VERSION


#
# These are some of the expensive mrv2 options
#
export BUILD_PYTHON=OFF
export MRV2_PYFLTK=OFF
export MRV2_PYBIND11=ON
export MRV2_NETWORK=OFF
export MRV2_PDF=OFF
export MRV2_PYTHON=/usr/bin/python3.11

#
# These are some of the expensive TLRENDER options
#
export TLRENDER_AV1=OFF
export TLRENDER_FFMPEG=OFF
export TLRENDER_FFMPEG_MINIMAL=OFF
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
export TLRENDER_YASM=OFF

echo
echo "Saving compile log to $BUILD_DIR/compile.log ..."
echo
cmd="./runme_nolog.sh --build-dir ${build_root} --ask $params 2>&1 | tee $BUILD_DIR/compile.log"
run_cmd $cmd

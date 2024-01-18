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

export TLRENDER_NDI=ON

if [[ $KERNEL == *Linux* ]]; then
    export TLRENDER_NDI_SDK="/home/gga/code/lib/NDI\ SDK\ for\ Linux/"
elif [[ $KERNEL == *Msys* ]]; then
    export TLRENDER_NDI_SDK="C:/Program\ Files/NDI/NDI\ 5\ SDK/"
else
    export TLRENDER_NDI_SDK="/Library/NDI\ SDK\ for\ Apple/"
fi

cmd="./runme_nolog.sh $params 2>&1 | tee $BUILD_DIR/compile.log"
run_cmd $cmd

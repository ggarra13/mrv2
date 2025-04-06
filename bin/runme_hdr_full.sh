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

KERNEL=`uname`
if [[ $KERNEL == *MSYS* || $KERNEL == *MINGW* ]]; then
    export KERNEL=Msys
fi
export BUILD_ROOT=${KERNEL}-vulkan

#
# Find out our build dir
#
. etc/build_dir.sh


#
# Clear the flags, as they will be set by runme_nolog.sh.
#
export FLAGS=""
export CMAKE_FLAGS=""


export MRV2_HDR=ON

echo
echo "Saving compile log to $BUILD_DIR/compile.log ..."
echo
cmd="./etc/runme_nolog.sh --ask $params -D MRV2_HDR=ON --dir $KERNEL-vulkan 2>&1 | tee $BUILD_DIR/compile.log"
run_cmd $cmd

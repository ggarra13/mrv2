#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
#
# Main build script for mrv2.  It builds all dependencies and will install the
# main executable on BUILD_DIR (by default
#                               BUILD-OS-ARCH/BUILD_TYPE/install/bin).
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

export BUILD_ROOT=BUILD-$KERNEL-$ARCH-usd

cmd="runmeq.sh $params --dir $BUILD_ROOT"
run_cmd $cmd

#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# Minimal build script for mrv2.  Run it from the root of the mrv2 dir, like:
#
# ./bin/runme_minimal.sh
#
# Intended to make sure cmake files, FLTK and mrv2 compile with no options.
# You cannot open any image or movie file with this.
#


#
# Store the parameters for passing them later
#
params=$*

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

. bin/options_off.sh

export TLRENDER_WAYLAND=ON
export TLRENDER_X11=ON

echo
echo "Saving compile log to $BUILD_DIR/compile.log ..."
echo
cmd="./etc/runme_nolog.sh --ask $params 2>&1 | tee $BUILD_DIR/compile.log"
run_cmd $cmd

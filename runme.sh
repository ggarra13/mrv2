#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

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


echo
echo "Saving compile log to $BUILD_DIR/compile.log ..."
cmd="./runme_nolog.sh $params 2>&1 | tee $BUILD_DIR/compile.log"
run_cmd $cmd

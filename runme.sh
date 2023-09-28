#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. etc/build_dir.sh

echo
echo "Saving compile log to $BUILD_DIR/compile.log ..."
cmd="./runme_nolog.sh 2>&1 | tee $BUILD_DIR/compile.log"
run_cmd $cmd

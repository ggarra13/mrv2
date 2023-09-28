#!/usr/bin/env bash

. etc/build_dir.sh $*

echo
echo "Saving compile log to $BUILD_DIR/compile.log ..."
echo
cmd="./runme_nolog.sh $FLAGS 2>&1 | tee $BUILD_DIR/compile.log"
run_cmd $cmd

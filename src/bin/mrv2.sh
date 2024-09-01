#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/bin/bash

if [[ $BASH_SOURCE != $0 ]]; then
    dir=$BASH_SOURCE
else
    dir=$0
fi


symlink=`readlink ${dir}`

if [ "$symlink" != "" ]; then
    dir="$symlink"
    dir=${dir%/*}
else
# Find directory where mrv2 bash script resides
    dir=${dir%/*}
fi

# If running from current directory, get full path
if [ "$dir" == '.' ]; then
    dir=$PWD
fi

source "$dir/environment.sh"

params=""

for param in "$@"
do
  params="${params} \"${param}\""
done


#
# Start up mrv2
#
sh -c "${dir}/bin/mrv2 $params"

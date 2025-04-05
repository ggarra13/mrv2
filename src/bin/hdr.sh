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


KERNEL=`uname`

if [[ $KERNEL == *Darwin* ]]; then
    if [ -z $VK_LAYER_PATH ]; then
	export VK_LAYER_PATH=/usr/local/opt/vulkan-profiles/share/vulkan/explicit_layer.d:/usr/local/opt/vulkan-validationlayers/share/vulkan/explicit_layer.d
    fi
    if [ -z $VK_ICD_FILENAMES ]; then
	export VK_ICD_FILENAMES=/usr/local/etc/vulkan/icd.d/MoltenVK_icd.json
    fi
fi

#
# Start up mrv2
#
"${dir}/bin/hdr" "$@"

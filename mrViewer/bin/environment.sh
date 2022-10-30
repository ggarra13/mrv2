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
# Find directory where mrViewer bash script resides
    dir=${dir%/*}
fi

# If running from current directory, get full path
if [[ "$dir" == "." ]]; then
    dir=$PWD
fi


#
# Platform specific directory for mrViewer
#
dir=${dir%/*}

#
# Add mrViewer's lib directory first to LD_LIBRARY_PATH
#
export LD_LIBRARY_PATH="${dir}/lib:${LD_LIBRARY_PATH}"

export CTL_MODULE_PATH="${dir}/ctl:${CTL_MODULE_PATH}"

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
# Start up mrViewer
#
sh -c "${dir}/bin/mrv2 $params"

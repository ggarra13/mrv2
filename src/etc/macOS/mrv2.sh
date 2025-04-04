#!/bin/sh

dir=$0

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

dir="`echo \"$dir\" | sed -e 's|/MacOS/*||'`"
rsrc="$dir/Resources/"
dynlib="$rsrc/lib"

# echo "$0"
# echo "dir=$dir"
# echo "rsrc=$rsrc"
# echo "dynlib=$dynlib"

export DYLD_LIBRARY_PATH="$dynlib:${DYLD_LIBRARY_PATH}"
export DYLD_FALLBACK_LIBRARY_PATH=$dynlib
export DYLD_FRAMEWORK_PATH=$dynlib
exec "$rsrc/bin/mrv2" $*

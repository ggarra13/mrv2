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

export DYLD_PRINT_LIBRARIES=1
export DYLD_LIBRARY_PATH="$dynlib:${DYLD_LIBRARY_PATH}"
export DYLD_FALLBACK_LIBRARY_PATH=$dynlib
export DYLD_FRAMEWORK_PATH=$dynlib

#
# Set the MoltenVK ICD filename
#
export VK_ICD_FILENAMES=$rsrc/etc/vulkan/icd.d/MoltenVK_icd.json:$VK_ICD_FILENAMES

#
# Set the MoltenVK log level to nothing
#
export MVK_CONFIG_LOG_LEVEL=0


# The first /opt/homebrew is for MacOS Silicon
# The second  /usr/local is for MacOS Intel
export VK_LAYER_PATH=/opt/homebrew/share/vulkan/explicit_layers.d:/usr/local/opt/vulkan-validationlayers/share/vulkan/explicit_layer.d:$VK_LAYER_PATH
exec "$rsrc/bin/hdr" $*

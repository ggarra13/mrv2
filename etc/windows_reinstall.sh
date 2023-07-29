#!/bin/bash

#
# Revert windows USD from release by moving all the USD DLL files in bin to lib
#
echo "Moving back USD DLLs to lib directory..."
mv -f $BUILD_DIR/bin/usd*.dll $BUILD_DIR/lib

echo "Moving USD bin/usd directory to lib direcotry..."
if [[ -e $BUILD_DIR/bin/usd ]]; then
    mv $BUILD_DIR/bin/usd   $BUILD_DIR/lib
fi

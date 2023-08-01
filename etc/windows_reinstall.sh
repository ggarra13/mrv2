#!/bin/bash

#
# Revert windows USD from release by moving all the USD DLL files in bin to lib
#

echo "Reverting Windows USD for compiling..."
if compgen -G "$BUILD_DIR/install/bin/usd*.dll" > /dev/null; then
    echo "Moving back USD DLLs to lib directory..."
    cp -f $BUILD_DIR/install/bin/usd*.dll $BUILD_DIR/install/lib
fi

if [[ -d $BUILD_DIR/install/bin/usd ]]; then
    echo "Moving USD bin/usd directory to lib direcotry..."
    cp -r $BUILD_DIR/install/bin/usd   $BUILD_DIR/install/lib/
fi

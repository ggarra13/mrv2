#!/bin/bash

#
# Prepare windows USD for release by moving all the DLL files in lib to bin
#
echo "Preparing Windows USD for install/package..."
if compgen -G "$BUILD_DIR/install/lib/*.dll" > /dev/null; then
    echo "Moving USD DLLs to bin directory..."
    mv -f $BUILD_DIR/install/lib/*.dll $BUILD_DIR/install/bin
fi

if [[ -d $BUILD_DIR/install/lib/usd ]]; then
    echo "Moving USD lib directory to bin direcotry..."
    rm -rf $BUILD_DIR/install/bin/usd
    mv -f $BUILD_DIR/install/lib/usd   $BUILD_DIR/install/bin
fi

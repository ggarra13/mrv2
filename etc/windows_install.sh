#!/bin/bash

#
# Prepare windows USD for release by moving all the DLL files in lib to bin
#
echo "Moving USD DLLs to bin directory..."
mv -f $BUILD_DIR/lib/*.dll $BUILD_DIR/bin

echo "Moving USD lib directory to bin direcotry..."
if [[ -e $BUILD_DIR/lib/usd ]]; then
    mv $BUILD_DIR/lib/usd   $BUILD_DIR/bin
fi

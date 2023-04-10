#!/usr/bin/env bash

echo "Getting latest release of cmake"

. ./etc/build_dir.sh

echo "Will install it in $BUILD_DIR/install.."

echo "Downloading cmake..."
wget -c -q https://github.com/Kitware/CMake/releases/download/v3.26.3/cmake-3.26.3-linux-x86_64.tar.gz

echo "Decompressing archive..."
tar -xf cmake-3.26.3-linux-x86_64.tar.gz

echo "Installing..."
mv -f cmake-3.26.3-linux-x86_64/* $BUILD_DIR/install/

echo "Cleaning up..."
rm -rf cmake-3.26.3-linux-x86_64*

echo "Checking executable is there:"
ls  $BUILD_DIR/install/bin

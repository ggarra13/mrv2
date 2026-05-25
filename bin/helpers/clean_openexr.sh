#!/usr/bin/env bash

. etc/build_dir.sh

echo "This script will clear OpenEXR and OpenJPH at $BUILD_DIR"
ask_to_continue

export INSTALL=$BUILD_DIR/install

echo "Cleaning lib/libOpenEXR*"
rm -rf $INSTALL/lib/libOpenEXR*
rm -rf $INSTALL/lib/OpenEXR*

echo "Cleaning lib/libopenjph*"
rm -rf $INSTALL/lib/libopenjph*
rm -rf $INSTALL/lib/openjph*

echo "Cleaning tlRender deps OpenEXR"
rm -rf $BUILD_DIR/tlRender/etc/SuperBuild/OpenEXR

echo "Cleaning tlRender deps OpenJPH"
rm -rf $BUILD_DIR/tlRender/etc/SuperBuild/OpenJPH

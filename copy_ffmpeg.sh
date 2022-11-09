#!/usr/bin/env bash

export FFMPEG_DIR=/E/mabs/local64/bin-video/ffmpegSHARED/

cp -r $FFMPEG_DIR/include/lib* $BUILD_DIR/install/include
cp -r $FFMPEG_DIR/bin/*.lib $BUILD_DIR/install/bin
cp -r $FFMPEG_DIR/bin/*.lib $BUILD_DIR/install/lib
cp -r $FFMPEG_DIR/bin/*.dll $BUILD_DIR/install/bin

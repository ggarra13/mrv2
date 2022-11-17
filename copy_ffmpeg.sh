#!/usr/bin/env bash

export FFMPEG_DIR=/E/mabs/local${ARCH}/bin-video/ffmpegSHARED/

echo "copy_ffmpeg.sh: Copying FFFMPEG from $FFMPEG_DIR to $BUILD_DIR"
cp -r $FFMPEG_DIR/include/lib* $BUILD_DIR/install/include
cp -r $FFMPEG_DIR/bin/*.lib $BUILD_DIR/install/bin
cp -r $FFMPEG_DIR/bin/*.dll $BUILD_DIR/install/bin

echo "Copying ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/
echo "Copying ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/


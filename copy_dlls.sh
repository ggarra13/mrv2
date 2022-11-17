#!/usr/bin/env bash

# Set the location of libintl on 32 and 64 bits.  On Linux and macOS this is
# picked from /usr/local
export LIBINTL_ROOT=/E/code/lib/win${ARCH}/

#
# Set the location of ffmpeg
# 
export FFMPEG_DIR=/E/mabs/local${ARCH}/bin-video/ffmpegSHARED/

echo "copy_dlls.sh: Copying FFFMPEG from $FFMPEG_DIR to $BUILD_DIR"
cp -r $FFMPEG_DIR/include/lib* $BUILD_DIR/install/include
cp -r $FFMPEG_DIR/bin/*.lib $BUILD_DIR/install/lib
cp -r $FFMPEG_DIR/bin/*.dll $BUILD_DIR/install/bin

echo "copy_dlls.sh: Copying ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/
echo "copy_dlls.sh: Copying ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/


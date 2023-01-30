#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.




echo "copy_dlls.sh: Copying FFFMPEG from $FFMPEG_DIR to $BUILD_DIR"

echo "Copying header files $BUILD_DIR/install/include"
cp -r $FFMPEG_DIR/include/lib* $BUILD_DIR/install/include

echo "Copying $FFMPEG_DIR/lib/*.lib to $BUILD_DIR/install/lib"
cp -f $FFMPEG_DIR/lib/*.lib $BUILD_DIR/install/lib

echo "Copying $FFMPEG_DIR/bin/*.dll to $BUILD_DIR/install/bin"
cp -f $FFMPEG_DIR/bin/*.dll $BUILD_DIR/install/bin

echo "copy_dlls.sh: Copying ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/
echo "copy_dlls.sh: Copying ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/


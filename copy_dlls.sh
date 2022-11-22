#!/usr/bin/env bash


echo "copy_dlls.sh: Copying FFFMPEG from $FFMPEG_DIR to $BUILD_DIR"
cp -r $FFMPEG_DIR/include/lib* $BUILD_DIR/install/include
cp -r $FFMPEG_DIR/bin/*.lib $BUILD_DIR/install/lib
cp -r $FFMPEG_DIR/bin/*.dll $BUILD_DIR/install/bin

echo "copy_dlls.sh: Copying ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/
echo "copy_dlls.sh: Copying ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/


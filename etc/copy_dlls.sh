#!/usr/bin/env bash


echo "copy_dlls.sh: Copying FFFMPEG from $FFMPEG_DIR to $BUILD_DIR"

if [[ ! -d $FFMPEG_DIR/include/ ]];then
   echo "$FFMPEG_DIR/include is missing"
   exit 1
fi

echo "Copying header files $BUILD_DIR/install/include"
cp -r $FFMPEG_DIR/include/lib* $BUILD_DIR/install/include

echo "Copying $FFMPEG_DIR/bin-video/*.lib to $BUILD_DIR/install/lib"
cp -f $FFMPEG_DIR/bin-video/*.lib $BUILD_DIR/install/lib

echo "Copying $FFMPEG_DIR/bin-video/*.dll to $BUILD_DIR/install/bin"
cp -f $FFMPEG_DIR/bin-video/*.dll $BUILD_DIR/install/bin

echo "copy_dlls.sh: Copying ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/
echo "copy_dlls.sh: Copying ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/

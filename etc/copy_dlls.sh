#!/usr/bin/env bash


echo "copy_dlls.sh: Copying FFFMPEG from $FFMPEG_ROOT to $BUILD_DIR"

if [[ ! -d $FFMPEG_ROOT/include/ ]];then
   echo "FFMPEG_ROOT $FFMPEG_ROOT/include is missing"
   exit 1
fi


echo "Copying header files from ${FFMPEG_ROOT}/include to $$PWD/$BUILD_DIR/install/include"
cp -r ${FFMPEG_ROOT}/include/lib* $PWD/$BUILD_DIR/install/include
ls $PWD/$BUILD_DIR/install/include

if [[ -d ${MABS_ROOT} ]]; then
    echo "We located a media-autobuild_suite install"
    echo "Copying $FFMPEG_ROOT/bin-video/*.lib to $PWD/$BUILD_DIR/install/lib"
    cp -f ${FFMPEG_ROOT}/bin-video/*.lib $PWD/$BUILD_DIR/install/lib
    ls $PWD/$BUILD_DIR/install/lib

    echo "Copying $FFMPEG_ROOT/bin-video/*.dll to $PWD/$BUILD_DIR/install/bin"
    cp -f ${FFMPEG_ROOT}/bin-video/*.dll $PWD/$BUILD_DIR/install/bin
    ls $PWD/$BUILD_DIR/install/bin
else
    echo "Not a media-autobuild_suite install"
    echo "Copying $FFMPEG_ROOT/bin/*.lib to $PWD/$BUILD_DIR/install/lib"
    cp -f ${FFMPEG_ROOT}/bin/*.lib $PWD/$BUILD_DIR/install/lib
    ls $PWD/$BUILD_DIR/install/lib

    echo "Copying $FFMPEG_ROOT/bin/*.dll to $PWD/$BUILD_DIR/install/bin"
    cp -f ${FFMPEG_ROOT}/bin/*.dll $PWD/$BUILD_DIR/install/bin
    ls $PWD/$BUILD_DIR/install/bin
fi

echo "copy_dlls.sh: Copying ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/lib/libintl.lib   $PWD/$BUILD_DIR/install/lib/
echo "copy_dlls.sh: Copying ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libintl-8.dll $PWD/$BUILD_DIR/install/bin/
echo "copy_dlls.sh: Copying ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/"
cp -f ${LIBINTL_ROOT}/bin/libiconv-2.dll $PWD/$BUILD_DIR/install/bin/

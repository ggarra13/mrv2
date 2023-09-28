#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. $PWD/etc/windows_prepare.sh

#
# Install libintl, libiconv and gettext
#
if [[ $RUNME == 1 ]]; then

    #
    # Install libintl, iconv and gettext from MSys repositories
    #
    bin/install_libintl_windows.sh


    #
    # Deal with FFmpeg next
    #
    if [[ ! -e $FFMPEG_ROOT/lib/avformat.lib ]]; then
	. bin/compile_ffmpeg_windows.sh
    else
	echo "Copying pre-built FFmpeg from $FFMPEG_ROOT"
	run_cmd cp -f $FFMPEG_ROOT/bin/av*.dll $BUILD_DIR/install/bin
	run_cmd cp -f $FFMPEG_ROOT/bin/sw*.dll $BUILD_DIR/install/bin
	run_cmd cp -f $FFMPEG_ROOT/lib/av*.lib $BUILD_DIR/install/lib
	run_cmd cp -f $FFMPEG_ROOT/lib/sw*.lib $BUILD_DIR/install/lib
	run_cmd cp -rf $FFMPEG_ROOT/include/libav* $BUILD_DIR/install/include
	run_cmd cp -rf $FFMPEG_ROOT/include/libsw* $BUILD_DIR/install/include
    fi


    #
    # Deal with libcms2
    #
    if [[ ! -e $LCMS2_ROOT/lib/liblcms2.lib ]]; then
	echo "Compiling liblcms2..."
	. bin/compile_liblcms2_windows.sh
    else
	echo "Copying pre-built liblcms from $LCMS2_ROOT"
	run_cmd cp -f $LCMS2_ROOT/bin/liblcms2*.dll  $BUILD_DIR/install/bin
	run_cmd cp -f $LCMS2_ROOT/lib/liblcms2.lib   $BUILD_DIR/install/lib
	run_cmd cp -f $LCMS2_ROOT/include/lcms2.h    $BUILD_DIR/install/include
    fi

fi

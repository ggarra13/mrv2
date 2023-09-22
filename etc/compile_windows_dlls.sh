#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

if [[ -e $PWD/$BUILD_DIR/install/lib/avformat.lib ]]; then
    #bin/compile_ffmpeg_windows.sh --gpl
    bin/compile_ffmpeg_windows.sh
fi


if [[ -e $PWD/$BUILD_DIR/install/lib/liblcms2.lib ]]; then
    bin/compile_liblcms2_windows.sh
fi


bin/install_libintl_windows.sh

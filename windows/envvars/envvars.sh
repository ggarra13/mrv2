#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
# Environment variables used to locate the pre-compiled libraries on Windows
#


#
# Set the location of precompiled liblcms2 on Windows.
# If not set, it will compile it.
#
# On Linux and macOS this is always compiled by cmake.
#
export LCMS2_ROOT=$PWD/precompiled/windows/win64/
if [[ ! -e $LCMS2_ROOT ]]; then
    echo "LCMS2_ROOT=$LCMS2_ROOT"
    echo "Not found in the repository."
    exit 1
fi

#
# Set the location of precompiled FFmpeg on Windows.
#
export FFMPEG_ROOT=$PWD/precompiled/windows/win64/
if [[ $FFMPEG_GPL == GPL || $FFMPEG_GPL == LGPL ]]; then
    export FFMPEG_ROOT=""
fi

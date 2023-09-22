#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
# Environment variables used to locate the pre-compiled libraries on Windows
#


#
# Set the location of libintl on windows 32 and 64 bits.
# On Linux and macOS this is picked from /usr/local
#
export LCMS2_ROOT=$PWD/windows/win64/

#
# Set the location of precompiled FFmpeg on Windows.
# We copy the contents of this directory to the build directory.
#
export FFMPEG_ROOT=$PWD/windows/win64/

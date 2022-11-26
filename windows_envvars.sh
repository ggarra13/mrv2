#!/usr/bin/env bash

# Set the location of libintl on 32 and 64 bits.  On Linux and macOS this is
# picked from /usr/local
export LIBINTL_ROOT=$PWD/windows/win${ARCH}/

#
# Set the location of ffmpeg
# 
export FFMPEG_DIR=$PWD/windows/win${ARCH}/

#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
# Rename /usr/bin/link.exe to /usr/bin/link_msys.exe
#
if [[ $KERNEL == *Msys* ]]; then
    if [[ -e /usr/bin/link.exe ]]; then
	echo "Renaming /usr/bin/link.exe as /usr/bin/link_msys.exe"
	mv /usr/bin/link.exe /usr/bin/link_msys.exe
    fi
fi

bits=64
if [[ "$ARCH" != "amd64" ]]; then
    bits=32
fi

# Set the location of libintl on windows 32 and 64 bits.
# On Linux and macOS this is picked from /usr/local
export LIBINTL_ROOT=$PWD/windows/win${bits}/

#
# Set the location of the medua-autobuild_suite for ffmpeg on Windows.
# We copy the contents of this directory to the build directory.
#
export FFMPEG_ROOT="/E/media-autobuild_suite2/"

if [[ ! -d "${FFMPEG_DIR}" ]]; then
    # media-autobuild_suite not installed, pick local pre-built library
    echo "media-autobuild_suite not installed in:"
    echo "   FFMPEG_ROOT=${FFMPEG_ROOT}"
    export FFMPEG_ROOT=${PWD}/windows/
    echo "Will pick local pre-built libs from:"
    echo "   FFMPEG_ROOT=${FFMPEG_ROOT}"
else
    # media-autobuild_suite is installed in FFMPEG_ROOT.
    # copy the ffmpeg_options.txt and the media-autobuild_suite.ini files
    echo "Copying configuration options to media-autobuild_suite..."
    cp windows/media-autobuild_suite/ffmpeg_options.txt ${FFMPEG_ROOT}/build
    cp windows/media-autobuild_suite/media-autobuild_suite.ini ${FFMPEG_ROOT}/build
fi

export FFMPEG_DIR=${FFMPEG_ROOT}/local${bits}

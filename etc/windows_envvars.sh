#!/usr/bin/env bash

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
export FFMPEG_ROOT=E:/media-autobuild_suite/
export FFMPEG_DIR=${FFMPEG_ROOT}/local${bits}

if [[ ! -d "${FFMPEG_DIR}" ]]; then
    # media-autobuild_suite not installed, pick local pre-built library
    echo "media-autobuild_suit not installed, will pick local pre-built libs:"
    export FFMPEG_DIR=$PWD/windows/local${bits}
    echo "PWD=$PWD FFMPEG_DIR=$FFMPEG_DIR"
else
    # media-autobuild_suite is installed in FFMPEG_ROOT.
    # copy the ffmpeg_options.txt and the media-autobuild_suite.ini files
    echo "Copying configuration options to media-autobuild_suite..."
    cp windows/build/ffmpeg_options.txt ${FFMPEG_ROOT}/build
    cp windows/build/media-autobuild_suite.ini ${FFMPEG_ROOT}/build
    sleep 3
fi

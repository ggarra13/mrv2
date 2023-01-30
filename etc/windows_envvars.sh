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


# Set the location of libintl on windows 32 and 64 bits.
# On Linux and macOS this is picked from /usr/local
export LIBINTL_ROOT=$PWD/windows/win${ARCH}/

#
# Set the location of ffmpeg on Windows.
# We copy the contents of this directory to the build directory.
#
export FFMPEG_DIR=$PWD/windows/win${ARCH}/

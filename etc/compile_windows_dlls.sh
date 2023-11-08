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
    # Deal with libcms2 (compile from source)
    #
    bin/compile_liblcms2_windows.sh
    
    #
    # Deal with FFmpeg next (compile from source)
    #
    . bin/compile_ffmpeg_windows.sh 

fi

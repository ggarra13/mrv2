#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. $PWD/etc/windows_prepare.sh

#
# Install libintl, iconv and gettext, subversion, libsqlite
# from MSys repositories
#
bin/install_libintl_windows.sh

#
# Deal with FFmpeg dependencies (compiled from source)
#
. bin/compile_ffmpeg_deps_windows.sh 

#
# Deal with libcms2 (compile from source)
#
bin/compile_liblcms2_windows.sh

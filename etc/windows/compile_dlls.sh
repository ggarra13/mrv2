#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. etc/windows/windows_prepare.sh

#
# Install libintl, iconv and gettext, subversion, libsqlite
# from MSys repositories
#
. bin/compile/install_libintl_windows.sh

#
# Deal with FFmpeg dependencies (compiled from source)
#
. bin/compile/compile_ffmpeg_deps_windows.sh 

#
# Deal with libcms2 (compile from source)
#
. bin/compile/compile_liblcms2_windows.sh

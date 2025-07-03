#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Rename /usr/bin/link.exe to /usr/bin/link_msys.exe so that we don't
# shallow MSVC's own link.exe (linker).
#
etc/windows/install_compiler.sh

if [[ -e /usr/bin/link.exe ]]; then
    run_cmd mv /usr/bin/link.exe /usr/bin/link_msys.exe
fi

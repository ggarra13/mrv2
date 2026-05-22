#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Flushes the disk caches to measure performance of vmrv2 on a cold start under
# Linux or macOS.
#

OS=`uname`

if [[ "$OS" == "Linux" ]]; then 
    # Write any dirty pages to disk
    sync

    #
    # Clean up disk caches.
    #
    sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
elif [[ "$OS" == "Darwin" ]]; then
    #
    # Clean up disk caches both for reading and writing.
    #
    sudo purge
else
    #
    # Clean up disk caches for C and D drives
    #
    powershell.exe -NoProfile -Command "Start-Process powershell -ArgumentList '-NoProfile -Command Write-VolumeCache -DriveLetter C' -Verb RunAs"
    powershell.exe -NoProfile -Command "Start-Process powershell -ArgumentList '-NoProfile -Command Write-VolumeCache -DriveLetter D' -Verb RunAs"
fi

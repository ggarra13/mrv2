#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

export CPU_CORES=4

#
# Get the CPU cores
#
if [[ $KERNEL == *Darwin* ]]; then
    export CPU_CORES=$(sysctl -n hw.ncpu)
else
    export CPU_CORES=$(grep -c ^processor /proc/cpuinfo)
fi

#
# Ninja uses CPU_CORES + 2, but that creates network issues on my network.
#
if [[ ! -z "$GITHUB_ACTIONS" ]]; then
    export CPU_CORES=$((CPU_CORES - 2))
fi

#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/usr/bin/emv bash

export CPU_CORES=4

#
# Get the CPU cores
#
if [[ $KERNEL == *Darwin* ]]; then
    export CPU_CORES=$(sysctl -n hw.ncpu)
else
    export CPU_CORES=$(grep -c ^processor /proc/cpuinfo)
fi

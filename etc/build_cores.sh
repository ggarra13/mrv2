#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/usr/bin/emv bash

export CPU_CORES=4

#
# Get the CPU cores
#
if [[ $KERNEL == *Linux* ]]; then
    export CPU_CORES=$(awk '/^processor/ {++n} END {print n+1}' /proc/cpuinfo)
elif [[ $KERNEL == *Darwin* ]]; then
    export CPU_CORES=$(sysctl -n hw.ncpu)
elif [[ $KERNEL == *Msys* ]]; then
    export CPU_CORES=$(wmic cpu get NumberOfCores | awk 'NR==2')
fi

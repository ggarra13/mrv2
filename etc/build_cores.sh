#!/usr/bin/emv bash

export CPU_CORES=4

#
# Get the CPU cores
#
if [[ $KERNEL == *Linux* ]]; then
    export CPU_CORES=$(awk '/^processor/ {++n} END {print n+1}' /proc/cpuinfo)
elif [[ $KERNEL == *Darwin* ]]; then
    export CPU_CORES=$(sysctl -n hw.ncpu)
fi

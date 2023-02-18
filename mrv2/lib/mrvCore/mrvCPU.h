// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

typedef struct cpucaps_s
{
    int cpuType;
    int cpuModel;
    int cpuStepping;
    int hasMMX;
    int hasMMX2;
    int has3DNow;
    int has3DNowExt;
    int hasSSE;
    int hasSSE2;
    int hasSSE3;
    int hasSSSE3;
    int hasSSE4;
    int hasSSE42;
    int hasAESNI;
    int isX86;
    unsigned cl_size; /* size of cache line */
    int hasAltiVec;
    int hasTSC;
} CpuCaps;

extern CpuCaps gCpuCaps;

std::string GetCpuCaps(CpuCaps* caps);

unsigned int cpu_count();

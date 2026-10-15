// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

/**
 * A struct holding the CPU capabilities
 *
 */
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
    int hasNEON;
    int hasARMv8;
    int hasARMv8Crypto;
    int hasARMv8Crc;     /* CRC32 instructions */
    int hasARMv8Dotprod; /* v8.2 SDOT/UDOT integer dot product */
    int hasARMv8Fp16;    /* half-precision (FP16) NEON arithmetic */
    int hasSVE;          /* Scalable Vector Extension */
    int hasSVE2;         /* Scalable Vector Extension 2 */
} CpuCaps;

extern CpuCaps gCpuCaps;

/**
 * Returns a string of CPU capabilities based on a CpuCaps struct.
 *
 * @param caps input CpuCaps struct.
 *
 * @return a std::string.
 */
std::string GetCpuCaps(CpuCaps* caps);

/**
 * Counts the number of CPUs.
 *
 *
 * @return cpu count.
 */
unsigned int cpu_count();

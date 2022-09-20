/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvCPU.h
 * @author gga
 * @date   Mon Jan 14 21:24:10 2008
 *
 * @brief  CPU detection routines
 *
 *
 */


#ifndef mrvCPU_h
#define mrvCPU_h

#include <string>

typedef struct cpucaps_s {
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

std::string GetCpuCaps(CpuCaps *caps);

unsigned int cpu_count();

#endif // mrvCPU_h

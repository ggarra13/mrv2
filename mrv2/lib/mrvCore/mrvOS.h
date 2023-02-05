// SPDX-License-Identifier: BSD-3-Clause
// mrv2 
// Copyright Contributors to the mrv2 Project. All rights reserved.



#pragma once


#ifdef _WIN32

#include <direct.h>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#if !(defined vsnprintf)
#  define vsnprintf       _vsnprintf
#endif

#if !(defined putenv)
#  define putenv(x)       _putenv(x)
#endif

#undef max
#undef min
#undef stricmp

#if !(defined strcasecmp)
#  ifdef _WIN32
#    define strcasecmp(a,b) _stricmp(a,b)
#  else
#    define strcasecmp(a,b) stricmp(a,b)
#  endif
#endif

#if !(defined strtok_r)
#  define strtok_r(a,b,c) strtok(a,b)
#endif

#if !(defined snprintf)
#  define snprintf        _snprintf
#endif

#if !(defined access)
#  define access          _access
#endif


#undef itoa
#define itoa(x,a,b) _itoa(x,a,b)

#undef getcwd
#define getcwd _getcwd

#undef chdir
#define chdir _chdir

int setenv (const char * name, const char * value, int overwrite );

#endif // _WIN32

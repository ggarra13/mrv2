// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include <FL/fl_utf8.h>

#if defined(WIN32) || defined(WIN64)
#    include <winsock2.h>
#    include <io.h>      // for _access
#    include <windows.h> // for GetModuleFileNameW
#    define strdup _strdup
#else
#    include <unistd.h> // for access
#endif                  /* WIN32 */

#ifdef __APPLE__ /* assume this is OSX */
#    include <sys/param.h>

/* _NSGetExecutablePath : must add -framework CoreFoundation to link line */
#    include <mach-o/dyld.h>

#    ifndef PATH_MAX
#        define PATH_MAX MAXPATHLEN
#    endif
#endif /* APPLE */

#ifndef PATH_MAX
#    define PATH_MAX 2048
#endif

#include "mrvCore/mrvEnv.h"

#include "mrvRoot.h"

namespace
{

#ifdef _WIN32
    int get_app_path(wchar_t* pname, size_t pathsize)
    {
        // Use a larger buffer for long paths (Windows supports up to
        // 32767 chars)
        DWORD result = GetModuleFileNameW(NULL, pname, DWORD(pathsize));
        if (result == 0 || result >= pathsize) {
            std::wcerr << L"GetModuleFileNameW failed or buffer too small"
                       << std::endl;
            return -1;
        }

        // Check if the file exists
        if (GetFileAttributesW(pname) == INVALID_FILE_ATTRIBUTES) {
            std::wcerr << L"File does not exist or is inaccessible"
                       << std::endl;;
            return -1;
        }

        // Replace backslashes with forward slashes
        wchar_t* p = pname;
        while (*p) {
            if (*p == L'\\') *p = L'/';
            p++;
        }
        
        return 0; // Success
    }
#else  // !_WIN32
    /*
     * Mechanism to handle determining *where* the exe actually lives
     */
    int get_app_path(char* pname, size_t pathsize)
    {
        long result;

#  ifdef __linux__
        /* Oddly, the readlink(2) man page says no NULL is appended. */
        /* So you have to do it yourself, based on the return value: */
        pathsize--; /* Preserve a space to add the trailing NULL */
        result = readlink("/proc/self/exe", pname, pathsize);
        if (result > 0)
        {
            pname[result] = 0; /* add the #@!%ing NULL */

            if ((access(pname, 0) == 0))
                return 0; /* file exists, return OK */
                          /*else name doesn't seem to exist, return FAIL (falls
                            through) */
        }
#  elif defined(SOLARIS)
        char* p = getexecname();
        if (p)
        {
            /* According to the Sun manpages, getexecname will
               "normally" return an */
            /* absolute path - BUT it may not AND therefore,
               prepending */
            /* getcwd() will "usually" be the correct thing... Urgh!
             */

            /* check pathname is absolute (begins with a / ???) */
            if (p[0] == '/') /* assume this means we have an
                                    absolute path */
            {
                strncpy(pname, p, pathsize);
                if ((access(pname, 0) == 0))
                    return 0; /* file exists, return OK */
            }
            else /* if not, prepend getcwd() then check if file
                    exists */
            {
                getcwd(pname, pathsize);
                result = strlen(pname);
                strncat(pname, "/", (pathsize - result));
                result++;
                strncat(pname, p, (pathsize - result));

                if ((access(pname, 0) == 0))
                    return 0; /* file exists, return OK */
                              /*else name doesn't seem to exist, return FAIL
                                (falls through) */
            }
        }
#  elif defined(__APPLE__) /* assume this is OSX */
        /*
          from http://www.hmug.org/man/3/NSModule.html

          extern int _NSGetExecutablePath(char *buf, unsigned long
          *bufsize);

          _NSGetExecutablePath  copies  the  path  of the executable
          into the buffer and returns 0 if the path was successfully
          copied  in the provided buffer. If the buffer is not large
          enough, -1 is returned and the  expected  buffer  size  is
          copied  in  *bufsize.  Note that _NSGetExecutablePath will
          return "a path" to the executable not a "real path" to the
          executable.  That  is  the path may be a symbolic link and
          not the real file. And with  deep  directories  the  total
          bufsize needed could be more than MAXPATHLEN.
        */
        int status = -1;
        char* given_path = (char*)malloc(MAXPATHLEN * 2);
        if (!given_path)
            return status;

        uint32_t pathSize = MAXPATHLEN * 2;
        result = _NSGetExecutablePath(given_path, &pathSize);
        if (result == 0)
        { /* OK, we got something - now try and resolve the real path...
           */
            if (realpath(given_path, pname) != NULL)
            {
                if ((access(pname, 0) == 0))
                    status = 0; /* file exists, return OK */
            }
        }
        free(given_path);
        return status;
#  else                    /* APPLE */
#    error Unknown OS
#  endif /* APPLE */

        return -1; /* Path Lookup Failed */
    } /* where_do_I_live */
#endif // ! _WIN32
} // namespace

namespace mrv
{

    void set_root_path(const int argc, char** argv)
    {
#ifdef _WIN32
        // Check existing environment variable
        wchar_t binpath[32767]; // Max path length on Windows
        binpath[0] = L'\0';

        int ok = get_app_path(binpath, 32767);
        if (ok != 0)
        {
            if (argc >= 1) {
                // Convert argv[0] from char* to wchar_t*
                size_t converted;
                mbstowcs_s(&converted, binpath, argv[0], 32767);
            }
        }
                
        fs::path rootdir(binpath);
        fs::path parent = rootdir.parent_path(); // Skip executable
        rootdir = parent.parent_path();          // Skip bin/ directory

        std::wstring root_str = rootdir.wstring();
        if (setenv(L"MRV2_ROOT", root_str.c_str(), 1) != 0)
        {
            std::wcerr << L"failed to set MRV2_ROOT" << std::endl;
        }
#else  // ! _WIN32
        char* root = fl_getenv("MRV2_ROOT");

        if (!root)
        {
            char binpath[PATH_MAX];
            binpath[0] = 0;

            int ok = get_app_path(binpath, PATH_MAX);
            if (ok != 0)
            {
                if (argc >= 1)
                    strcpy(binpath, argv[0]);
            }

            fs::path rootdir(binpath);
            fs::path parent = rootdir.parent_path(); // skip executable
            rootdir = parent.parent_path();          // skip bin/ directory

            setenv("MRV2_ROOT", rootdir.string().c_str(), 1);
        }
#endif
    }
} // namespace mrv

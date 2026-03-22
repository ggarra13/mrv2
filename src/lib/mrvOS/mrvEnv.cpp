// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string.h>
#include <stdlib.h>

#ifdef _WIN32

#include <iostream>

#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>

/**
 * Defines a setenv() function equivalent to the Unix one for Windows.
 *
 * @param name variable name
 * @param value value to change
 * @param overwrite overwrite (unused)
 *
 * @return 0 if success, -1 if not.
 */
int setenv(const wchar_t* name, const wchar_t* value, int overwrite)
{
    /* On Win32, each process has two copies of the environment variables,
       one managed by the OS and one managed by the C library. We set
       the value in both locations, so that other software that looks in
       one place or the other is guaranteed to see the value. Even if it's
       a bit slow. See also
       <https://article.gmane.org/gmane.comp.gnu.mingw.user/8272>
       <https://article.gmane.org/gmane.comp.gnu.mingw.user/8273>
       <https://www.cygwin.com/ml/cygwin/1999-04/msg00478.html> */
    if (!SetEnvironmentVariableW(name, value))
    {
        std::wcerr << "SetEnvironmentVariableW failed" << std::endl;
        return -1;
    }
    if (_wputenv_s(name, value) != 0) {
        std::wcerr << L"_wputenv_s failed" << std::endl;
        return -1;
    }
    
    return 0;
}

/**
 * Defines an unsetenv() function like the Unix one.
 *
 * @param name variable name
 *
 * @return 0 if success, other if not.
 */
int unsetenv(const wchar_t* name)
{
    if (!SetEnvironmentVariableW(name, L""))
        return -1;
    return _wputenv_s(name, L"");
}
#endif

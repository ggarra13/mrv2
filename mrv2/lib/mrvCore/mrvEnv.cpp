// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string.h>
#include <stdlib.h>

#ifdef _WIN32

#    include "mrvCore/mrvOS.h"

/**
 * Defines a setenv() function equivalent to the Unix one for Windows.
 *
 * @param name variable name
 * @param value value to change
 * @param overwrite overwrite (unused)
 *
 * @return 0 if success, -1 if not.
 */
int setenv(const char* name, const char* value, int overwrite)
{
    /* On Woe32, each process has two copies of the environment variables,
       one managed by the OS and one managed by the C library. We set
       the value in both locations, so that other software that looks in
       one place or the other is guaranteed to see the value. Even if it's
       a bit slow. See also
       <https://article.gmane.org/gmane.comp.gnu.mingw.user/8272>
       <https://article.gmane.org/gmane.comp.gnu.mingw.user/8273>
       <https://www.cygwin.com/ml/cygwin/1999-04/msg00478.html> */
    if (!SetEnvironmentVariableA(name, value))
        return -1;
#    if 1
    return _putenv_s(name, value);
#    else
    size_t namelen = strlen(name);
    size_t valuelen = (value == NULL ? 0 : strlen(value));
    char* buffer = (char*)malloc(namelen + 1 + valuelen + 1);
    if (!buffer)
        return -1; /* no need to set errno = ENOMEM */
    memcpy(buffer, name, namelen);
    if (value != NULL)
    {
        buffer[namelen] = '=';
        memcpy(buffer + namelen + 1, value, valuelen);
        buffer[namelen + 1 + valuelen] = 0;
    }
    else
        buffer[namelen] = 0;
    return putenv(buffer);
#    endif
}

/**
 * Defines an unsetenv() function like the Unix one.
 *
 * @param name variable name
 *
 * @return 0 if success, other if not.
 */
int unsetenv(const char* name)
{
    if (!SetEnvironmentVariableA(name, ""))
        return -1;
    return _putenv_s(name, "");
}
#endif

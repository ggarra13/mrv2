// SPDX-License-Identifier: BSD-3-Clause
// mrv2 
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string.h>
#include <stdlib.h>

#ifdef _WIN32

#include "mrvCore/mrvOS.h"

int setenv (const char * name, const char * value, int overwrite) {
  /* On Woe32, each process has two copies of the environment variables,
     one managed by the OS and one managed by the C library. We set
     the value in both locations, so that other software that looks in
     one place or the other is guaranteed to see the value. Even if it's
     a bit slow. See also
     <https://article.gmane.org/gmane.comp.gnu.mingw.user/8272>
     <https://article.gmane.org/gmane.comp.gnu.mingw.user/8273>
     <https://www.cygwin.com/ml/cygwin/1999-04/msg00478.html> */
  if (!SetEnvironmentVariableA(name,value))
    return -1;
  return _putenv_s(name, value);
}
#endif
